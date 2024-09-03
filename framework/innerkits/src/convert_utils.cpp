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
#include "convert_utils.h"

#include "pasteboard_hilog.h"
#include "unified_meta.h"
namespace OHOS {
namespace MiscServices {
using UnifiedRecord = UDMF::UnifiedRecord;
using UnifiedData = UDMF::UnifiedData;
using UnifiedDataProperties = UDMF::UnifiedDataProperties;
using UDType = UDMF::UDType;

std::shared_ptr<PasteData> ConvertUtils::Convert(const UnifiedData& unifiedData)
{
    auto pasteData = std::make_shared<PasteData>(Convert(unifiedData.GetRecords()));
    pasteData->SetProperty(ConvertProperty(unifiedData.GetProperties(), unifiedData));
    return pasteData;
}

std::shared_ptr<UnifiedData> ConvertUtils::Convert(const PasteData& pasteData)
{
    auto unifiedData = std::make_shared<UnifiedData>();
    unifiedData->SetRecords(Convert(pasteData.AllRecords()));
    unifiedData->SetProperties(ConvertProperty(pasteData.GetProperty()));
    unifiedData->SetDataId(pasteData.GetDataId());
    return unifiedData;
}

std::vector<std::shared_ptr<UnifiedRecord>> ConvertUtils::Convert(
    const std::vector<std::shared_ptr<PasteDataRecord>>& records)
{
    std::vector<std::shared_ptr<UnifiedRecord>> unifiedRecords;
    for (auto const& record : records) {
        unifiedRecords.emplace_back(Convert(record));
    }
    return unifiedRecords;
}

std::vector<std::shared_ptr<PasteDataRecord>> ConvertUtils::Convert(
    const std::vector<std::shared_ptr<UnifiedRecord>>& records)
{
    std::vector<std::shared_ptr<PasteDataRecord>> pasteboardRecords;
    for (auto const& record : records) {
        pasteboardRecords.emplace_back(Convert(record));
    }
    return pasteboardRecords;
}

std::shared_ptr<UnifiedRecord> ConvertUtils::Convert(std::shared_ptr<PasteDataRecord> record)
{
    if (record == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "paste record is nullptr");
        return nullptr;
    }
    std::shared_ptr<UnifiedRecord> udmfRecord = std::make_shared<UnifiedRecord>();
    auto entries = Convert(record->GetEntries());
    if (entries->empty()) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "entries is nullptr");
        auto udmfValue = record->GetUDMFValue();
        if (udmfValue) {
            udmfRecord->AddEntry(CommonUtils::Convert2UtdId(record->GetUDType(), record->GetMimeType()),
                std::move(*udmfValue));
        }
        return udmfRecord;
    }
    for (auto &[utdId, value] : *entries) {
        udmfRecord->AddEntry(utdId, std::move(value));
    }
    udmfRecord->SetChannelName(CHANNEL_NAME);
    udmfRecord->SetDataId(record->GetDataId());
    udmfRecord->SetRecordId(record->GetRecordId());
    return udmfRecord;
}

std::shared_ptr<PasteDataRecord> ConvertUtils::Convert(std::shared_ptr<UnifiedRecord> record)
{
    if (record == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "udmfRecord is nullptr");
        return nullptr;
    }
    std::shared_ptr<PasteDataRecord> pbRecord = std::make_shared<PasteDataRecord>();
    auto utdId = record->GetUtdId();
    pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, record->GetValue()));
    for (auto const& entry : Convert(record->GetEntries())) {
        if (entry == nullptr) {
            PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "entry is empty");
            continue;
        }
        if (utdId == entry->GetUtdId()) {
            continue;
        }
        pbRecord->AddEntry(entry->GetUtdId(), entry);
    }
    pbRecord->SetDataId(record->GetDataId());
    pbRecord->SetRecordId(record->GetRecordId());
    if (record->GetEntryGetter() != nullptr) {
        pbRecord->SetDelayRecordFlag(true);
    }
    return pbRecord;
}

std::vector<std::shared_ptr<PasteDataEntry>> ConvertUtils::Convert(
    const std::shared_ptr<std::map<std::string, UDMF::ValueType>>& entries)
{
    std::vector<std::shared_ptr<PasteDataEntry>> pbEntries;
    if (entries == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "pbEntries is empty");
        return pbEntries;
    }
    for (auto const& [utdId, value] : *entries) {
        pbEntries.emplace_back(std::make_shared<PasteDataEntry>(utdId, value));
    }
    return pbEntries;
}

std::shared_ptr<std::map<std::string, UDMF::ValueType>> ConvertUtils::Convert(
    const std::vector<std::shared_ptr<PasteDataEntry>>& entries)
{
    std::map<std::string, UDMF::ValueType> udmfEntries;
    for (auto const& entry : entries) {
        if (entry == nullptr) {
            continue;
        }
        udmfEntries.emplace(entry->GetUtdId(), entry->GetValue());
    }
    return std::make_shared<std::map<std::string, UDMF::ValueType>>(udmfEntries);
}

PasteDataProperty ConvertUtils::ConvertProperty(const std::shared_ptr<UnifiedDataProperties>& properties,
    const UnifiedData& unifiedData)
{
    if (!properties) {
        return {};
    }
    PasteDataProperty pasteDataProperty;
    pasteDataProperty.shareOption = static_cast<ShareOption>(properties->shareOptions);
    pasteDataProperty.additions = properties->extras;
    pasteDataProperty.timestamp = properties->timestamp;
    pasteDataProperty.tag = properties->tag;
    auto utdIds = unifiedData.GetTypesLabels();
    pasteDataProperty.mimeTypes = Convert(utdIds);
    pasteDataProperty.isRemote = properties->isRemote;
    return PasteDataProperty(pasteDataProperty);
}

std::shared_ptr<UnifiedDataProperties> ConvertUtils::ConvertProperty(const PasteDataProperty& properties)
{
    auto unifiedDataProperties = std::make_shared<UnifiedDataProperties>();
    unifiedDataProperties->shareOptions = properties.shareOption == InApp ? UDMF::ShareOptions::IN_APP
                                                                          : UDMF::ShareOptions::CROSS_APP;
    unifiedDataProperties->extras = properties.additions;
    unifiedDataProperties->timestamp = properties.timestamp;
    unifiedDataProperties->tag = properties.tag;
    unifiedDataProperties->isRemote = properties.isRemote;
    return unifiedDataProperties;
}

std::vector<std::string> ConvertUtils::Convert(const std::vector<std::string>& utdIds)
{
    std::vector<std::string> types;
    for (const auto& utdId : utdIds) {
        types.push_back(CommonUtils::Convert2MimeType(utdId));
    }
    return types;
}
} // namespace MiscServices
} // namespace OHOS