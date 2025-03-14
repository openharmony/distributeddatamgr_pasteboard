
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

#include "int_wrapper.h"
#include "long_wrapper.h"
#include "pasteboard_common.h"
#include "pasteboard_hilog.h"

using namespace std::chrono;
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
enum TAG_PASTEBOARD : uint16_t {
    TAG_PROPS = TAG_BUFF + 1,
    TAG_RECORDS,
    TAG_DRAGGED_DATA_FLAG,
    TAG_LOCAL_PASTE_FLAG,
    TAG_DELAY_DATA_FLAG,
    TAG_DEVICE_ID,
    TAG_PASTE_ID,
    TAG_DELAY_RECORD_FLAG,
    TAG_DATA_ID,
    TAG_RECORD_ID,
};
enum TAG_PROPERTY : uint16_t {
    TAG_ADDITIONS = TAG_BUFF + 1,
    TAG_MIMETYPES,
    TAG_TAG,
    TAG_LOCAL_ONLY,
    TAG_TIMESTAMP,
    TAG_SHAREOPTION,
    TAG_TOKENID,
    TAG_ISREMOTE,
    TAG_BUNDLENAME,
    TAG_SETTIME,
    TAG_SCREENSTATUS,
};

std::string PasteData::sharePath = "";
std::string PasteData::WEBVIEW_PASTEDATA_TAG = "WebviewPasteDataTag";
const std::string PasteData::DISTRIBUTEDFILES_TAG = "distributedfiles";
const std::string PasteData::PATH_SHARE = "/data/storage/el2/share/r/";
const std::string PasteData::IMG_LOCAL_URI = "file:///";
const std::string PasteData::SHARE_PATH_PREFIX = "/mnt/hmdfs/";
const std::string PasteData::SHARE_PATH_PREFIX_ACCOUNT = "/account/merge_view/services/";
const std::string PasteData::DOCS_LOCAL_TAG = "/docs/";
const char *REMOTE_FILE_SIZE = "remoteFileSize";
const char *REMOTE_FILE_SIZE_LONG = "remoteFileSizeLong";

PasteData::PasteData()
{
    props_.timestamp = steady_clock::now().time_since_epoch().count();
    props_.localOnly = false;
    props_.shareOption = ShareOption::CrossDevice;
    InitDecodeMap();
}

PasteData::~PasteData()
{
    records_.clear();
    decodeMap_.clear();
}

PasteData::PasteData(const PasteData &data)
    : originAuthority_(data.originAuthority_), valid_(data.valid_), isDraggedData_(data.isDraggedData_),
      isLocalPaste_(data.isLocalPaste_), isDelayData_(data.isDelayData_), pasteId_(data.pasteId_),
      isDelayRecord_(data.isDelayRecord_), dataId_(data.dataId_), recordId_(data.recordId_)
{
    this->props_ = data.props_;
    for (const auto &item : data.records_) {
        this->records_.emplace_back(std::make_shared<PasteDataRecord>(*item));
    }
    InitDecodeMap();
}

PasteData::PasteData(std::vector<std::shared_ptr<PasteDataRecord>> records) : records_{ std::move(records) }
{
    for (const auto &item : records_) {
        item->SetRecordId(++recordId_);
    }
    props_.timestamp = steady_clock::now().time_since_epoch().count();
    props_.localOnly = false;
    props_.shareOption = ShareOption::CrossDevice;
    InitDecodeMap();
}

PasteData &PasteData::operator=(const PasteData &data)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "PasteData copy");
    if (this == &data) {
        return *this;
    }
    this->originAuthority_ = data.originAuthority_;
    this->valid_ = data.valid_;
    this->isDraggedData_ = data.isDraggedData_;
    this->isLocalPaste_ = data.isLocalPaste_;
    this->isDelayData_ = data.isDelayData_;
    this->isDelayRecord_ = data.isDelayRecord_;
    this->dataId_ = data.dataId_;
    this->props_ = data.props_;
    this->records_.clear();
    this->deviceId_ = data.deviceId_;
    this->pasteId_ = data.pasteId_;
    for (const auto &item : data.records_) {
        this->records_.emplace_back(std::make_shared<PasteDataRecord>(*item));
    }
    this->recordId_ = data.GetRecordId();
    return *this;
}

PasteDataProperty PasteData::GetProperty() const
{
    return PasteDataProperty(props_);
}

