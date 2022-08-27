/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_OBJECT_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_OBJECT_H
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "api/visibility.h"
#include "endian_converter.h"
#include "parcel.h"

namespace OHOS::MiscServices {
#pragma pack(1)
struct TLVHead {
    uint16_t tag;
    uint32_t len;
    std::uint8_t value[0];
};
#pragma pack()

/*
 * Common tag definitions.
 * Product should use after TAG_BUFF
 **/
enum COMMON_TAG : uint16_t {
    TAG_VECTOR = 0x0000,
    TAG_VECTOR_INFO,
    TAG_VECTOR_ITEM,

    TAG_BUFF = 0x0100,
};
struct API_EXPORT TLVObject {
public:
    virtual bool Encode(std::vector<std::uint8_t> &buffer) = 0;
    virtual bool Decode(const std::vector<std::uint8_t> &buffer) = 0;

    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, bool value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, int8_t value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, int16_t value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, int32_t value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, int64_t value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, const std::string &value);
    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, TLVObject &value);
//    bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, Parcel &value);
    template<typename ParcelType> bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, ParcelType &value)
    {
        Parcel parcel;
        value.Marshalling(parcel);
        return Write(buffer, type, parcel);
    }
    template<typename T> bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, std::vector<T> &value)
    {
        if (!Check(buffer, sizeof(TLVHead))) {
            return false;
        }
        auto *tlvHead = reinterpret_cast<TLVHead *>(buffer.data() + cursor_);
        tlvHead->tag = HostToNet(type);
        cursor_ += sizeof(TLVHead);
        auto preCursor = cursor_;
        // item count
        bool ret = Write(buffer, TAG_VECTOR_INFO, (int32_t)(value.size()));
        // items iterator
        for (auto &item : value) {
            ret = ret && Write(buffer, TAG_VECTOR_ITEM, item);
        }
        tlvHead->len = HostToNet((uint32_t)(cursor_ - preCursor));
        return ret;
    }

    template<typename T> bool Write(std::vector<std::uint8_t> &buffer, uint16_t type, std::shared_ptr<T> &value)
    {
        if (value == nullptr) {
            return false;
        }
        return Write(buffer, type, *value);
    }
    bool ReadHead(const std::vector<std::uint8_t> &buffer, TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, bool &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, int8_t &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, int16_t &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, int32_t &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, int64_t &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, std::string &value, const TLVHead &head);
    bool ReadValue(const std::vector<std::uint8_t> &buffer, TLVObject &value, const TLVHead &head);
//    bool ReadValue(const std::vector<std::uint8_t> &buffer, Parcel &value, const TLVHead &head);
    template<typename ParcelType>
    bool ReadValue(const std::vector<std::uint8_t> &buffer, ParcelType &value, const TLVHead &head)
    {
        Parcel parcel;
        bool ret = ReadValue(buffer, parcel, head);
        auto *obj = ParcelType::Unmarshalling(parcel);
        if (obj == nullptr) {
            return false;
        }
        value = *obj;
        return ret;
    }

    template<typename T>
    bool ReadValue(const std::vector<std::uint8_t> &buffer, std::vector<T> &value, const TLVHead &head)
    {
        TLVHead vectorInfo{};
        bool ret = ReadHead(buffer, vectorInfo);
        int32_t count = 0;
        ret = ret && ReadValue(buffer, count, head);
        if (!ret) {
            return false;
        }

        for (int32_t i = 0; i < count; ++i) {
            TLVHead itemHead{};
            ret = ReadHead(buffer, itemHead);
            T item{};
            ret = ret && ReadValue(buffer, item, itemHead);
            if (!ret) {
                return false;
            }
            value.push_back(item);
        }
        return true;
    }

    template<typename T>
    bool ReadValue(const std::vector<std::uint8_t> &buffer, std::shared_ptr<T> &value, const TLVHead &head)
    {
        value = std::make_shared<T>();
        return ReadValue(buffer, *value, head);
    }

protected:
    virtual ~TLVObject() = default;
    inline bool Skip(size_t len, size_t total)
    {
        if (total < len || total - len < cursor_) {
            return false;
        }
        cursor_ += len;
        return true;
    }
    inline bool IsEnough() const
    {
        return cursor_ < total_;
    }

    size_t total_ = 0;

private:
    bool Encode(std::vector<std::uint8_t> &buffer, size_t &cursor, size_t total);
    bool Decode(const std::vector<std::uint8_t> &buffer, size_t &cursor, size_t total);
    template<typename T> bool WriteBasic(std::vector<std::uint8_t> &buffer, uint16_t type, T value)
    {
        if (!Check(buffer, sizeof(TLVHead) + sizeof(value))) {
            return false;
        }
        auto *tlvHead = reinterpret_cast<TLVHead *>(buffer.data() + cursor_);
        tlvHead->tag = HostToNet(type);
        tlvHead->len = HostToNet((uint32_t)sizeof(value));
        *((T *)(tlvHead->value)) = HostToNet(value);
        cursor_ += sizeof(TLVHead) + sizeof(value);
        return true;
    }

    template<typename T> bool ReadBasicValue(const std::vector<std::uint8_t> &buffer, T &value, const TLVHead &head)
    {
        if (!Check(buffer, head.len)) {
            return false;
        }
        value = NetToHost(*((T *)(buffer.data() + cursor_)));
        cursor_ += sizeof(T);
        return true;
    }

    inline bool Check(const std::vector<std::uint8_t> &buffer, uint32_t expectLen) const
    {
        if (buffer.size() > cursor_ && buffer.size() - cursor_ < expectLen) {
            return false;
        }
        return true;
    }

    size_t cursor_ = 0;
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_OBJECT_H
