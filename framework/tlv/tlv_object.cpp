/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "tlv_object.h"

#include "securec.h"
namespace OHOS::MiscServices {
bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, bool value)
{
    return WriteBasic(buffer, type, (int8_t)(value));
}
bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, int8_t value)
{
    return WriteBasic(buffer, type, value);
}
bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, int16_t value)
{
    return WriteBasic(buffer, type, value);
}
bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, double value)
{
    return WriteBasic(buffer, type, value);
}
bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, int32_t value)
{
    return WriteBasic(buffer, type, value);
}
bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, int64_t value)
{
    return WriteBasic(buffer, type, value);
}
bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, uint32_t value)
{
    return WriteBasic(buffer, type, value);
}
bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, const std::string &value)
{
    if (!HasExpectBuffer(buffer, sizeof(TLVHead) + value.size())) {
        return false;
    }
    auto *tlvHead = reinterpret_cast<TLVHead *>(buffer.data() + cursor_);
    tlvHead->tag = HostToNet(type);
    tlvHead->len = HostToNet((uint32_t)value.size());
    if (!value.empty()) {
        auto err = memcpy_s(tlvHead->value, value.size(), value.c_str(), value.size());
        if (err != EOK) {
            return false;
        }
    }

    cursor_ += sizeof(TLVHead) + value.size();
    return true;
}

bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, const RawMem &value)
{
    if (!HasExpectBuffer(buffer, sizeof(TLVHead) + value.bufferLen)) {
        return false;
    }
    auto *tlvHead = reinterpret_cast<TLVHead *>(buffer.data() + cursor_);
    tlvHead->tag = HostToNet(type);
    cursor_ += sizeof(TLVHead);

    if (value.bufferLen != 0 && value.buffer != 0) {
        auto err = memcpy_s(buffer.data() + cursor_, buffer.size() - cursor_,
            reinterpret_cast<const void *>(value.buffer), value.bufferLen);
        if (err != EOK) {
            return false;
        }
    }
    cursor_ += value.bufferLen;
    tlvHead->len = HostToNet((uint32_t)value.bufferLen);
    return true;
}
bool TLVObject::Write(
    std::vector<std::uint8_t> &buffer, uint16_t type, std::map<std::string, std::vector<uint8_t>> &value)
{
    if (!HasExpectBuffer(buffer, sizeof(TLVHead))) {
        return false;
    }
    auto tagCursor = cursor_;
    cursor_ += sizeof(TLVHead);
    auto valueCursor = cursor_;

    bool ret = true;
    for (auto &item : value) {
        ret = ret && Write(buffer, TAG_MAP_KEY, item.first);
        ret = ret && Write(buffer, TAG_MAP_VALUE, item.second);
    }
    WriteHead(buffer, type, tagCursor, cursor_ - valueCursor);
    return ret;
}

bool TLVObject::Write(std::vector<std::uint8_t>& buffer, uint16_t type, Details& value)
{
    if (!HasExpectBuffer(buffer, sizeof(TLVHead))) {
        return false;
    }
    auto tagCursor = cursor_;
    cursor_ += sizeof(TLVHead);
    auto valueCursor = cursor_;
    for (auto [key, val] : value) {
        if (!Write(buffer, TAG_MAP_KEY, key)) {
            return false;
        }
        auto* strValue = std::get_if<std::string>(&val);
        if (strValue != nullptr) {
            Write(buffer, TAG_STRING, *strValue);
            continue;
        }
        auto* int32Value = std::get_if<int32_t>(&val);
        if (int32Value != nullptr) {
            Write(buffer, TAG_INT32, *int32Value);
            continue;
        }
        auto* int64Value = std::get_if<int64_t>(&val);
        if (int64Value != nullptr) {
            Write(buffer, TAG_INT64, *int64Value);
            continue;
        }
        auto* boolValue = std::get_if<bool>(&val);
        if (boolValue != nullptr) {
            Write(buffer, TAG_BOOL, *boolValue);
            continue;
        }
        auto* doubleValue = std::get_if<double>(&val);
        if (doubleValue != nullptr) {
            Write(buffer, TAG_DOUBLE, *doubleValue);
            continue;
        }
        auto* u8ArrayValue = std::get_if<std::vector<uint8_t>>(&val);
        if (u8ArrayValue != nullptr) {
            Write(buffer, TAG_MAP_VALUE, *u8ArrayValue);
            continue;
        }
    }
    WriteHead(buffer, type, tagCursor, cursor_ - valueCursor);
    return true;
}

bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, TLVObject &value)
{
    if (!HasExpectBuffer(buffer, sizeof(TLVHead))) {
        return false;
    }
    auto tagCursor = cursor_;
    cursor_ += sizeof(TLVHead);
    auto valueCursor = cursor_;
    bool ret = value.Encode(buffer, cursor_, buffer.size());
    WriteHead(buffer, type, tagCursor, cursor_ - valueCursor);
    return ret;
}
bool TLVObject::Write(std::vector<std::uint8_t> &buffer, uint16_t type, std::vector<uint8_t> &value)
{
    if (!HasExpectBuffer(buffer, sizeof(TLVHead) + value.size())) {
        return false;
    }
    WriteHead(buffer, type, cursor_, value.size());
    cursor_ += sizeof(TLVHead);

    if (!value.empty()) {
        auto err = memcpy_s(buffer.data() + cursor_, buffer.size() - cursor_, value.data(), value.size());
        if (err != EOK) {
            return false;
        }
    }
    cursor_ += value.size();
    return true;
}
bool TLVObject::ReadHead(const std::vector<std::uint8_t> &buffer, TLVHead &head)
{
    if (!HasExpectBuffer(buffer, sizeof(TLVHead))) {
        return false;
    }
    const auto *pHead = reinterpret_cast<const TLVHead *>(buffer.data() + cursor_);
    if (!HasExpectBuffer(buffer, NetToHost(pHead->len)) &&
        !HasExpectBuffer(buffer, NetToHost(pHead->len) + sizeof(TLVHead))) {
        return false;
    }
    head.tag = NetToHost(pHead->tag);
    head.len = NetToHost(pHead->len);
    cursor_ += sizeof(TLVHead);
    return true;
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, bool &value, const TLVHead &head)
{
    return ReadBasicValue(buffer, value, head);
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, int8_t &value, const TLVHead &head)
{
    return ReadBasicValue(buffer, value, head);
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, int16_t &value, const TLVHead &head)
{
    return ReadBasicValue(buffer, value, head);
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, int32_t &value, const TLVHead &head)
{
    return ReadBasicValue(buffer, value, head);
}

bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, int64_t &value, const TLVHead &head)
{
    return ReadBasicValue(buffer, value, head);
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, double &value, const TLVHead &head)
{
    return ReadBasicValue(buffer, value, head);
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, uint32_t &value, const TLVHead &head)
{
    return ReadBasicValue(buffer, value, head);
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, std::string &value, const TLVHead &head)
{
    if (!HasExpectBuffer(buffer, head.len)) {
        return false;
    }
    value.append(reinterpret_cast<const char *>(buffer.data() + cursor_), head.len);
    cursor_ += head.len;
    return true;
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, RawMem &rawMem, const TLVHead &head)
{
    if (!HasExpectBuffer(buffer, head.len)) {
        return false;
    }
    rawMem.buffer = (uintptr_t)(buffer.data() + cursor_);
    rawMem.bufferLen = head.len;
    cursor_ += head.len;
    return true;
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, TLVObject &value, const TLVHead &head)
{
    return value.Decode(buffer, cursor_, cursor_ + head.len);
}
bool TLVObject::ReadValue(const std::vector<std::uint8_t> &buffer, std::vector<uint8_t> &value, const TLVHead &head)
{
    if (!HasExpectBuffer(buffer, head.len)) {
        return false;
    }
    std::vector<uint8_t> buff(buffer.data() + cursor_, buffer.data() + cursor_ + head.len);
    value = std::move(buff);
    cursor_ += head.len;
    return true;
}
bool TLVObject::ReadValue(
    const std::vector<std::uint8_t> &buffer, std::map<std::string, std::vector<uint8_t>> &value, const TLVHead &head)
{
    auto mapEnd = cursor_ + head.len;
    for (; cursor_ < mapEnd;) {
        // item key
        TLVHead keyHead{};
        bool ret = ReadHead(buffer, keyHead);
        std::string itemKey;
        ret = ret && ReadValue(buffer, itemKey, keyHead);

        // item value
        TLVHead valueHead{};
        ret = ret && ReadHead(buffer, valueHead);
        std::vector<uint8_t> itemValue(0);
        ret = ret && ReadValue(buffer, itemValue, valueHead);
        if (!ret) {
            return false;
        }
        value.emplace(itemKey, itemValue);
    }
    return true;
}

bool TLVObject::ReadValue(const std::vector<std::uint8_t>& buffer, Details& value, const TLVHead& head)
{
    auto mapEnd = cursor_ + head.len;
    while (cursor_ < mapEnd) {
        TLVHead keyHead{};
        bool ret = ReadHead(buffer, keyHead);
        std::string itemKey = "";
        if (!ReadValue(buffer, itemKey, keyHead)) {
            return false;
        }
        TLVHead valueHead{};
        ret = ReadHead(buffer, valueHead);
        ValueType itemValue;
        if (!ReadValue(buffer, itemValue, valueHead)) {
            return false;
        }
        value.emplace(itemKey, itemValue);
    }
    return true;
}

bool TLVObject::ReadValue(const std::vector<std::uint8_t>& buffer, ValueType& value, const TLVHead& head)
{
    switch (head.tag) {
        case TAG_STRING: {
            std::string val;
            if (!ReadValue(buffer, val, head)) {
                return false;
            }
            value = std::move(val);
            return true;
        }
        case TAG_INT32: {
            int32_t val;
            if (!ReadValue(buffer, val, head)) {
                return false;
            }
            value = val;
            return true;
        }
        case TAG_INT64: {
            int64_t val;
            if (!ReadValue(buffer, val, head)) {
                return false;
            }
            value = val;
            return true;
        }
        case TAG_BOOL: {
            bool val;
            if (!ReadValue(buffer, val, head)) {
                return false;
            }
            value = val;
            return true;
        }
        case TAG_DOUBLE: {
            double val;
            if (!ReadValue(buffer, val, head)) {
                return false;
            }
            value = val;
            return true;
        }
        case TAG_MAP_VALUE: {
            std::vector<uint8_t> val(0);
            if (!ReadValue(buffer, val, head)) {
                return false;
            }
            value = val;
            return true;
        }
        default:
            return true;
    }
}

bool TLVObject::Encode(std::vector<std::uint8_t> &buffer, size_t &cursor, size_t total)
{
    cursor_ = cursor;
    total_ = total;
    bool ret = Encode(buffer);
    cursor = cursor_;
    return ret;
}
bool TLVObject::Decode(const std::vector<std::uint8_t> &buffer, size_t &cursor, size_t total)
{
    cursor_ = cursor;
    total_ = total;
    bool ret = Decode(buffer);
    cursor = cursor_;
    return ret;
}
} // namespace OHOS::MiscServices