void PasteData::SetProperty(const PasteDataProperty &property)
{
    this->props_ = property;
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
    this->AddRecord(PasteDataRecord::NewPlainTextRecord(text));
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
    PASTEBOARD_CHECK_AND_RETURN_LOGE(record != nullptr, PASTEBOARD_MODULE_CLIENT, "record is null");
    record->SetRecordId(++recordId_);

    if (PasteBoardCommon::IsPasteboardService()) {
        props_.mimeTypes.emplace_back(record->GetMimeType());
        records_.emplace_back(std::move(record));
    } else {
        props_.mimeTypes.insert(props_.mimeTypes.begin(), record->GetMimeType());
        records_.insert(records_.begin(), std::move(record));
    }
}

void PasteData::AddRecord(const PasteDataRecord &record)
{
    this->AddRecord(std::make_shared<PasteDataRecord>(record));
}

std::vector<std::string> PasteData::GetMimeTypes()
{
    std::set<std::string> mimeTypes;
    for (const auto &item : records_) {
        if (item->GetFrom() > 0 && item->GetRecordId() != item->GetFrom()) {
            continue;
        }
        auto itemTypes = item->GetMimeTypes();
        mimeTypes.insert(itemTypes.begin(), itemTypes.end());
    }
    return std::vector<std::string>(mimeTypes.begin(), mimeTypes.end());
}

