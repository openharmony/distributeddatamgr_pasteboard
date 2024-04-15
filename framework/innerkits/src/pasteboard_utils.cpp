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

#include "application_defined_record.h"
#include "audio.h"
#include "file.h"
#include "folder.h"
#include "html.h"
#include "image.h"
#include "link.h"
#include "log_print.h"
#include "paste_data_record.h"
#include "pixel_map.h"
#include "plain_text.h"
#include "system_defined_appitem.h"
#include "system_defined_form.h"
#include "system_defined_pixelmap.h"
#include "system_defined_record.h"
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
    pasteRecords2UnifiedRecordMap_ = {
        { UDMF::TEXT, (PasteRecord2UnifiedRecordFunc)PasteRecord2Text },
        { UDMF::PLAIN_TEXT, (PasteRecord2UnifiedRecordFunc)PasteRecord2PlaintText },
        { UDMF::OPENHARMONY_WANT, (PasteRecord2UnifiedRecordFunc)PasteRecord2Want },
        { UDMF::HTML, (PasteRecord2UnifiedRecordFunc)PasteRecord2Html },
        { UDMF::HYPERLINK, (PasteRecord2UnifiedRecordFunc)PasteRecord2Link },
        { UDMF::FILE, (PasteRecord2UnifiedRecordFunc)PasteRecord2File },
        { UDMF::IMAGE, (PasteRecord2UnifiedRecordFunc)PasteRecord2Image },
        { UDMF::VIDEO, (PasteRecord2UnifiedRecordFunc)PasteRecord2Video },
        { UDMF::AUDIO, (PasteRecord2UnifiedRecordFunc)PasteRecord2Audio },
        { UDMF::FOLDER, (PasteRecord2UnifiedRecordFunc)PasteRecord2Folder },
        { UDMF::SYSTEM_DEFINED_PIXEL_MAP, (PasteRecord2UnifiedRecordFunc)PasteRecord2PixelMap },
        { UDMF::SYSTEM_DEFINED_RECORD, (PasteRecord2UnifiedRecordFunc)PasteRecord2SystemDefined },
        { UDMF::SYSTEM_DEFINED_FORM, (PasteRecord2UnifiedRecordFunc)PasteRecord2Form },
        { UDMF::SYSTEM_DEFINED_APP_ITEM, (PasteRecord2UnifiedRecordFunc)PasteRecord2AppItem },
    };

    unifiedRecord2PasteRecordsMap_ = {
        { UDMF::TEXT, (UnifiedRecord2PasteRecordFunc)Text2PasteRecord },
        { UDMF::PLAIN_TEXT, (UnifiedRecord2PasteRecordFunc)PlainText2PasteRecord },
        { UDMF::OPENHARMONY_WANT, (UnifiedRecord2PasteRecordFunc)Want2PasteRecord },
        { UDMF::HTML, (UnifiedRecord2PasteRecordFunc)Html2PasteRecord },
        { UDMF::HYPERLINK, (UnifiedRecord2PasteRecordFunc)Link2PasteRecord },
        { UDMF::FILE, (UnifiedRecord2PasteRecordFunc)File2PasteRecord },
        { UDMF::IMAGE, (UnifiedRecord2PasteRecordFunc)Image2PasteRecord },
        { UDMF::VIDEO, (UnifiedRecord2PasteRecordFunc)Video2PasteRecord },
        { UDMF::AUDIO, (UnifiedRecord2PasteRecordFunc)Audio2PasteRecord },
        { UDMF::FOLDER, (UnifiedRecord2PasteRecordFunc)Folder2PasteRecord },
        { UDMF::SYSTEM_DEFINED_PIXEL_MAP, (UnifiedRecord2PasteRecordFunc)PixelMap2PasteRecord },
        { UDMF::SYSTEM_DEFINED_RECORD, (UnifiedRecord2PasteRecordFunc)SystemDefined2PasteRecord },
        { UDMF::SYSTEM_DEFINED_FORM, (UnifiedRecord2PasteRecordFunc)Form2PasteRecord },
        { UDMF::SYSTEM_DEFINED_APP_ITEM, (UnifiedRecord2PasteRecordFunc)AppItem2PasteRecord },
        { UDMF::APPLICATION_DEFINED_RECORD, (UnifiedRecord2PasteRecordFunc)AppDefined2PasteRecord },
    };
}

