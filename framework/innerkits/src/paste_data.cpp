
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

#include "paste_data.h"
#include <new>
#include "paste_data_record.h"
#include "pasteboard_hilog_wreapper.h"
#include "type_traits"

using namespace std::chrono;
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
namespace {
const std::uint32_t MAX_RECORD_NUM = 512;
}

enum TAG_PASTEBOARD : uint16_t {
    TAG_PROPS = TAG_BUFF + 1,
    TAG_RECORDS,
    TAG_RECORDS_COUNT,
    TAG_RECORDS_ITEM,
};
enum TAG_PROPERTY : uint16_t {
    TAG_ADDITIONS = TAG_BUFF + 1,
    TAG_MIMETYPES,
    TAG_TAG,
    TAG_TIMESTAMP,
    TAG_SHAREOPTION,
    TAG_APPID
};

PasteData::PasteData(std::vector<std::shared_ptr<PasteDataRecord>> records)
    : records_ {std::move(records)}
{
    props_.timestamp = steady_clock::now().time_since_epoch().count();
    props_.localOnly = false;
    props_.shareOption = ShareOption::CrossDevice;
}

PasteData::PasteData()
{
    props_.shareOption = ShareOption::CrossDevice;
}

PasteDataProperty PasteData::GetProperty()
{
    return props_;
}

void PasteData::AddHtmlRecord(const std::string &html)
{
    this->AddRecord(PasteDataRecord::NewHtmlRecord(html));
}

void PasteData::AddPixelMapRecord(std::shared_ptr<PixelMap> pixelMap)
{
    this->AddRecord(PasteDataRecord::NewPixelMapRecord(std::move(pixelMap)));
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

void PasteData::AddKvRecord(const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer)
{
    AddRecord(PasteDataRecord::NewKvRecord(mimeType, arrayBuffer));
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

std::shared_ptr<PixelMap> PasteData::GetPrimaryPixelMap()
{
    for (const auto &item : records_) {
        if (item->GetMimeType() == MIMETYPE_PIXELMAP) {
            return item->GetPixelMap();
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
        if ((item->GetPlainText() != nullptr) && (item->GetPlainText()->size() > 0)) {
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

ShareOption PasteData::GetShareOption()
{
    return props_.shareOption;
}

void PasteData::SetShareOption(ShareOption shareOption)
{
    props_.shareOption = shareOption;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "shareOption = %{public}d.", shareOption);
}

std::string PasteData::GetAppId()
{
    return props_.appId;
}

void PasteData::SetAppId(const std::string &appId)
{
    props_.appId = appId;
}

bool PasteData::RemoveRecordAt(std::size_t number)
{
    if (records_.size() > number) {
        records_.erase(records_.begin() + static_cast<std::int64_t>(number));
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
    for (const auto& record : records_) {
        if (record == nullptr) {
            continue;
        }
        mimeTypes.insert(mimeTypes.end(), record->GetMimeType());
    }
    props_.mimeTypes = mimeTypes;
}

bool PasteData::MarshallingProps(Parcel &parcel) const
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "props.shareOption =  %{public}d.", props_.shareOption);
    return parcel.WriteParcelable(&props_.additions) && parcel.WriteStringVector(props_.mimeTypes)
           && parcel.WriteString(props_.tag) && parcel.WriteInt64(props_.timestamp)
           && parcel.WriteInt32(static_cast<int32_t>(props_.shareOption));
}

bool PasteData::UnMarshalling(Parcel &parcel, PasteDataProperty &props)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");

    std::shared_ptr<AAFwk::WantParams> wantParams(parcel.ReadParcelable<AAFwk::WantParams>());
    if (wantParams != nullptr) {
        props.additions = *wantParams;
    }
    if (!parcel.ReadStringVector(&props.mimeTypes)) {
        return false;
    }
    props.tag = parcel.ReadString();
    props.timestamp = parcel.ReadInt64();
    props.shareOption = static_cast<ShareOption>(parcel.ReadInt32());
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "props.shareOption =  %{public}d.", props.shareOption);
    return true;
}

bool PasteData::Marshalling(Parcel &parcel) const
{
    auto length = records_.size();
    if (length == 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "length == 0.");
        return false;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start, length = %{public}zu.", length);
    if (!parcel.WriteUint32(static_cast<uint32_t>(length))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write length failed.");
        return false;
    }

    for (const auto &item : records_) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "for.");
        if (!parcel.WriteParcelable(item.get())) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteParcelable failed.");
            return false;
        }
    }

    auto ret = MarshallingProps(parcel);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "MarshallingProps failed.");
        return false;
    }

    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "end.");
    return true;
}