std::shared_ptr<std::string> PasteData::GetPrimaryHtml()
{
    for (const auto &item : records_) {
        std::shared_ptr<std::string> primary = item->GetHtmlText();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
}

std::shared_ptr<PixelMap> PasteData::GetPrimaryPixelMap()
{
    for (const auto &item : records_) {
        std::shared_ptr<PixelMap> primary = item->GetPixelMap();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
}

std::shared_ptr<OHOS::AAFwk::Want> PasteData::GetPrimaryWant()
{
    for (const auto &item : records_) {
        std::shared_ptr<OHOS::AAFwk::Want> primary = item->GetWant();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
}

std::shared_ptr<std::string> PasteData::GetPrimaryText()
{
    for (const auto &item : records_) {
        std::shared_ptr<std::string> primary = item->GetPlainText();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
}

std::shared_ptr<OHOS::Uri> PasteData::GetPrimaryUri()
{
    for (const auto &item : records_) {
        std::shared_ptr<OHOS::Uri> primary = item->GetUri();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
}

std::shared_ptr<std::string> PasteData::GetPrimaryMimeType()
{
    if (records_.empty()) {
        return nullptr;
    }
    return std::make_shared<std::string>(records_.front()->GetMimeType());
}

std::shared_ptr<PasteDataRecord> PasteData::GetRecordAt(std::size_t index) const
{
    if (records_.size() > index) {
        return records_[index];
    } else {
        return nullptr;
    }
}

std::shared_ptr<PasteDataRecord> PasteData::GetRecordById(uint32_t recordId) const
{
    for (const auto &record : records_) {
        if (record != nullptr && record->GetRecordId() == recordId) {
            return record;
        }
    }
    return nullptr;
}

std::size_t PasteData::GetRecordCount() const
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

std::uint32_t PasteData::GetTokenId()
{
    return props_.tokenId;
}

void PasteData::SetTokenId(uint32_t tokenId)
{
    props_.tokenId = tokenId;
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
    if (record == nullptr) {
        return false;
    }
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
    for (const auto &item : records_) {
        auto itemTypes = item->GetMimeTypes();
        if (itemTypes.find(mimeType) != itemTypes.end()) {
            return true;
        }
    }
    return false;
}

std::vector<std::shared_ptr<PasteDataRecord>> PasteData::AllRecords() const
{
    return this->records_;
}

bool PasteData::IsDraggedData() const
{
    return isDraggedData_;
}

bool PasteData::IsLocalPaste() const
{
    return isLocalPaste_;
}

void PasteData::SetDraggedDataFlag(bool isDraggedData)
{
    isDraggedData_ = isDraggedData;
}

void PasteData::SetLocalPasteFlag(bool isLocalPaste)
{
    isLocalPaste_ = isLocalPaste;
}

void PasteData::SetRemote(bool isRemote)
{
    props_.isRemote = isRemote;
}

bool PasteData::IsRemote()
{
    return props_.isRemote;
}

void PasteData::SetBundleName(const std::string &bundleName)
{
    props_.bundleName = bundleName;
}

std::string PasteData::GetBundleName() const
{
    return props_.bundleName;
}

void PasteData::SetOriginAuthority(const std::string &bundleName)
{
    originAuthority_ = bundleName;
}

std::string PasteData::GetOriginAuthority() const
{
    return originAuthority_;
}

void PasteData::SetTime(const std::string &setTime)
{
    props_.setTime = setTime;
}

std::string PasteData::GetTime()
{
    return props_.setTime;
}

void PasteData::SetScreenStatus(ScreenEvent screenStatus)
{
    props_.screenStatus = screenStatus;
}

ScreenEvent PasteData::GetScreenStatus()
{
    return props_.screenStatus;
}

void PasteData::SetTag(const std::string &tag)
{
    props_.tag = tag;
}

std::string PasteData::GetTag()
{
    return props_.tag;
}

void PasteData::SetAdditions(const AAFwk::WantParams &additions)
{
    props_.additions = additions;
}

void PasteData::SetAddition(const std::string &key, AAFwk::IInterface *value)
{
    PASTEBOARD_CHECK_AND_RETURN_LOGE(value != nullptr, PASTEBOARD_MODULE_CLIENT, "value is null");
    props_.additions.SetParam(key, value);
}

void PasteData::SetFileSize(int64_t fileSize)
{
    int32_t fileIntSize = (fileSize > INT_MAX) ? INT_MAX : static_cast<int32_t>(fileSize);
    SetAddition(REMOTE_FILE_SIZE, AAFwk::Integer::Box(fileIntSize));
    SetAddition(REMOTE_FILE_SIZE_LONG, AAFwk::Long::Box(fileSize));
}

int64_t PasteData::GetFileSize() const
{
    int64_t fileSize = 0L;
    auto value = props_.additions.GetParam(REMOTE_FILE_SIZE_LONG);
    AAFwk::ILong *ao = AAFwk::ILong::Query(value);
    if (ao != nullptr) {
        fileSize = AAFwk::Long::Unbox(ao);
    } else {
        fileSize = props_.additions.GetIntParam(REMOTE_FILE_SIZE, -1);
    }
    return fileSize;
}

void PasteData::SetLocalOnly(bool localOnly)
{
    props_.localOnly = localOnly;
}

bool PasteData::GetLocalOnly()
{
    return props_.localOnly;
}

void PasteData::RefreshMimeProp()
{
    std::vector<std::string> mimeTypes;
    for (const auto &record : records_) {
        if (record == nullptr) {
            continue;
        }
        mimeTypes.emplace_back(record->GetMimeType());
    }
    props_.mimeTypes = mimeTypes;
}

bool PasteData::EncodeTLV(WriteOnlyBuffer &buffer) const
{
    bool ret = buffer.Write(TAG_PROPS, props_);
    ret = ret && buffer.Write(TAG_RECORDS, records_);
    ret = ret && buffer.Write(TAG_DRAGGED_DATA_FLAG, isDraggedData_);
    ret = ret && buffer.Write(TAG_LOCAL_PASTE_FLAG, isLocalPaste_);
    ret = ret && buffer.Write(TAG_DELAY_DATA_FLAG, isDelayData_);
    ret = ret && buffer.Write(TAG_DEVICE_ID, deviceId_);
    ret = ret && buffer.Write(TAG_PASTE_ID, pasteId_);
    ret = ret && buffer.Write(TAG_DELAY_RECORD_FLAG, isDelayRecord_);
    ret = ret && buffer.Write(TAG_DATA_ID, dataId_);
    ret = ret && buffer.Write(TAG_RECORD_ID, recordId_);
    return ret;
}

void PasteData::InitDecodeMap()
{
    decodeMap_ = {
        {TAG_PROPS,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(props_, head); }},
        {TAG_RECORDS,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(records_, head); }},
        {TAG_DRAGGED_DATA_FLAG,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(isDraggedData_, head); }},
        {TAG_LOCAL_PASTE_FLAG,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(isLocalPaste_, head); }},
        {TAG_DELAY_DATA_FLAG,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(isDelayData_, head); }},
        {TAG_DEVICE_ID,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(deviceId_, head); }},
        {TAG_PASTE_ID,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(pasteId_, head); }},
        {TAG_DELAY_RECORD_FLAG,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(isDelayRecord_, head); }},
        {TAG_DATA_ID,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(dataId_, head); }},
        {TAG_RECORD_ID,
            [&](ReadOnlyBuffer &buffer, TLVHead &head) { return buffer.ReadValue(recordId_, head); }},
    };
}