std::shared_ptr<PasteData> PasteboardUtils::UnifiedData2PasteData(UnifiedData& unifiedData)
{
    // 1.UnifiedRecords2PasteRecords
    auto unifiedRecords = unifiedData.GetRecords();
    auto pasteData = std::make_shared<PasteData>(UnifiedRecords2PasteRecords(unifiedRecords));
    // 2.UnifiedProp2PaseteProp
    auto unifiedDataProperties = unifiedData.GetProperties();
    auto properties = UnifiedProp2PaseteProp(*unifiedDataProperties);
    auto recordTypes = unifiedData.GetUDTypes();
    properties.mimeTypes = UtdTypes2PaseteTypes(recordTypes);
    pasteData->SetProperty(properties);
    return pasteData;
}

std::shared_ptr<UnifiedData> PasteboardUtils::PasteData2UnifiedData(PasteData& pasteData)
{
    std::shared_ptr<UnifiedData> unifiedData = std::make_shared<UnifiedData>();
    // 1.UnifiedRecords2PasteRecords
    for (std::size_t i = 0; i < pasteData.GetRecordCount(); ++i) {
        auto pasteboardRecord = pasteData.GetRecordAt(i);
        auto type = PaseteType2UDType(pasteboardRecord->GetUDType(), pasteboardRecord->GetMimeType());
        // 拿到对应的UDType
        auto it = pasteRecords2UnifiedRecordMap_.find(type);
        if (it == pasteRecords2UnifiedRecordMap_.end()) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "not find type, go to customData");
            unifiedData->AddRecords(Custom2UnifiedRecord(pasteboardRecord));
        } else {
            unifiedData->AddRecord(pasteRecords2UnifiedRecordMap_[type](pasteboardRecord));
        }
    }
    // 2.UnifiedProp2PaseteProp
    auto pastedProp = pasteData.GetProperty();
    auto unifiedProp = PaseteProp2UnifiedProp(pastedProp);
    unifiedData->SetProperties(unifiedProp);
    return unifiedData;
}

std::vector<std::shared_ptr<PasteDataRecord>> PasteboardUtils::UnifiedRecords2PasteRecords(
    std::vector<std::shared_ptr<UnifiedRecord>>& records)
{
    std::vector<std::shared_ptr<PasteDataRecord>> pasteboardRecords;
    for (auto& record : records) {
        auto type = record->GetType();
        // 拿到对应的UDType
        auto it = pasteRecords2UnifiedRecordMap_.find(type);
        if (it == pasteRecords2UnifiedRecordMap_.end()) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "not find type, convert to AppDefinedRecord");
            pasteboardRecords.push_back(AppDefined2PasteRecord(record));
        } else {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "find type, convert to unifiedRecord");
            pasteboardRecords.push_back(unifiedRecord2PasteRecordsMap_[type](record));
        }
    }
    return pasteboardRecords;
}

PasteDataProperty PasteboardUtils::UnifiedProp2PaseteProp(UnifiedDataProperties& properties)
{
    PasteDataProperty pasteDataProperty;
    pasteDataProperty.shareOption = static_cast<ShareOption>(properties.shareOption);
    pasteDataProperty.additions = properties.extras;
    pasteDataProperty.timestamp = properties.timestamp;
    pasteDataProperty.tag = properties.tag;
    return PasteDataProperty(pasteDataProperty);
}

