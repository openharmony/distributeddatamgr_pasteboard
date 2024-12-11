/*
* Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "paste_data_entry.h"

#include "common/constant.h"
#include "pasteboard_hilog.h"
#include "pixel_map.h"
#include "tlv_object.h"
namespace OHOS {
namespace MiscServices {

enum TAG_CUSTOMDATA : uint16_t {
    TAG_ITEM_DATA = TAG_BUFF + 1,
};

enum TAG_ENTRY : uint16_t {
    TAG_ENTRY_UTDID = TAG_BUFF + 1,
    TAG_ENTRY_MIMETYPE,
    TAG_ENTRY_VALUE,
};

std::map<std::string, std::vector<uint8_t>> MineCustomData::GetItemData()
{
    return this->itemData_;
}

void MineCustomData::AddItemData(const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer)
{
    itemData_.emplace(mimeType, arrayBuffer);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "itemData_.size = %{public}zu", itemData_.size());
}

bool MineCustomData::Encode(std::vector<std::uint8_t>& buffer)
{
    return Write(buffer, TAG_ITEM_DATA, itemData_);
}

bool MineCustomData::Decode(const std::vector<std::uint8_t>& buffer)
{
    for (; IsEnough();) {
        TLVHead head{};
        bool ret = ReadHead(buffer, head);
        switch (head.tag) {
            case TAG_ITEM_DATA:
                ret = ret && ReadValue(buffer, itemData_, head);
                break;
            default:
                ret = ret && Skip(head.len, buffer.size());
                break;
        }
        if (!ret) {
            return false;
        }
    }
    return true;
}

size_t MineCustomData::Count()
{
    return TLVObject::Count(itemData_);
}

PasteDataEntry::PasteDataEntry(const PasteDataEntry& entry)
    : utdId_(entry.utdId_), mimeType_(entry.mimeType_), value_(entry.value_)
{
}

PasteDataEntry& PasteDataEntry::operator=(const PasteDataEntry& entry)
{
    if (this == &entry) {
        return *this;
    }
    this->utdId_ = entry.GetUtdId();
    this->mimeType_ = entry.GetMimeType();
    this->value_ = entry.GetValue();
    return *this;
}

PasteDataEntry::PasteDataEntry(const std::string& utdId, const EntryValue& value) : utdId_(utdId), value_(value)
{
    mimeType_ = CommonUtils::Convert2MimeType(utdId_);
}

PasteDataEntry::PasteDataEntry(const std::string& utdId, const std::string& mimeType, const EntryValue& value)
    : utdId_(utdId), mimeType_(std::move(mimeType)), value_(std::move(value))
{
}

void PasteDataEntry::SetUtdId(const std::string& utdId)
{
    utdId_ = utdId;
}

std::string PasteDataEntry::GetUtdId() const
{
    return utdId_;
}

void PasteDataEntry::SetMimeType(const std::string& mimeType)
{
    mimeType_ = mimeType;
}

std::string PasteDataEntry::GetMimeType() const
{
    return mimeType_;
}

EntryValue PasteDataEntry::GetValue() const
{
    return value_;
}

void PasteDataEntry::SetValue(const EntryValue& value)
{
    value_ = value;
}

bool PasteDataEntry::Encode(std::vector<std::uint8_t>& buffer)
{
    bool ret = Write(buffer, TAG_ENTRY_UTDID, utdId_);
    ret = ret && Write(buffer, TAG_ENTRY_MIMETYPE, mimeType_);
    ret = ret && Write(buffer, TAG_ENTRY_VALUE, value_);
    return ret;
}

bool PasteDataEntry::Decode(const std::vector<std::uint8_t>& buffer)
{
    for (; IsEnough();) {
        TLVHead head{};
        bool ret = ReadHead(buffer, head);
        switch (head.tag) {
            case TAG_ENTRY_UTDID:
                ret = ret && ReadValue(buffer, utdId_, head);
                break;
            case TAG_ENTRY_MIMETYPE: {
                ret = ret && ReadValue(buffer, mimeType_, head);
                break;
            }
            case TAG_ENTRY_VALUE: {
                ret = ret && ReadValue(buffer, value_, head);
                break;
            }
            default:
                ret = ret && Skip(head.len, buffer.size());
                break;
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "read value,tag:%{public}u, len:%{public}u, ret:%{public}d",
            head.tag, head.len, ret);
        if (!ret) {
            return false;
        }
    }
    return true;
}

bool PasteDataEntry::Marshalling(std::vector<std::uint8_t>& buffer)
{
    Init(buffer);
    return Encode(buffer);
}

bool PasteDataEntry::Unmarshalling(const std::vector<std::uint8_t>& buffer)
{
    total_ = buffer.size();
    return Decode(buffer);
}

size_t PasteDataEntry::Count()
{
    size_t expectedSize = 0;
    expectedSize += TLVObject::Count(utdId_);
    expectedSize += TLVObject::Count(mimeType_);
    expectedSize += TLVObject::Count(value_);
    return expectedSize;
}

std::shared_ptr<std::string> PasteDataEntry::ConvertToPlianText() const
{
    std::string res;
    auto utdId = GetUtdId();
    auto entry = GetValue();
    if (std::holds_alternative<std::string>(entry)) {
        res = std::get<std::string>(entry);
        return std::make_shared<std::string>(res);
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no plaintext");
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::PLAIN_TEXT)) {
        object->GetValue(UDMF::CONTENT, res);
    } else {
        object->GetValue(UDMF::URL, res);
    }
    return std::make_shared<std::string>(res);
}

std::shared_ptr<std::string> PasteDataEntry::ConvertToHtml() const
{
    std::string res;
    if (GetUtdId() != UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HTML)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", GetUtdId().c_str());
        return nullptr;
    }
    auto entry = GetValue();
    if (std::holds_alternative<std::string>(entry)) {
        res = std::get<std::string>(entry);
        return std::make_shared<std::string>(res);
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no html");
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    object->GetValue(UDMF::HTML_CONTENT, res);
    return std::make_shared<std::string>(res);
}

std::shared_ptr<Uri> PasteDataEntry::ConvertToUri() const
{
    std::string res;
    if (!CommonUtils::IsFileUri(GetUtdId())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", GetUtdId().c_str());
        return nullptr;
    }
    auto entry = GetValue();
    if (std::holds_alternative<std::string>(entry)) {
        res = std::get<std::string>(entry);
        return std::make_shared<Uri>(Uri(res));
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no uri");
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    object->GetValue(UDMF::FILE_URI_PARAM, res);
    return std::make_shared<Uri>(Uri(res));
}

std::shared_ptr<AAFwk::Want> PasteDataEntry::ConvertToWant() const
{
    if (GetUtdId() != UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::OPENHARMONY_WANT)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", GetUtdId().c_str());
        return nullptr;
    }
    auto entry = GetValue();
    if (!std::holds_alternative<std::shared_ptr<AAFwk::Want>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no plaintext");
        return nullptr;
    }
    // no uds want
    return std::get<std::shared_ptr<AAFwk::Want>>(entry);
}

std::shared_ptr<Media::PixelMap> PasteDataEntry::ConvertToPixelMap() const
{
    auto utdId = GetUtdId();
    if (utdId != UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::SYSTEM_DEFINED_PIXEL_MAP)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", utdId.c_str());
        return nullptr;
    }
    auto entry = GetValue();
    if (std::holds_alternative<std::shared_ptr<Media::PixelMap>>(entry)) {
        return std::get<std::shared_ptr<Media::PixelMap>>(entry);
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no pixelmap");
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    std::string objecType;
    if (!object->GetValue(UDMF::UNIFORM_DATA_TYPE, objecType)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", utdId.c_str());
        return nullptr;
    }
    if (objecType != UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, objecType:%{public}s", objecType.c_str());
        return nullptr;
    }
    auto val = object->value_[UDMF::PIXEL_MAP];
    if (std::holds_alternative<std::shared_ptr<Media::PixelMap>>(val)) {
        return std::get<std::shared_ptr<Media::PixelMap>>(val);
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no pixelmap");
    return nullptr;
}

std::shared_ptr<MineCustomData> PasteDataEntry::ConvertToCustomData() const
{
    auto entry = GetValue();
    MineCustomData customdata;
    if (std::holds_alternative<std::vector<uint8_t>>(entry)) {
        customdata.AddItemData(GetMimeType(), std::get<std::vector<uint8_t>>(entry));
        return std::make_shared<MineCustomData>(customdata);
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error,  utdId:%{public}s", utdId_.c_str());
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    std::string objecType;
    if (!object->GetValue(UDMF::UNIFORM_DATA_TYPE, objecType)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", utdId_.c_str());
        return nullptr;
    }
    if (objecType != GetUtdId()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type diff error, utdId:%{public}s, objecType:%{public}s",
            utdId_.c_str(), objecType.c_str());
    }
    std::vector<uint8_t> recordValue;
    if (!object->GetValue(UDMF::ARRAY_BUFFER, recordValue)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "get value error, utdId:%{public}s", utdId_.c_str());
        return nullptr;
    }
    customdata.AddItemData(utdId_, recordValue);
    return std::make_shared<MineCustomData>(customdata);
}

bool PasteDataEntry::HasContent(const std::string &utdId) const
{
    auto mimeType = CommonUtils::Convert2MimeType(utdId);
    if (mimeType == MIMETYPE_PIXELMAP) {
        return ConvertToPixelMap() != nullptr;
    } else if (mimeType == MIMETYPE_TEXT_HTML) {
        auto html = ConvertToHtml();
        return html != nullptr && !html->empty();
    } else if (mimeType == MIMETYPE_TEXT_PLAIN) {
        auto plianText = ConvertToPlianText();
        return plianText != nullptr && !plianText->empty();
    } else if (mimeType == MIMETYPE_TEXT_URI) {
        auto uri = ConvertToUri();
        return uri != nullptr && !uri->ToString().empty();
    } else if (mimeType == MIMETYPE_TEXT_WANT) {
        return ConvertToWant() != nullptr;
    } else {
        return ConvertToCustomData() != nullptr;
    }
}

std::string CommonUtils::Convert(UDType uDType)
{
    switch (uDType) {
        case UDType::PLAIN_TEXT:
        case UDType::HYPERLINK:
            return MIMETYPE_TEXT_PLAIN;
        case UDType::HTML:
            return MIMETYPE_TEXT_HTML;
        case UDType::FILE:
        case UDType::IMAGE:
        case UDType::VIDEO:
        case UDType::AUDIO:
        case UDType::FOLDER:
        case UDType::FILE_URI:
            return MIMETYPE_TEXT_URI;
        case UDType::SYSTEM_DEFINED_PIXEL_MAP:
            return MIMETYPE_PIXELMAP;
        case UDType::OPENHARMONY_WANT:
            return MIMETYPE_TEXT_WANT;
        default:
            return UDMF::UtdUtils::GetUtdIdFromUtdEnum(uDType);
    }
}

std::string CommonUtils::Convert2MimeType(const std::string& utdId)
{
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::PLAIN_TEXT) ||
        utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HYPERLINK)) {
        return MIMETYPE_TEXT_PLAIN;
    }
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HTML)) {
        return MIMETYPE_TEXT_HTML;
    }
    if (IsFileUri(utdId)) {
        return MIMETYPE_TEXT_URI;
    }
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::SYSTEM_DEFINED_PIXEL_MAP)) {
        return MIMETYPE_PIXELMAP;
    }
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::OPENHARMONY_WANT)) {
        return MIMETYPE_TEXT_WANT;
    }
    return utdId;
}

// other is appdefined-types
std::string CommonUtils::Convert2UtdId(int32_t uDType, const std::string& mimeType)
{
    if (mimeType == MIMETYPE_TEXT_PLAIN && uDType == UDMF::HYPERLINK) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HYPERLINK);
    }
    if (mimeType == MIMETYPE_TEXT_PLAIN) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::PLAIN_TEXT);
    }
    if (mimeType == MIMETYPE_TEXT_URI) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::FILE_URI);
    }
    if (mimeType == MIMETYPE_TEXT_HTML) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HTML);
    }
    if (mimeType == MIMETYPE_TEXT_WANT) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::OPENHARMONY_WANT);
    }
    if (mimeType == MIMETYPE_PIXELMAP) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::SYSTEM_DEFINED_PIXEL_MAP);
    }
    return mimeType;
}

UDMF::UDType CommonUtils::Convert(int32_t uDType, const std::string& mimeType)
{
    if (uDType != UDMF::UD_BUTT) {
        return static_cast<UDType>(uDType);
    }
    if (mimeType == MIMETYPE_TEXT_URI) {
        return UDMF::FILE_URI;
    }
    if (mimeType == MIMETYPE_TEXT_PLAIN) {
        return UDMF::PLAIN_TEXT;
    }
    if (mimeType == MIMETYPE_TEXT_HTML) {
        return UDMF::HTML;
    }
    if (mimeType == MIMETYPE_TEXT_WANT) {
        return UDMF::OPENHARMONY_WANT;
    }
    if (mimeType == MIMETYPE_PIXELMAP) {
        return UDMF::SYSTEM_DEFINED_PIXEL_MAP;
    }
    auto type = UDMF::UtdUtils::GetUtdEnumFromUtdId(mimeType);
    if (type != UDMF::UD_BUTT) {
        return static_cast<UDType>(type);
    }
    return UDMF::APPLICATION_DEFINED_RECORD;
}

bool CommonUtils::IsFileUri(const std::string &utdId)
{
    return utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::FILE_URI) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::FILE) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::AUDIO) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::IMAGE) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::FOLDER) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::VIDEO);
}
} // namespace MiscServices
} // namespace OHOS