bool PasteData::DecodeTLV(ReadOnlyBuffer &buffer)
{
    for (; buffer.IsEnough();) {
        TLVHead head{};
        bool ret = buffer.ReadHead(head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "read head failed");

        const auto it = decodeMap_.find(head.tag);
        if (it == decodeMap_.end()) {
            ret = buffer.Skip(head.len);
        } else {
            ret = it->second(buffer, head);
        }

        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON,
            "read value failed, tag=%{public}hu, len=%{public}u", head.tag, head.len);
    }
    return true;
}

size_t PasteData::CountTLV() const
{
    size_t expectSize = 0;
    expectSize += TLVCountable::Count(props_);
    expectSize += TLVCountable::Count(records_);
    expectSize += TLVCountable::Count(isDraggedData_);
    expectSize += TLVCountable::Count(isLocalPaste_);
    expectSize += TLVCountable::Count(isDelayData_);
    expectSize += TLVCountable::Count(deviceId_);
    expectSize += TLVCountable::Count(pasteId_);
    expectSize += TLVCountable::Count(isDelayRecord_);
    expectSize += TLVCountable::Count(dataId_);
    expectSize += TLVCountable::Count(recordId_);
    return expectSize;
}

bool PasteData::Marshalling(Parcel &parcel) const
{
    std::vector<uint8_t> pasteDataTlv(0);
    bool ret = Encode(pasteDataTlv);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "encode failed");

    ret = parcel.WriteUInt8Vector(pasteDataTlv);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "write vector failed");
    return true;
}

PasteData *PasteData::Unmarshalling(Parcel &parcel)
{
    std::vector<uint8_t> pasteDataTlv(0);
    bool ret = parcel.ReadUInt8Vector(&pasteDataTlv);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, nullptr, PASTEBOARD_MODULE_COMMON, "read vector failed");
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(!pasteDataTlv.empty(), nullptr, PASTEBOARD_MODULE_COMMON, "vector empty");

    PasteData *pasteData = new (std::nothrow) PasteData();
    if (!pasteData->Decode(pasteDataTlv)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "decode failed");
        delete pasteData;
        pasteData = nullptr;
    }
    return pasteData;
}

bool PasteData::IsValid() const
{
    return valid_;
}

void PasteData::SetInvalid()
{
    valid_ = false;
}

void PasteData::SetDelayData(bool isDelay)
{
    isDelayData_ = isDelay;
}

bool PasteData::IsDelayData() const
{
    return isDelayData_;
}

void PasteData::SetDelayRecord(bool isDelay)
{
    isDelayRecord_ = isDelay;
}

bool PasteData::IsDelayRecord() const
{
    return isDelayRecord_;
}

void PasteData::SetDataId(uint32_t dataId)
{
    dataId_ = dataId;
}

uint32_t PasteData::GetDataId() const
{
    return dataId_;
}

uint32_t PasteData::GetRecordId() const
{
    return recordId_;
}

PasteDataProperty::PasteDataProperty(const PasteDataProperty &property)
    : tag(property.tag), timestamp(property.timestamp), localOnly(property.localOnly),
      shareOption(property.shareOption), tokenId(property.tokenId), isRemote(property.isRemote),
      bundleName(property.bundleName), setTime(property.setTime), screenStatus(property.screenStatus)
{
    this->additions = property.additions;
    std::copy(property.mimeTypes.begin(), property.mimeTypes.end(), std::back_inserter(this->mimeTypes));
}

PasteDataProperty::~PasteDataProperty()
{
    mimeTypes.clear();
}

PasteDataProperty &PasteDataProperty::operator=(const PasteDataProperty &property)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "PasteDataProperty copy");
    if (this == &property) {
        return *this;
    }
    this->tag = property.tag;
    this->timestamp = property.timestamp;
    this->localOnly = property.localOnly;
    this->shareOption = property.shareOption;
    this->tokenId = property.tokenId;
    this->isRemote = property.isRemote;
    this->bundleName = property.bundleName;
    this->setTime = property.setTime;
    this->screenStatus = property.screenStatus;
    this->additions = property.additions;
    this->mimeTypes.clear();
    std::copy(property.mimeTypes.begin(), property.mimeTypes.end(), std::back_inserter(this->mimeTypes));
    return *this;
}

