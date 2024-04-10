/*
* Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
#include "pasteboard_utils.h"

#include "data/application_defined_record.h"
#include "data/file.h"
#include "data/html.h"
#include "log_print.h"
#include "paste_data_record.h"
#include "pixel_map.h"
#include "data/plain_text.h"
#include "data/system_defined_pixelmap.h"
#include "data/unified_record.h"
namespace OHOS {
namespace MiscServices {
using UnifiedRecord = UDMF::UnifiedRecord;
using UnifiedData = UDMF::UnifiedData;
using UnifiedDataProperties = UDMF::UnifiedDataProperties;
using UDType = UDMF::UDType;

std::shared_ptr<PasteData> PasteboardUtils::ConvertData(UnifiedData& unifiedData)
{
    // 1.ConvertRecords
    auto unifiedRecords = unifiedData.GetRecords();
    auto pasteData = std::make_shared<PasteData>(ConvertRecords(unifiedRecords));
    // 2.ConvertProperties
    auto unifiedDataProperties = unifiedData.GetProperties();
    auto properties = ConvertProperties(*unifiedDataProperties);
    auto recordTypes = unifiedData.GetUDTypes();
    properties.mimeTypes = ConvertTypes(recordTypes);
    pasteData->SetProperty(properties);
    return pasteData;
}

std::shared_ptr<UnifiedData> PasteboardUtils::ConvertData(PasteData& pasteData)
{
    std::shared_ptr<UnifiedData> unifiedData = std::make_shared<UnifiedData>();
    // 1.ConvertRecords
    for (std::size_t i = 0; i < pasteData.GetRecordCount(); ++i) {
        auto pasteboardRecord = pasteData.GetRecordAt(i);
        auto type = pasteboardRecord->GetMimeType();
        if (type != MIMETYPE_TEXT_PLAIN && type != MIMETYPE_TEXT_WANT && type != MIMETYPE_PIXELMAP &&
            type != MIMETYPE_TEXT_HTML && type != MIMETYPE_TEXT_URI) {
            unifiedData->AddRecords(ConvertRecords(pasteboardRecord));
        }
        unifiedData->AddRecord(ConvertRecord(pasteboardRecord));
    }
    // 2.ConvertProperties
    auto pastedProp = pasteData.GetProperty();
    auto unifiedProp = ConvertProperties(pastedProp);
    unifiedData->SetProperties(unifiedProp);
    return unifiedData;
}

std::vector<std::shared_ptr<PasteDataRecord>> PasteboardUtils::ConvertRecords(
    std::vector<std::shared_ptr<UnifiedRecord>>& records)
{
    std::vector<std::shared_ptr<PasteDataRecord>> pasteboardRecords;
    for (auto& record : records) {
        auto pasteRecord = ConvertRecord(record);
        if (pasteRecord == nullptr) {
            continue;
        }
        pasteboardRecords.push_back(ConvertRecord(record));
    }
    return pasteboardRecords;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::ConvertRecord(std::shared_ptr<UnifiedRecord> record)
{
    auto type = record->GetType();
    switch (type) {
        case UDType::PLAIN_TEXT: {
            auto plainText = static_cast<UDMF::PlainText*>(record.get());
            if (plainText == nullptr) {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get PLAIN_TEXT record field.");
                return nullptr;
            }
            return PasteDataRecord::NewPlaintTextRecord(plainText->GetContent());
        }
        case UDType::HTML: {
            auto html = static_cast<UDMF::Html*>(record.get());
            if (html == nullptr) {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get HTML record field.");
                return nullptr;
            }
            return PasteDataRecord::NewHtmlRecord(html->GetHtmlContent());
        }
        case UDType::FILE:
        case UDType::IMAGE:
        case UDType::VIDEO:
        case UDType::AUDIO:
        case UDType::FOLDER: {
            auto file = static_cast<UDMF::File*>(record.get());
            if (file == nullptr) {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get file record field.");
                return nullptr;
            }
            return PasteDataRecord::NewUriRecord(OHOS::Uri(file->GetUri()));
        }
        case UDType::SYSTEM_DEFINED_PIXEL_MAP: {
            auto pixelMap = static_cast<UDMF::SystemDefinedPixelMap*>(record.get());
            if (pixelMap == nullptr) {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get pixel_map record field.");
                return nullptr;
            }
            auto rawData = pixelMap->GetRawData();
            return PasteDataRecord::NewPixelMapRecord(PasteDataRecord::Vector2PixelMap(rawData));
        }
        case UDType::APPLICATION_DEFINED_RECORD: {
            auto appRecord = static_cast<UDMF::ApplicationDefinedRecord*>(record.get());
            if (appRecord == nullptr) {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get ApplicationDefinedRecord record field.");
                return nullptr;
            }
            return PasteDataRecord::NewKvRecord(appRecord->GetApplicationDefinedType(), appRecord->GetRawData());
        }
        default:
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get unified record, UDType:%{public}d", type);
            return nullptr;
    }
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::ConvertRecord(std::shared_ptr<PasteDataRecord> record)
{
    auto type = record->GetMimeType();
    if (type == MIMETYPE_TEXT_URI) {
        auto uriStr = record->GetUri()->ToString();
        return std::make_shared<UDMF::File>(uriStr);
    }
    if (type == MIMETYPE_TEXT_PLAIN) {
        auto content = *(record->GetPlainText());
        return std::make_shared<UDMF::PlainText>(content, "");
    }
    if (type == MIMETYPE_TEXT_HTML) {
        auto content = *(record->GetHtmlText());
        return std::make_shared<UDMF::Html>(content, "");
    }
    if (type == MIMETYPE_TEXT_WANT) {
        auto content = *(record->GetPlainText());
        return std::make_shared<UDMF::PlainText>(content, "");
    }
    if (type == MIMETYPE_PIXELMAP) {
        auto content = record->GetPixelMap();
        auto rawData = PasteDataRecord::PixelMap2Vector(content);
        return std::make_shared<UDMF::SystemDefinedPixelMap>(rawData);
    }
    return nullptr;
}

std::vector<std::shared_ptr<UnifiedRecord>> PasteboardUtils::ConvertRecords(std::shared_ptr<PasteDataRecord> record)
{
    std::vector<std::shared_ptr<UnifiedRecord>> unifiedRecords;
    auto customData = record->GetCustomData();
    for (auto& [type, rawData] : customData->GetItemData()) {
        unifiedRecords.push_back(std::make_shared<UDMF::ApplicationDefinedRecord>(type, rawData));
    }
    return unifiedRecords;
}

PasteDataProperty PasteboardUtils::ConvertProperties(UnifiedDataProperties& properties)
{
    PasteDataProperty pasteDataProperty;
    pasteDataProperty.shareOption = static_cast<ShareOption>(properties.shareOption);
    pasteDataProperty.additions = properties.extras;
    pasteDataProperty.timestamp = properties.timestamp;
    pasteDataProperty.tag = properties.tag;
    return PasteDataProperty(pasteDataProperty);
}

std::shared_ptr<UnifiedDataProperties> PasteboardUtils::ConvertProperties(PasteDataProperty& properties)
{
    std::shared_ptr<UnifiedDataProperties> unifiedDataProperties = std::make_shared<UnifiedDataProperties>();
    unifiedDataProperties->shareOption = properties.shareOption == ShareOption::InApp ? UDMF::ShareOption::IN_APP : UDMF::ShareOption::CROSS_APP;
    unifiedDataProperties->extras = properties.additions;
    unifiedDataProperties->timestamp = properties.timestamp;
    unifiedDataProperties->tag = properties.tag;
    return unifiedDataProperties;
}

std::vector<std::string> PasteboardUtils::ConvertTypes(std::vector<UDType> &uDTypes)
{
    std::vector<std::string> types;
    for (auto udType : uDTypes) {
        types.push_back(ConvertType(udType));
    }
    return types;
}

std::string PasteboardUtils::ConvertType(UDType uDType)
{
    switch (uDType) {
        case UDType::PLAIN_TEXT: return MIMETYPE_TEXT_PLAIN;
        case UDType::HTML: return MIMETYPE_TEXT_HTML;
        case UDType::FILE:
        case UDType::IMAGE:
        case UDType::VIDEO:
        case UDType::AUDIO:
        case UDType::FOLDER: return MIMETYPE_TEXT_URI;
        case UDType::SYSTEM_DEFINED_PIXEL_MAP: return MIMETYPE_PIXELMAP;
        default:
            return "";
    }
}
} // namespace MiscServices
} // namespace OHOS