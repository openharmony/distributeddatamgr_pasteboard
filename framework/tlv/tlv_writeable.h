/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_WRITEABLE_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_WRITEABLE_H

#include "endian_converter.h"
#include "tlv_countable.h"

namespace OHOS::MiscServices {

bool IsRemoteEncode();

class WriteOnlyBuffer;

class TLVWriteable : public TLVCountable {
public:
    virtual ~TLVWriteable() = default;

    virtual bool EncodeTLV(WriteOnlyBuffer &buffer) const = 0;

    API_EXPORT bool Encode(std::vector<uint8_t> &buffer, bool isRemote = false) const;
};

class WriteOnlyBuffer : public TLVBuffer {
public:
    explicit WriteOnlyBuffer(size_t len) : TLVBuffer(len), data_(len)
    {
    }

    template<typename T>
    bool Write(uint16_t type, const std::vector<T> &value)
    {
        if (!HasExpectBuffer(sizeof(TLVHead))) {
            return false;
        }
        auto tagCursor = cursor_;
        cursor_ += sizeof(TLVHead); // placeholder
        auto valueCursor = cursor_;
        bool ret = WriteValue(value);
        WriteHead(type, tagCursor, cursor_ - valueCursor);
        return ret;
    }

    template<typename T>
    bool Write(uint16_t type, const std::shared_ptr<T> &value)
    {
        if (value == nullptr) {
            return true;
        }
        return Write(type, *value);
    }

    bool Write(uint16_t type, std::monostate value);
    bool Write(uint16_t type, const void *value);
    bool Write(uint16_t type, bool value);
    bool Write(uint16_t type, double value);
    bool Write(uint16_t type, int8_t value);
    bool Write(uint16_t type, int16_t value);
    bool Write(uint16_t type, int32_t value);
    bool Write(uint16_t type, int64_t value);
    bool Write(uint16_t type, uint32_t value);
    bool Write(uint16_t type, const std::string &value);
    bool Write(uint16_t type, const Object &value);
    bool Write(uint16_t type, const AAFwk::Want &value);
    bool Write(uint16_t type, const Media::PixelMap &value);
    bool Write(uint16_t type, const RawMem &value);
    bool Write(uint16_t type, const TLVWriteable &value);
    bool Write(uint16_t type, const std::vector<uint8_t> &value);
    bool Write(uint16_t type, const std::map<std::string, std::vector<uint8_t>> &value);
    bool Write(uint16_t type, const Details &value);

    template<typename _InTp>
    bool WriteVariant(uint16_t type, uint32_t step, const _InTp &input);

    template<typename _InTp, typename _First, typename... _Rest>
    bool WriteVariant(uint16_t type, uint32_t step, const _InTp &input);

    template<typename... _Types>
    bool Write(uint16_t type, const std::variant<_Types...> &input);

    template<>
    bool Write(uint16_t type, const EntryValue &input);

private:
    void WriteHead(uint16_t type, size_t tagCursor, uint32_t len)
    {
        auto *tlvHead = reinterpret_cast<TLVHead *>(data_.data() + tagCursor);
        tlvHead->tag = HostToNet(type);
        tlvHead->len = HostToNet(len);
    }

    template<typename T>
    bool WriteBasic(uint16_t type, T value)
    {
        if (!HasExpectBuffer(sizeof(TLVHead) + sizeof(value))) {
            return false;
        }
        auto *tlvHead = reinterpret_cast<TLVHead *>(data_.data() + cursor_);
        tlvHead->tag = HostToNet(type);
        tlvHead->len = HostToNet((uint32_t)sizeof(value));
        auto valueBuff = HostToNet(value);
        auto ret = memcpy_s(tlvHead->value, total_ - cursor_ - sizeof(TLVHead), &valueBuff, sizeof(value));
        if (ret != EOK) {
            return false;
        }
        cursor_ += sizeof(TLVHead) + sizeof(value);
        return true;
    }

    template<typename T>
    bool WriteValue(const std::vector<T> &value)
    {
        // items iterator
        bool ret = true;
        for (const T &item : value) {
            // V:item value
            ret = ret && Write(TAG_VECTOR_ITEM, item);
        }
        return ret;
    }

