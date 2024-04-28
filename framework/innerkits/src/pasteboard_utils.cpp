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
#include "pasteboard_utils.h"

#include "application_defined_record.h"
#include "audio.h"
#include "folder.h"
#include "html.h"
#include "image.h"
#include "link.h"
#include "paste_data_record.h"
#include "pixel_map.h"
#include "plain_text.h"
#include "system_defined_appitem.h"
#include "system_defined_form.h"
#include "unified_record.h"
#include "video.h"
namespace OHOS {
namespace MiscServices {
using UnifiedRecord = UDMF::UnifiedRecord;
using UnifiedData = UDMF::UnifiedData;
using UnifiedDataProperties = UDMF::UnifiedDataProperties;
using UDType = UDMF::UDType;

void PasteboardUtils::InitDecodeMap()
{
    convert2URecordMap_ = {
        { UDMF::TEXT, PasteRecord2Text },
        { UDMF::PLAIN_TEXT, PasteRecord2PlaintText },
        { UDMF::OPENHARMONY_WANT, PasteRecord2Want },
        { UDMF::HTML, PasteRecord2Html },
        { UDMF::HYPERLINK, PasteRecord2Link },
        { UDMF::FILE, PasteRecord2File },
        { UDMF::IMAGE, PasteRecord2Image },
        { UDMF::VIDEO, PasteRecord2Video },
        { UDMF::AUDIO, PasteRecord2Audio },
        { UDMF::FOLDER, PasteRecord2Folder },
        { UDMF::SYSTEM_DEFINED_PIXEL_MAP, PasteRecord2PixelMap },
        { UDMF::SYSTEM_DEFINED_RECORD, PasteRecord2SystemDefined },
        { UDMF::SYSTEM_DEFINED_FORM, PasteRecord2Form },
        { UDMF::SYSTEM_DEFINED_APP_ITEM, PasteRecord2AppItem },
    };

    convert2PRecordMap_ = {
        { UDMF::TEXT, Text2PasteRecord },
        { UDMF::PLAIN_TEXT, PlainText2PasteRecord },
        { UDMF::OPENHARMONY_WANT, Want2PasteRecord },
        { UDMF::HTML, Html2PasteRecord },
        { UDMF::HYPERLINK, Link2PasteRecord },
        { UDMF::FILE, File2PasteRecord },
        { UDMF::IMAGE, Image2PasteRecord },
        { UDMF::VIDEO, Video2PasteRecord },
        { UDMF::AUDIO, Audio2PasteRecord },
        { UDMF::FOLDER, Folder2PasteRecord },
        { UDMF::SYSTEM_DEFINED_PIXEL_MAP, PixelMap2PasteRecord },
        { UDMF::SYSTEM_DEFINED_RECORD, SystemDefined2PasteRecord },
        { UDMF::SYSTEM_DEFINED_FORM, Form2PasteRecord },
        { UDMF::SYSTEM_DEFINED_APP_ITEM, AppItem2PasteRecord },
        { UDMF::APPLICATION_DEFINED_RECORD, AppDefined2PasteRecord },
    };
}

std::shared_ptr<PasteData> PasteboardUtils::Convert(const UnifiedData& unifiedData)
{
    auto unifiedRecords = unifiedData.GetRecords();
    auto pasteData = std::make_shared<PasteData>(Convert(unifiedRecords));
    auto unifiedDataProperties = unifiedData.GetProperties();
    auto properties = Convert(*unifiedDataProperties);
    auto recordTypes = unifiedData.GetUDTypes();
    properties.mimeTypes = Convert(recordTypes);
    pasteData->SetProperty(properties);
    return pasteData;
}

std::shared_ptr<UnifiedData> PasteboardUtils::Convert(const PasteData& pasteData)
{
    auto unifiedData = std::make_shared<UnifiedData>();
    for (std::size_t i = 0; i < pasteData.GetRecordCount(); ++i) {
        auto pasteboardRecord = pasteData.GetRecordAt(i);
        if (pasteboardRecord == nullptr) {
            continue;
        }
        auto type = Convert(pasteboardRecord->GetUDType(), pasteboardRecord->GetMimeType());
        auto it = convert2URecordMap_.find(type);
        if (it == convert2URecordMap_.end()) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "not find type, go to customData");
            unifiedData->AddRecords(Custom2AppDefined(pasteboardRecord));
        } else {
            unifiedData->AddRecord(it->second(pasteboardRecord));
        }
    }
    auto pastedProp = pasteData.GetProperty();
    auto unifiedProp = Convert(pastedProp);
    unifiedData->SetProperties(unifiedProp);
    return unifiedData;
}