bool PasteDataProperty::EncodeTLV(WriteOnlyBuffer &buffer) const
{
    bool ret = buffer.Write(TAG_ADDITIONS, TLVUtils::Parcelable2Raw(&additions));
    ret = ret && buffer.Write(TAG_MIMETYPES, mimeTypes);
    ret = ret && buffer.Write(TAG_TAG, tag);
    ret = ret && buffer.Write(TAG_LOCAL_ONLY, localOnly);
    ret = ret && buffer.Write(TAG_TIMESTAMP, timestamp);
    ret = ret && buffer.Write(TAG_SHAREOPTION, static_cast<int32_t>(shareOption));
    ret = ret && buffer.Write(TAG_TOKENID, tokenId);
    ret = ret && buffer.Write(TAG_ISREMOTE, isRemote);
    ret = ret && buffer.Write(TAG_BUNDLENAME, bundleName);
    ret = ret && buffer.Write(TAG_SETTIME, setTime);
    ret = ret && buffer.Write(TAG_SCREENSTATUS, static_cast<int32_t>(screenStatus));
    return ret;
}

bool PasteDataProperty::DecodeTLV(ReadOnlyBuffer &buffer)
{
    for (; buffer.IsEnough();) {
        TLVHead head{};
        bool ret = buffer.ReadHead(head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "read head failed");
        if (head.tag == TAG_ADDITIONS) {
            RawMem rawMem{};
            ret = buffer.ReadValue(rawMem, head);
            auto buff = TLVUtils::Raw2Parcelable<AAFwk::WantParams>(rawMem);
            additions = (buff != nullptr) ? *buff : additions;
        } else if (head.tag == TAG_MIMETYPES) {
            ret = buffer.ReadValue(mimeTypes, head);
        } else if (head.tag == TAG_TAG) {
            ret = buffer.ReadValue(tag, head);
        } else if (head.tag == TAG_LOCAL_ONLY) {
            ret = buffer.ReadValue(localOnly, head);
        } else if (head.tag == TAG_TIMESTAMP) {
            ret = buffer.ReadValue(timestamp, head);
        } else if (head.tag == TAG_SHAREOPTION) {
            ret = buffer.ReadValue((int32_t &)shareOption, head);
        } else if (head.tag == TAG_TOKENID) {
            ret = buffer.ReadValue(tokenId, head);
        } else if (head.tag == TAG_ISREMOTE) {
            ret = buffer.ReadValue(isRemote, head);
        } else if (head.tag == TAG_BUNDLENAME) {
            ret = buffer.ReadValue(bundleName, head);
        } else if (head.tag == TAG_SETTIME) {
            ret = buffer.ReadValue(setTime, head);
        } else if (head.tag == TAG_SCREENSTATUS) {
            ret = buffer.ReadValue((int32_t &)screenStatus, head);
        } else {
            ret = buffer.Skip(head.len);
        }
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON,
            "read value failed, tag=%{public}hu, len=%{public}u", head.tag, head.len);
    }
    return true;
}

size_t PasteDataProperty::CountTLV() const
{
    size_t expectedSize = 0;
    expectedSize += TLVCountable::Count(TLVUtils::Parcelable2Raw(&additions));
    expectedSize += TLVCountable::Count(mimeTypes);
    expectedSize += TLVCountable::Count(tag);
    expectedSize += TLVCountable::Count(localOnly);
    expectedSize += TLVCountable::Count(timestamp);
    expectedSize += TLVCountable::Count(shareOption);
    expectedSize += TLVCountable::Count(tokenId);
    expectedSize += TLVCountable::Count(isRemote);
    expectedSize += TLVCountable::Count(bundleName);
    expectedSize += TLVCountable::Count(setTime);
    expectedSize += TLVCountable::Count(screenStatus);
    return expectedSize;
}

void PasteData::ShareOptionToString(ShareOption shareOption, std::string &out)
{
    if (shareOption == ShareOption::InApp) {
        out = "InAPP";
    } else if (shareOption == ShareOption::LocalDevice) {
        out = "LocalDevice";
    } else {
        out = "CrossDevice";
    }
}

std::string PasteData::GetDeviceId() const
{
    return deviceId_;
}

void PasteData::SetPasteId(const std::string &pasteId)
{
    pasteId_ = pasteId;
}

std::string PasteData::GetPasteId() const
{
    return pasteId_;
}
} // namespace MiscServices
} // namespace OHOS