    friend class TLVWriteable;
    std::vector<uint8_t> data_;
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_WRITEABLE_H
#include "tlv_writeable.h"
#include <thread>
#include "pasteboard_hilog.h"

namespace OHOS::MiscServices {

thread_local bool g_isRemoteEncode = false;

bool IsRemoteEncode()
{
    return g_isRemoteEncode;
}

bool TLVWriteable::Encode(std::vector<uint8_t> &buffer, bool isRemote) const
{
    g_isRemoteEncode = isRemote;
    size_t len = CountTLV();
    WriteOnlyBuffer buff(len);
    bool ret = EncodeTLV(buff);
    buffer = std::move(buff.data_);
    return ret;
}

bool WriteOnlyBuffer::Write(uint16_t type, std::monostate value)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead)), false,
        PASTEBOARD_MODULE_COMMON, "write monostate failed, type=%{public}hu", type);
    cursor_ += sizeof(TLVHead);
    return true;
}

bool WriteOnlyBuffer::Write(uint16_t type, const void *value)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead)), false,
        PASTEBOARD_MODULE_COMMON, "write void* failed, type=%{public}hu", type);
    cursor_ += sizeof(TLVHead);
    return true;
}

bool WriteOnlyBuffer::Write(uint16_t type, bool value)
{
    return WriteBasic(type, static_cast<int8_t>(value));
}

bool WriteOnlyBuffer::Write(uint16_t type, int8_t value)
{
    return WriteBasic(type, value);
}

bool WriteOnlyBuffer::Write(uint16_t type, int16_t value)
{
    return WriteBasic(type, value);
}

bool WriteOnlyBuffer::Write(uint16_t type, double value)
{
    return WriteBasic(type, value);
}

bool WriteOnlyBuffer::Write(uint16_t type, int32_t value)
{
    return WriteBasic(type, value);
}

bool WriteOnlyBuffer::Write(uint16_t type, int64_t value)
{
    return WriteBasic(type, value);
}

bool WriteOnlyBuffer::Write(uint16_t type, uint32_t value)
{
    return WriteBasic(type, value);
}

bool WriteOnlyBuffer::Write(uint16_t type, const std::string &value)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead) + value.size()), false,
        PASTEBOARD_MODULE_COMMON, "write string failed, type=%{public}hu", type);

    auto *tlvHead = reinterpret_cast<TLVHead *>(data_.data() + cursor_);
    tlvHead->tag = HostToNet(type);
    tlvHead->len = HostToNet(static_cast<uint32_t>(value.size()));

    if (!value.empty()) {
        auto err = memcpy_s(tlvHead->value, value.size(), value.c_str(), value.size());
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE((err == EOK), false, PASTEBOARD_MODULE_COMMON,
            "copy string failed, type=%{public}hu", type);
    }

    cursor_ += sizeof(TLVHead) + value.size();
    return true;
}

bool WriteOnlyBuffer::Write(uint16_t type, const RawMem &value)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead) + value.bufferLen), false,
        PASTEBOARD_MODULE_COMMON, "write RawMem failed, type=%{public}hu", type);

    auto *tlvHead = reinterpret_cast<TLVHead *>(data_.data() + cursor_);
    tlvHead->tag = HostToNet(type);
    tlvHead->len = HostToNet(static_cast<uint32_t>(value.bufferLen));
    cursor_ += sizeof(TLVHead);

    if (value.bufferLen != 0 && reinterpret_cast<const void *>(value.buffer) != nullptr)
    if (value.bufferLen != 0 && value.buffer != 0) {
        auto err = memcpy_s(data_.data() + cursor_, total_ - cursor_,
            reinterpret_cast<const void *>(value.buffer), value.bufferLen);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(err == EOK, false, PASTEBOARD_MODULE_COMMON,
            "copy RawMem failed, type=%{public}hu", type);
    }

    cursor_ += value.bufferLen;
    return true;
}

bool WriteOnlyBuffer::Write(uint16_t type, const AAFwk::Want &value)
{
    return Write(type, TLVUtils::Parcelable2Raw(&value));
}

bool WriteOnlyBuffer::Write(uint16_t type, const Media::PixelMap &value)
{
    std::vector<std::uint8_t> rawData;
    if (!value.EncodeTlv(rawData)) {
        return false;
    }
    return Write(type, rawData);
}

bool WriteOnlyBuffer::Write(uint16_t type, const Object &value)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead)), false,
        PASTEBOARD_MODULE_COMMON, "write object failed, type=%{public}hu", type);

    auto tagCursor = cursor_;
    cursor_ += sizeof(TLVHead);
    auto valueCursor = cursor_;

    bool ret = true;
    for (const auto &[key, val] : value.value_) {
        ret = ret && Write(TAG_MAP_KEY, key);
        ret = ret && Write(TAG_MAP_VALUE_TYPE, val);
    }
    WriteHead(type, tagCursor, cursor_ - valueCursor);
    return ret;
}