std::vector<std::shared_ptr<PasteDataRecord>> PasteboardUtils::Convert(
    const std::vector<std::shared_ptr<UnifiedRecord>>& records)
{
    std::vector<std::shared_ptr<PasteDataRecord>> pasteboardRecords;
    for (auto const& record : records) {
        if (record == nullptr) {
            continue;
        }
        auto type = record->GetType();
        auto it = convert2PRecordMap_.find(type);
        if (it == convert2PRecordMap_.end()) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "not find type, convert to AppDefinedRecord");
            pasteboardRecords.push_back(AppDefined2PasteRecord(record));
        } else {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "find type, convert to pasteRecord, type:%{public}d", type);
            pasteboardRecords.push_back(it->second(record));
        }
    }
    return pasteboardRecords;
}

PasteDataProperty PasteboardUtils::Convert(const UnifiedDataProperties& properties)
{
    PasteDataProperty pasteDataProperty;
    pasteDataProperty.shareOption = static_cast<ShareOption>(properties.shareOptions);
    pasteDataProperty.additions = properties.extras;
    pasteDataProperty.timestamp = properties.timestamp;
    pasteDataProperty.tag = properties.tag;
    return PasteDataProperty(pasteDataProperty);
}

std::shared_ptr<UnifiedDataProperties> PasteboardUtils::Convert(const PasteDataProperty& properties)
{
    auto unifiedDataProperties = std::make_shared<UnifiedDataProperties>();
    unifiedDataProperties->shareOptions = properties.shareOption == ShareOption::InApp ? UDMF::ShareOptions::IN_APP
                                                                                      : UDMF::ShareOptions::CROSS_APP;
    unifiedDataProperties->extras = properties.additions;
    unifiedDataProperties->timestamp = properties.timestamp;
    unifiedDataProperties->tag = properties.tag;
    return unifiedDataProperties;
}

std::vector<std::string> PasteboardUtils::Convert(const std::vector<UDType>& uDTypes)
{
    std::vector<std::string> types;
    for (const auto& udType : uDTypes) {
        types.push_back(Convert(udType));
    }
    return types;
}

std::string PasteboardUtils::Convert(UDType uDType)
{
    switch (uDType) {
        case UDType::PLAIN_TEXT:
            return MIMETYPE_TEXT_PLAIN;
        case UDType::HTML:
            return MIMETYPE_TEXT_HTML;
        case UDType::FILE:
        case UDType::IMAGE:
        case UDType::VIDEO:
        case UDType::AUDIO:
        case UDType::FOLDER:
            return MIMETYPE_TEXT_URI;
        case UDType::SYSTEM_DEFINED_PIXEL_MAP:
            return MIMETYPE_PIXELMAP;
        case UDType::OPENHARMONY_WANT:
            return MIMETYPE_TEXT_WANT;
        default:
            return UDMF::UD_TYPE_MAP.at(uDType);
    }
}

