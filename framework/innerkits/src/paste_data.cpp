
/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <chrono>
#include "paste_data_record.h"
#include "pasteboard_common.h"
#include "paste_data.h"

using namespace std::chrono;

namespace OHOS {
namespace MiscServices {
namespace {
const std::uint32_t MAX_RECORD_NUM = 128;
}

PasteData::PasteData(std::vector<std::shared_ptr<PasteDataRecord>> records)
    : records_ {std::move(records)}
{
    props_.timestamp = steady_clock::now().time_since_epoch().count();
    localOnly = false;
}

PasteDataProperty PasteData::GetProperty()
{
    return PasteDataProperty {};
}

void PasteData::AddHtmlRecord(const std::string &html)
{
    this->AddRecord(PasteDataRecord::NewHtmlRecord(html));
}

void PasteData::AddWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    this->AddRecord(PasteDataRecord::NewWantRecord(std::move(want)));
}

void PasteData::AddTextRecord(const std::string &text)
{
    this->AddRecord(PasteDataRecord::NewPlaintTextRecord(text));
}

void PasteData::AddUriRecord(const OHOS::Uri &uri)
{
    this->AddRecord(PasteDataRecord::NewUriRecord(uri));
}

void PasteData::AddRecord(std::shared_ptr<PasteDataRecord> record)
{
    if (record == nullptr) {
        return;
    }
    records_.insert(records_.begin(), std::move(record));
    if (records_.size() > MAX_RECORD_NUM) {
        std::vector<std::shared_ptr<PasteDataRecord>> new_records(records_.begin(), records_.end()-1);
        this->records_ = new_records;
    }
    RefreshMimeProp();
}

void PasteData::AddRecord(PasteDataRecord& record)
{
    this->AddRecord(std::make_shared<PasteDataRecord>(record));
    RefreshMimeProp();
}

std::vector<std::string> PasteData::GetMimeTypes()
{
    std::vector<std::string> mimeType;
    for (const auto &item: records_) {
        mimeType.push_back(item->GetMimeType());
    }
    return mimeType;
}

std::shared_ptr<std::string> PasteData::GetPrimaryHtml()
{
    for (const auto &item: records_) {
        if (item->GetMimeType() == MIMETYPE_TEXT_HTML) {
            return item->GetHtmlText();
        }
    }
    return nullptr;
}

std::shared_ptr<OHOS::AAFwk::Want> PasteData::GetPrimaryWant()
{
    for (const auto &item: records_) {
        if (item->GetMimeType() == MIMETYPE_TEXT_WANT) {
            return item->GetWant();
        }
    }
    return nullptr;
}

std::shared_ptr<std::string> PasteData::GetPrimaryText()
{
    for (const auto &item: records_) {
        if (item->GetMimeType() == MIMETYPE_TEXT_PLAIN) {
            return item->GetPlainText();
        }
    }
    return nullptr;
}

std::shared_ptr<OHOS::Uri> PasteData::GetPrimaryUri()
{
    for (const auto &item : records_) {
        if (item->GetMimeType() == MIMETYPE_TEXT_URI) {
            return item->GetUri();
        }
    }
    return nullptr;
}

std::shared_ptr<std::string> PasteData::GetPrimaryMimeType()
{
    if (!records_.empty()) {
        return std::make_shared<std::string>(records_.front()->GetMimeType());
    } else {
        return nullptr;
    }
}

std::shared_ptr<PasteDataRecord> PasteData::GetRecordAt(std::size_t index)
{
    if (records_.size() > index) {
        return records_[index];
    } else {
        return nullptr;
    }
}

std::size_t PasteData::GetRecordCount()
{
    return records_.size();
}

bool PasteData::RemoveRecordAt(std::size_t number)
{
    if (records_.size() > number) {
        records_.emplace(records_.begin() + static_cast<std::int64_t>(number));
        RefreshMimeProp();
        return true;
    } else {
        return false;
    }
}

bool PasteData::ReplaceRecordAt(std::size_t number, std::shared_ptr<PasteDataRecord> record)
{
    if (records_.size() > number) {
        records_[number] = std::move(record);
        RefreshMimeProp();
        return true;
    } else {
        return false;
    }
}

bool PasteData::HasMimeType(const std::string &mimeType)
{
    for (auto &item : records_) {
        if (item->GetMimeType() == mimeType) {
            return true;
        }
    }
    return false;
}

std::string PasteData::GetTag()
{
    return props_.tag;
}

std::vector<std::shared_ptr<PasteDataRecord>> PasteData::AllRecords() const
{
    return this->records_;
}

void PasteData::RefreshMimeProp()
{
    std::vector<std::string> mimeTypes;
    for (const auto record : records_) {
        if (record == nullptr) {
            continue;
        }
        mimeTypes.insert(mimeTypes.end(), record->GetMimeType());
    }
    props_.mimeTypes = mimeTypes;
}

bool PasteData::Marshalling(Parcel &parcel) const
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    auto length = records_.size();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "length: %{public}zu.", length);
    // write length
    if (!parcel.WriteUint32(static_cast<uint32_t>(length))) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
        return false;
    }
    for (const auto item : records_) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "for.");
        if (!parcel.WriteParcelable(item.get())) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "write failed end.");
            return false;
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return true;
}

bool PasteData::ReadFromParcel(Parcel &parcel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    records_.clear();
    // read vector length
    auto length = parcel.ReadUint32();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "length: %{public}u.", length);
    for (uint32_t i = 0; i < length; i++) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "for.");
        std::unique_ptr<PasteDataRecord> record(parcel.ReadParcelable<PasteDataRecord>());
        if (!record) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "nullptr.");
            continue;
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "add.");
        AddRecord(*record);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return true;
}

PasteData *PasteData::Unmarshalling(Parcel &parcel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    PasteData *pasteData = new PasteData();

    if (pasteData && !pasteData->ReadFromParcel(parcel)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "delete end.");
        delete pasteData;
        pasteData = nullptr;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return pasteData;
}
} // MiscServices
} // OHOS