std::shared_ptr<UnifiedDataProperties> PasteboardUtils::PaseteProp2UnifiedProp(PasteDataProperty& properties)
{
    std::shared_ptr<UnifiedDataProperties> unifiedDataProperties = std::make_shared<UnifiedDataProperties>();
    unifiedDataProperties->shareOption = properties.shareOption == ShareOption::InApp ? UDMF::ShareOption::IN_APP
                                                                                      : UDMF::ShareOption::CROSS_APP;
    unifiedDataProperties->extras = properties.additions;
    unifiedDataProperties->timestamp = properties.timestamp;
    unifiedDataProperties->tag = properties.tag;
    return unifiedDataProperties;
}

std::vector<std::string> PasteboardUtils::UtdTypes2PaseteTypes(std::vector<UDType>& uDTypes)
{
    std::vector<std::string> types;
    for (auto udType : uDTypes) {
        types.push_back(UtdType2PaseteType(udType));
    }
    return types;
}

std::string PasteboardUtils::UtdType2PaseteType(UDType uDType)
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
        default:
            return UDMF::UD_TYPE_MAP.at(uDType);
    }
}

UDType PasteboardUtils::PaseteType2UDType(int32_t uDType, const std::string& mimeType)
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
            return (UDType)type;
        }
    }
    return UDMF::UD_BUTT;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::PlainText2PasteRecord(std::shared_ptr<UnifiedRecord> record)
{
    auto plainText = static_cast<UDMF::PlainText*>(record.get());
    if (plainText == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get PLAIN_TEXT record field.");
        return nullptr;
    }
    auto plainTextRecord = PasteDataRecord::NewPlaintTextRecord(plainText->GetContent());
    plainTextRecord->SetDetails(std::move(plainText->GetDetails()));
    plainTextRecord->SetTextContent(std::move(plainText->GetAbstract()));
    plainTextRecord->SetUDType(UDMF::PLAIN_TEXT);
    return plainTextRecord;
}
std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2PlaintText(std::shared_ptr<PasteDataRecord> record)
{
    auto plainText = *(record->GetPlainText());
    auto plainTextRecord = std::make_shared<UDMF::PlainText>(plainText, record->GetTextContent());
    plainTextRecord->SetDetails(*record->GetDetails());
    return plainTextRecord;
}