bool WriteOnlyBuffer::Write(uint16_t type, const std::map<std::string, std::vector<uint8_t>> &value)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead)), false,
        PASTEBOARD_MODULE_COMMON, "write vector failed, type=%{public}hu", type);

    auto tagCursor = cursor_;
    cursor_ += sizeof(TLVHead);
    auto valueCursor = cursor_;

    bool ret = true;
    for (auto &item : value) {
        ret = ret && Write(TAG_MAP_KEY, item.first);
        ret = ret && Write(TAG_MAP_VALUE, item.second);
    }
    WriteHead(type, tagCursor, cursor_ - valueCursor);
    return ret;
}

template<typename _InTp>
bool WriteOnlyBuffer::WriteVariant(uint16_t type, uint32_t step, const _InTp &input)
{
    (void)type;
    (void)step;
    (void)input;
    return true;
}

template<typename _InTp, typename _First, typename... _Rest>
bool WriteOnlyBuffer::WriteVariant(uint16_t type, uint32_t step, const _InTp &input)
{
    if (step == input.index()) {
        auto val = std::get<_First>(input);
        return Write(type, val);
    }
    return WriteVariant<_InTp, _Rest...>(type, step + 1, input);
}

template<typename... _Types>
bool WriteOnlyBuffer::Write(uint16_t type, const std::variant<_Types...> &input)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead)), false,
        PASTEBOARD_MODULE_COMMON, "write variant failed, type=%{public}hu", type);

    auto tagCursor = cursor_;
    cursor_ += sizeof(TLVHead);
    auto valueCursor = cursor_;

    uint32_t index = static_cast<uint32_t>(input.index());
    if (!Write(TAG_VARIANT_INDEX, index)) {
        return false;
    }
    WriteVariant<decltype(input), _Types...>(TAG_VARIANT_VALUE, 0, input);
    WriteHead(type, tagCursor, cursor_ - valueCursor);
    return true;
}

template<>
bool WriteOnlyBuffer::Write(uint16_t type, const EntryValue &input)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead)), false,
        PASTEBOARD_MODULE_COMMON, "write UDMF::ValueType failed, type=%{public}hu", type);

    auto tagCursor = cursor_;
    cursor_ += sizeof(TLVHead);
    auto valueCursor = cursor_;

    uint32_t index = static_cast<uint32_t>(input.index());
    if (!Write(TAG_VARIANT_INDEX, index)) {
        return false;
    }
    WriteVariant<decltype(input), std::monostate, int32_t, int64_t, double, bool, std::string, std::vector<uint8_t>,
        std::shared_ptr<OHOS::AAFwk::Want>, std::shared_ptr<OHOS::Media::PixelMap>, std::shared_ptr<Object>, nullptr_t>(
        TAG_VARIANT_VALUE, 0, input);
    WriteHead(type, tagCursor, cursor_ - valueCursor);
    return true;
}

bool WriteOnlyBuffer::Write(uint16_t type, const Details &value)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead)), false,
        PASTEBOARD_MODULE_COMMON, "write Details failed, type=%{public}hu", type);

    auto tagCursor = cursor_;
    cursor_ += sizeof(TLVHead);
    auto valueCursor = cursor_;

    bool ret = true;
    for (const auto &[key, val] : value) {
        ret = ret && Write(TAG_MAP_KEY, key);
        ret = ret && Write(TAG_MAP_VALUE_TYPE, val);
    }
    WriteHead(type, tagCursor, cursor_ - valueCursor);
    return ret;
}

bool WriteOnlyBuffer::Write(uint16_t type, const TLVWriteable &value)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead)), false,
        PASTEBOARD_MODULE_COMMON, "write TLVWriteable failed, type=%{public}hu", type);

    auto tagCursor = cursor_;
    cursor_ += sizeof(TLVHead);
    auto valueCursor = cursor_;

    bool ret = value.EncodeTLV(*this);
    WriteHead(type, tagCursor, cursor_ - valueCursor);
    return ret;
}

bool WriteOnlyBuffer::Write(uint16_t type, const std::vector<uint8_t> &value)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(sizeof(TLVHead) + value.size()), false,
        PASTEBOARD_MODULE_COMMON, "write uint8 vector failed, type=%{public}hu", type);

    WriteHead(type, cursor_, value.size());
    cursor_ += sizeof(TLVHead);

    if (!value.empty()) {
        auto err = memcpy_s(data_.data() + cursor_, total_ - cursor_, value.data(), value.size());
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(err == EOK, false, PASTEBOARD_MODULE_COMMON,
            "copy uint8 vector failed, type=%{public}hu", type);
    }
    cursor_ += value.size();
    return true;
}
} // namespace OHOS::MiscServices