PasteData *PasteData::Unmarshalling(Parcel &parcel)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start.");
    auto *pasteData = new (std::nothrow) PasteData();
    if (pasteData == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "pasteData is nullptr.");
        return pasteData;
    }

    auto length = parcel.ReadUint32();
    if (length <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "length error.");
        delete pasteData;
        pasteData = nullptr;
        return pasteData;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "length = %{public}u.", length);

    uint32_t failedNum = 0;
    for (uint32_t i = 0; i < length; i++) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "for.");
        std::unique_ptr<PasteDataRecord> record(parcel.ReadParcelable<PasteDataRecord>());
        if (!record) {
            failedNum++;
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "ReadParcelable failed, i = %{public}d.", i);
            continue;
        }
        pasteData->AddRecord(*record);
    }
    if (failedNum == length) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "all failed.");
        delete pasteData;
        pasteData = nullptr;
        return pasteData;
    }

    auto ret = UnMarshalling(parcel, pasteData->props_);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "UnMarshallingProps failed.");
        delete pasteData;
        pasteData = nullptr;
    }

    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "end.");
    return pasteData;
}

bool PasteData::Encode(std::vector<std::uint8_t> &buffer)
{
    bool ret = Write(buffer, TAG_PROPS, (TLVObject &)props_);
    //    ret = Write(buffer, TAG_RECORDS, records_) && ret;
    ret = Write(buffer, TAG_RECORDS_COUNT, (int32_t)records_.size()) && ret;
    for (auto &record : records_) {
        ret = Write(buffer, TAG_RECORDS_ITEM, (TLVObject &)(*record)) && ret;
    }
    return ret;
}

bool PasteData::Decode(const std::vector<std::uint8_t> &buffer)
{
    total_ = buffer.size();
    for (; IsEnough();) {
        TLVHead head{};
        bool ret = ReadHead(buffer, head);
        switch (head.tag) {
            case TAG_PROPS:
                ret = ret && ReadValue(buffer, (TLVObject &)props_, head);
                break;
            case TAG_RECORDS_COUNT: {
                int32_t count = 0;
                ret = ret && ReadValue(buffer, count, head);
                records_.resize(count);
                break;
            }
            case TAG_RECORDS_ITEM: {
                auto item = std::make_shared<PasteDataRecord>();
                ret = ret && ReadValue(buffer, (TLVObject &)(*item), head);
                for (auto &record : records_) {
                    if (record == nullptr) {
                        record = item;
                        break;
                    }
                }
                break;
            }
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
size_t PasteData::Count()
{
    size_t expectSize = 0;
    expectSize += props_.Count() + sizeof(TLVHead);
    expectSize += sizeof(TLVHead);
    expectSize += sizeof(TLVHead) + sizeof(int32_t);
    for (auto & record : records_) {
        expectSize += record->Count();
    }
    return expectSize;
}
bool PasteDataProperty::Encode(std::vector<std::uint8_t> &buffer)
{
    bool ret = Write(buffer, TAG_ADDITIONS, additions);
    ret = Write(buffer, TAG_MIMETYPES, mimeTypes) && ret;
    ret = Write(buffer, TAG_TAG, tag) && ret;
    ret = Write(buffer, TAG_TIMESTAMP, timestamp) && ret;
    ret = Write(buffer, TAG_SHAREOPTION, (int32_t &)shareOption) && ret;
    ret = Write(buffer, TAG_APPID, appId) && ret;
    return ret;
}
bool PasteDataProperty::Decode(const std::vector<std::uint8_t> &buffer)
{
    for (; IsEnough();) {
        TLVHead head{};
        bool ret = ReadHead(buffer, head);
        switch (head.tag) {
            case TAG_ADDITIONS: {
                ret = ret && ReadValue(buffer, additions, head);
                break;
            }
            case TAG_MIMETYPES:
                ret = ret && ReadValue(buffer, mimeTypes, head);
                break;
            case TAG_TAG:
                ret = ret && ReadValue(buffer, tag, head);
                break;
            case TAG_TIMESTAMP:
                ret = ret && ReadValue(buffer, timestamp, head);
                break;
            case TAG_SHAREOPTION:
                ret = ret && ReadValue(buffer, (int32_t &)shareOption, head);
                break;
            case TAG_APPID:
                ret = ret && ReadValue(buffer, appId, head);
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
size_t PasteDataProperty::Count()
{
    size_t expectedSize = 0;
    expectedSize += TLVObject::Count(additions);
    expectedSize += sizeof(TLVHead);
    expectedSize += sizeof(int32_t) + sizeof(TLVHead);
    for (auto &item : mimeTypes) {
        expectedSize += TLVObject::Count(item);
    }
    expectedSize += TLVObject::Count(tag);
    expectedSize += TLVObject::Count(timestamp);
    expectedSize += TLVObject::Count(shareOption);
    expectedSize += TLVObject::Count(appId);
    return expectedSize;
}
} // MiscServices
} // OHOS