// TODO:Want2PasteRecord
std::shared_ptr<PasteDataRecord> PasteboardUtils::Want2PasteRecord(std::shared_ptr<UnifiedRecord> record)
{
    return std::shared_ptr<PasteDataRecord>();
}
// TODO:WANT
std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Want(std::shared_ptr<PasteDataRecord> record)
{
    //    auto plainText = *(record->GetPlainText());
    //    auto link = std::make_shared<UDMF::Link>(plainText, record->GetTextContent());
    //    link->SetDetails(*record->GetDetails());
    return nullptr;
}
std::shared_ptr<PasteDataRecord> PasteboardUtils::Html2PasteRecord(std::shared_ptr<UnifiedRecord> record)
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
std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Html(std::shared_ptr<PasteDataRecord> record)
{
    auto html = std::make_shared<UDMF::Html>(*(record->GetHtmlText()), record->GetTextContent());
    html->SetDetails(*record->GetDetails());
    return html;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Link2PasteRecord(std::shared_ptr<UnifiedRecord> record)
{
    auto link = static_cast<UDMF::Link*>(record.get());
    if (link == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get HYPERLINK record field.");
        return nullptr;
    }
    auto plainTextRecord = PasteDataRecord::NewPlaintTextRecord(link->GetUrl());
    plainTextRecord->SetDetails(std::move(link->GetDetails()));
    plainTextRecord->SetTextContent(std::move(link->GetDescription()));
    plainTextRecord->SetUDType(UDMF::HYPERLINK);
    return plainTextRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Link(std::shared_ptr<PasteDataRecord> record)
{
    auto plainText = *(record->GetPlainText());
    auto link = std::make_shared<UDMF::Link>(plainText, record->GetTextContent());
    link->SetDetails(*record->GetDetails());
    return link;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::File2PasteRecord(std::shared_ptr<UnifiedRecord> record)
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
std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2File(std::shared_ptr<PasteDataRecord> record)
{
    auto file = std::make_shared<UDMF::File>(record->GetUri()->ToString());
    file->SetDetails(*record->GetDetails());
    return file;
}
std::shared_ptr<PasteDataRecord> PasteboardUtils::Image2PasteRecord(std::shared_ptr<UnifiedRecord> record)
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

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Image(std::shared_ptr<PasteDataRecord> record)
{
    auto image = std::make_shared<UDMF::Image>(record->GetUri()->ToString());
    image->SetDetails(*record->GetDetails());
    return image;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Video2PasteRecord(std::shared_ptr<UnifiedRecord> record)
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

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Video(std::shared_ptr<PasteDataRecord> record)
{
    auto video = std::make_shared<UDMF::Video>(record->GetUri()->ToString());
    video->SetDetails(*record->GetDetails());
    return video;
}
std::shared_ptr<PasteDataRecord> PasteboardUtils::Audio2PasteRecord(std::shared_ptr<UnifiedRecord> record)
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

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Audio(std::shared_ptr<PasteDataRecord> record)
{
    auto audio = std::make_shared<UDMF::Audio>(record->GetUri()->ToString());
    audio->SetDetails(*record->GetDetails());
    return audio;
}
std::shared_ptr<PasteDataRecord> PasteboardUtils::Folder2PasteRecord(std::shared_ptr<UnifiedRecord> record)
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

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Folder(std::shared_ptr<PasteDataRecord> record)
{
    auto folder = std::make_shared<UDMF::Folder>(record->GetUri()->ToString());
    folder->SetDetails(*record->GetDetails());
    return folder;
}
// TODO:PixelMap
std::shared_ptr<PasteDataRecord> PasteboardUtils::PixelMap2PasteRecord(std::shared_ptr<UnifiedRecord> record)
{
    return std::shared_ptr<PasteDataRecord>();
}
// TODO: pixelMap
std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2PixelMap(std::shared_ptr<PasteDataRecord> record)
{
    auto content = record->GetPixelMap();
    auto rawData = PasteDataRecord::PixelMap2Vector(content);
    return std::make_shared<UDMF::SystemDefinedPixelMap>(rawData);
}
std::shared_ptr<PasteDataRecord> PasteboardUtils::AppItem2PasteRecord(std::shared_ptr<UnifiedRecord> record)
{
    auto appItem = static_cast<UDMF::SystemDefinedAppItem*>(record.get());
    if (appItem == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get systemRecord record field.");
        return nullptr;
    }
    // TODO:customData要不要
    std::vector<uint8_t> arrayBuffer;
    auto kvRecord = PasteDataRecord::NewKvRecord(UtdType2PaseteType(UDType::SYSTEM_DEFINED_APP_ITEM), arrayBuffer);
    kvRecord->SetDetails(appItem->GetDetails());
    kvRecord->SetSystemDefinedContent(appItem->GetItems());
    kvRecord->SetUDType(UDType::SYSTEM_DEFINED_APP_ITEM);
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2AppItem(std::shared_ptr<PasteDataRecord> record)
{
    auto systemDefinedContent = record->GetSystemDefinedContent();
    auto systemDefinedDetails = record->GetDetails();
    auto appItem = std::make_shared<UDMF::SystemDefinedAppItem>();
    Details details;
    for (auto const& [key, value] : *systemDefinedDetails) {
        details[key] = value;
    }
    appItem->SetDetails(details);
    Details items;
    for (auto const& [key, value] : *systemDefinedContent) {
        items[key] = value;
    }
    appItem->SetItems(items);
    return appItem;
}
std::shared_ptr<PasteDataRecord> PasteboardUtils::Form2PasteRecord(std::shared_ptr<UnifiedRecord> record)
{
    auto form = static_cast<UDMF::SystemDefinedForm*>(record.get());
    if (form == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get systemRecord record field.");
        return nullptr;
    }
    // TODO:customData要不要
    std::vector<uint8_t> arrayBuffer;
    auto kvRecord = PasteDataRecord::NewKvRecord(UtdType2PaseteType(UDType::SYSTEM_DEFINED_FORM), arrayBuffer);
    kvRecord->SetDetails(form->GetDetails());
    kvRecord->SetSystemDefinedContent(form->GetItems());
    kvRecord->SetUDType(UDType::SYSTEM_DEFINED_FORM);
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Form(std::shared_ptr<PasteDataRecord> record)
{
    auto systemDefinedContent = record->GetSystemDefinedContent();
    auto systemDefinedDetails = record->GetDetails();
    auto form = std::make_shared<UDMF::SystemDefinedForm>();
    Details details;
    for (auto const& [key, value] : *systemDefinedDetails) {
        details[key] = value;
    }
    form->SetDetails(details);
    Details items;
    for (auto const& [key, value] : *systemDefinedContent) {
        items[key] = value;
    }
    form->SetItems(items);
    return form;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::SystemDefined2PasteRecord(std::shared_ptr<UnifiedRecord> record)
{
    auto systemDefined = static_cast<UDMF::SystemDefinedRecord*>(record.get());
    if (systemDefined == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get systemRecord record field.");
        return nullptr;
    }
    std::vector<uint8_t> arrayBuffer;
    auto kvRecord = PasteDataRecord::NewKvRecord(UtdType2PaseteType(UDType::SYSTEM_DEFINED_RECORD), arrayBuffer);
    kvRecord->SetDetails(systemDefined->GetDetails());
    kvRecord->SetUDType(UDType::SYSTEM_DEFINED_RECORD);
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2SystemDefined(std::shared_ptr<PasteDataRecord> record)
{
    auto systemDefinedDetails = record->GetDetails();
    auto systemRecord = std::make_shared<UDMF::SystemDefinedRecord>();
    Details details;
    for (auto const& [key, value] : *systemDefinedDetails) {
        details[key] = value;
    }
    systemRecord->SetDetails(details);
    return systemRecord;
}

// Convert to CustomRecord
std::shared_ptr<PasteDataRecord> PasteboardUtils::Text2PasteRecord(std::shared_ptr<UnifiedRecord> record)
{
    auto text = static_cast<UDMF::Text*>(record.get());
    if (text == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get TEXT record field.");
        return nullptr;
    }
    // TODO:转成u8
    std::vector<uint8_t> arrayBuffer;
    std::string type = UDMF::UD_TYPE_MAP.at(UDMF::TEXT);
    auto kvRecord = PasteDataRecord::NewKvRecord(type, arrayBuffer);
    kvRecord->SetUDType(UDMF::TEXT);
    kvRecord->SetDetails(text->GetDetails());
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Text(std::shared_ptr<PasteDataRecord> record)
{
    auto text = std::make_shared<UDMF::Text>();
    text->SetDetails(*record->GetDetails());
    return text;
}

std::vector<std::shared_ptr<UnifiedRecord>> PasteboardUtils::Custom2UnifiedRecord(
    std::shared_ptr<PasteDataRecord> record)
{
    std::vector<std::shared_ptr<UnifiedRecord>> unifiedRecords;
    auto customData = record->GetCustomData();
    for (auto& [type, rawData] : customData->GetItemData()) {
        unifiedRecords.push_back(std::make_shared<UDMF::ApplicationDefinedRecord>(type, rawData));
    }
    return unifiedRecords;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::AppDefined2PasteRecord(std::shared_ptr<UnifiedRecord> record)
{
    auto appRecord = static_cast<UDMF::ApplicationDefinedRecord*>(record.get());
    if (appRecord == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "get ApplicationDefinedRecord record field.");
        return nullptr;
    }
    auto kvRecord = PasteDataRecord::NewKvRecord(appRecord->GetApplicationDefinedType(), appRecord->GetRawData());
    kvRecord->SetUDType(appRecord->GetType());
    return kvRecord;
}

PasteboardUtils::PasteboardUtils()
{
    InitDecodeMap();
}
PasteboardUtils* PasteboardUtils::GetInstance()
{
    static PasteboardUtils* loader = new PasteboardUtils();
    return loader;
}
} // namespace MiscServices
} // namespace OHOS