UDType PasteboardUtils::Convert(int32_t uDType, const std::string& mimeType)
{
    if (uDType != UDMF::UD_BUTT) {
        return static_cast<UDType>(uDType);
    }
    if (mimeType == MIMETYPE_TEXT_URI) {
        return UDMF::FILE;
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
    for (const auto& [type, typeString] : UDMF::UD_TYPE_MAP) {
        if (mimeType == typeString) {
            return static_cast<UDType>(type);
        }
    }
    return UDMF::UD_BUTT;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::PlainText2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto plainText = static_cast<UDMF::PlainText*>(record.get());
    if (plainText == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get PLAIN_TEXT record field.");
        return nullptr;
    }
    auto plainTextRecord = PasteDataRecord::NewPlaintTextRecord(plainText->GetContent());
    plainTextRecord->SetDetails(plainText->GetDetails());
    plainTextRecord->SetTextContent(plainText->GetAbstract());
    plainTextRecord->SetUDType(UDMF::PLAIN_TEXT);
    return plainTextRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2PlaintText(const std::shared_ptr<PasteDataRecord> record)
{
    auto plaint = record->GetPlainText();
    if (plaint == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get plain from paste record to plain field.");
        return nullptr;
    }
    auto unifiedRecord = std::make_shared<UDMF::PlainText>(*plaint, record->GetTextContent());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Want2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto want = static_cast<UDMF::UnifiedRecord*>(record.get());
    if (want == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get want record field.");
        return nullptr;
    }
    auto recordValue = want->GetValue();
    auto wantValue = std::get_if<std::shared_ptr<OHOS::AAFwk::Want>>(&recordValue);
    if (wantValue == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get want from unified record field.");
        return nullptr;
    }
    auto wantRecord = PasteDataRecord::NewWantRecord(*(wantValue));
    wantRecord->SetUDType(UDMF::OPENHARMONY_WANT);
    return wantRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Want(const std::shared_ptr<PasteDataRecord> record)
{
    auto wantRecord = record->GetWant();
    if (wantRecord == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "get want from paste record field.");
        return nullptr;
    }
    return std::make_shared<UDMF::UnifiedRecord>(UDMF::OPENHARMONY_WANT, wantRecord);
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Html2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto html = static_cast<UDMF::Html*>(record.get());
    if (html == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get HTML record field.");
        return nullptr;
    }
    auto htmlRecord = PasteDataRecord::NewHtmlRecord(html->GetHtmlContent());
    htmlRecord->SetTextContent(html->GetPlainContent());
    htmlRecord->SetUDType(UDMF::HTML);
    htmlRecord->SetDetails(html->GetDetails());
    return htmlRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Html(const std::shared_ptr<PasteDataRecord> record)
{
    auto html = record->GetHtmlText();
    if (html == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get HTML from paste record field.");
        return nullptr;
    }
    auto unifiedRecord = std::make_shared<UDMF::Html>(*(record->GetHtmlText()), record->GetTextContent());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Link2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto link = static_cast<UDMF::Link*>(record.get());
    if (link == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get Link record field.");
        return nullptr;
    }
    auto plainTextRecord = PasteDataRecord::NewPlaintTextRecord(link->GetUrl());
    plainTextRecord->SetDetails(link->GetDetails());
    plainTextRecord->SetTextContent(link->GetDescription());
    plainTextRecord->SetUDType(UDMF::HYPERLINK);
    return plainTextRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Link(const std::shared_ptr<PasteDataRecord> record)
{
    auto plaint = record->GetPlainText();
    if (plaint == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get plain from paste record to link field.");
        return nullptr;
    }
    auto unifiedRecord = std::make_shared<UDMF::Link>(*plaint, record->GetTextContent());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::File2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto file = static_cast<UDMF::File*>(record.get());
    if (file == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get file record field.");
        return nullptr;
    }
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(file->GetUri()));
    uriRecord->SetDetails(file->GetDetails());
    uriRecord->SetUDType(UDMF::FILE);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2File(const std::shared_ptr<PasteDataRecord> record)
{
    auto uri = record->GetUri();
    if (uri == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get uri from paste record to file field.");
        return nullptr;
    }
    auto unifiedRecord = std::make_shared<UDMF::File>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Image2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto image = static_cast<UDMF::Image*>(record.get());
    if (image == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get image record field.");
        return nullptr;
    }
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(image->GetUri()));
    uriRecord->SetDetails(image->GetDetails());
    uriRecord->SetUDType(UDMF::IMAGE);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Image(const std::shared_ptr<PasteDataRecord> record)
{
    auto uri = record->GetUri();
    if (uri == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get uri from paste record to image field.");
        return nullptr;
    }
    auto unifiedRecord = std::make_shared<UDMF::Image>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Video2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto video = static_cast<UDMF::Video*>(record.get());
    if (video == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get video record field.");
        return nullptr;
    }
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(video->GetUri()));
    uriRecord->SetDetails(video->GetDetails());
    uriRecord->SetUDType(UDMF::VIDEO);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Video(const std::shared_ptr<PasteDataRecord> record)
{
    auto uri = record->GetUri();
    if (uri == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get uri from paste record to video field.");
        return nullptr;
    }
    auto unifiedRecord = std::make_shared<UDMF::Video>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Audio2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto audio = static_cast<UDMF::Audio*>(record.get());
    if (audio == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get audio record field.");
        return nullptr;
    }
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(audio->GetUri()));
    uriRecord->SetDetails(audio->GetDetails());
    uriRecord->SetUDType(UDMF::AUDIO);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Audio(const std::shared_ptr<PasteDataRecord> record)
{
    auto uri = record->GetUri();
    if (uri == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get uri from paste record to audio field.");
        return nullptr;
    }
    auto unifiedRecord = std::make_shared<UDMF::Audio>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Folder2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto folder = static_cast<UDMF::Folder*>(record.get());
    if (folder == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get folder record field.");
        return nullptr;
    }
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(folder->GetUri()));
    uriRecord->SetDetails(folder->GetDetails());
    uriRecord->SetUDType(UDMF::FOLDER);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Folder(const std::shared_ptr<PasteDataRecord> record)
{
    auto uri = record->GetUri();
    if (uri == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get uri from paste record to folder field.");
        return nullptr;
    }
    auto unifiedRecord = std::make_shared<UDMF::Folder>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::PixelMap2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto pixelMap = static_cast<UDMF::UnifiedRecord*>(record.get());
    if (pixelMap == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get pixelMap record field.");
        return nullptr;
    }
    auto recordValue = pixelMap->GetValue();
    auto pixelMapValue = std::get_if<std::shared_ptr<Media::PixelMap>>(&recordValue);
    if (pixelMapValue == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get pixelMap from unified record field.");
        return nullptr;
    }
    auto pixelMapRecord = PasteDataRecord::NewPixelMapRecord(*(pixelMapValue));
    pixelMapRecord->SetUDType(UDMF::SYSTEM_DEFINED_PIXEL_MAP);
    return pixelMapRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2PixelMap(const std::shared_ptr<PasteDataRecord> record)
{
    auto pixelMapRecord = record->GetPixelMap();
    if (pixelMapRecord == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "get pixelMap from paste record field.");
        return nullptr;
    }
    return std::make_shared<UDMF::UnifiedRecord>(UDMF::SYSTEM_DEFINED_PIXEL_MAP, pixelMapRecord);
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::AppItem2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto appItem = static_cast<UDMF::SystemDefinedAppItem*>(record.get());
    if (appItem == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get SystemDefinedAppItem record field.");
        return nullptr;
    }

    std::vector<uint8_t> arrayBuffer;
    auto kvRecord = PasteDataRecord::NewKvRecord(Convert(UDType::SYSTEM_DEFINED_APP_ITEM), arrayBuffer);
    kvRecord->SetDetails(appItem->GetDetails());
    kvRecord->SetSystemDefinedContent(appItem->GetItems());
    kvRecord->SetUDType(UDType::SYSTEM_DEFINED_APP_ITEM);
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2AppItem(const std::shared_ptr<PasteDataRecord> record)
{
    auto unifiedRecord = std::make_shared<UDMF::SystemDefinedAppItem>();
    if (record->GetSystemDefinedContent() != nullptr) {
        unifiedRecord->SetItems(*record->GetSystemDefinedContent());
    }
    if (record->GetDetails() != nullptr) {
        unifiedRecord->SetDetails(*record->GetDetails());
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Form2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto form = static_cast<UDMF::SystemDefinedForm*>(record.get());
    if (form == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get SystemDefinedForm record field.");
        return nullptr;
    }
    std::vector<uint8_t> arrayBuffer;
    auto kvRecord = PasteDataRecord::NewKvRecord(Convert(UDType::SYSTEM_DEFINED_FORM), arrayBuffer);
    kvRecord->SetDetails(form->GetDetails());
    kvRecord->SetSystemDefinedContent(form->GetItems());
    kvRecord->SetUDType(UDType::SYSTEM_DEFINED_FORM);
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Form(const std::shared_ptr<PasteDataRecord> record)
{
    auto unifiedRecord = std::make_shared<UDMF::SystemDefinedForm>();
    if (record->GetSystemDefinedContent() != nullptr) {
        unifiedRecord->SetItems(*record->GetSystemDefinedContent());
    }
    if (record->GetDetails() != nullptr) {
        unifiedRecord->SetDetails(*record->GetDetails());
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::SystemDefined2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto systemDefined = static_cast<UDMF::SystemDefinedRecord*>(record.get());
    if (systemDefined == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get systemRecord record field.");
        return nullptr;
    }
    std::vector<uint8_t> arrayBuffer;
    auto kvRecord = PasteDataRecord::NewKvRecord(Convert(UDType::SYSTEM_DEFINED_RECORD), arrayBuffer);
    kvRecord->SetDetails(systemDefined->GetDetails());
    kvRecord->SetUDType(UDType::SYSTEM_DEFINED_RECORD);
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2SystemDefined(const std::shared_ptr<PasteDataRecord> record)
{
    auto unifiedRecord = std::make_shared<UDMF::SystemDefinedRecord>();
    if (record->GetDetails() != nullptr) {
        unifiedRecord->SetDetails(*record->GetDetails());
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Text2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto text = static_cast<UDMF::Text*>(record.get());
    if (text == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get TEXT record field.");
        return nullptr;
    }
    std::vector<uint8_t> arrayBuffer;
    std::string type = UDMF::UD_TYPE_MAP.at(UDMF::TEXT);
    auto kvRecord = PasteDataRecord::NewKvRecord(type, arrayBuffer);
    kvRecord->SetUDType(UDMF::TEXT);
    kvRecord->SetDetails(text->GetDetails());
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Text(std::shared_ptr<PasteDataRecord> record)
{
    auto unifiedRecord = std::make_shared<UDMF::Text>();
    if (record->GetDetails() != nullptr) {
        unifiedRecord->SetDetails(*record->GetDetails());
    }
    return unifiedRecord;
}

std::vector<std::shared_ptr<UnifiedRecord>> PasteboardUtils::Custom2AppDefined(
    const std::shared_ptr<PasteDataRecord> record)
{
    std::vector<std::shared_ptr<UnifiedRecord>> unifiedRecords;
    auto customData = record->GetCustomData();
    for (auto& [type, rawData] : customData->GetItemData()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "app defied type:%{public}s.", type.c_str());
        unifiedRecords.push_back(std::make_shared<UDMF::ApplicationDefinedRecord>(type, rawData));
    }
    return unifiedRecords;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::AppDefined2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto appRecord = static_cast<UDMF::ApplicationDefinedRecord*>(record.get());
    if (appRecord == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get ApplicationDefinedRecord record field.");
        return nullptr;
    }
    auto type = appRecord->GetApplicationDefinedType();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "custom type:%{public}s.", type.c_str());
    auto kvRecord = PasteDataRecord::NewKvRecord(type, appRecord->GetRawData());
    kvRecord->SetUDType(appRecord->GetType());
    return kvRecord;
}

PasteboardUtils::PasteboardUtils()
{
    InitDecodeMap();
}

PasteboardUtils& PasteboardUtils::GetInstance()
{
    static PasteboardUtils instance;
    return instance;
}
} // namespace MiscServices
} // namespace OHOS