
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

#include <new>

#include "parcel_util.h"
#include "paste_data.h"
#include "paste_data_record.h"
#include "pasteboard_hilog.h"
#include "type_traits"

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
const std::string PasteData::FILE_SCHEME_PREFIX = "file://";
const std::string PasteData::IMG_LOCAL_URI = "file:///";
const std::string PasteData::SHARE_PATH_PREFIX = "/mnt/hmdfs/";
const std::string PasteData::SHARE_PATH_PREFIX_ACCOUNT = "/account/merge_view/services/";
const std::string PasteData::REMOTE_FILE_SIZE = "remoteFileSize";
const std::string PasteData::REMOTE_FILE_SIZE_LONG = "remoteFileSizeLong";

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
}

PasteData::PasteData(const PasteData &data)
    : orginAuthority_(data.orginAuthority_), valid_(data.valid_), isDraggedData_(data.isDraggedData_),
      isLocalPaste_(data.isLocalPaste_), isDelayData_(data.isDelayData_), pasteId_(data.pasteId_),
      isDelayRecord_(data.isDelayRecord_), dataId_(data.dataId_)
{
    this->props_ = data.props_;
    for (const auto &item : data.records_) {
        this->records_.emplace_back(std::make_shared<PasteDataRecord>(*item));
    }
    InitDecodeMap();
}

PasteData::PasteData(std::vector<std::shared_ptr<PasteDataRecord>> records) : records_{ std::move(records) }
{
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
    this->orginAuthority_ = data.orginAuthority_;
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
    if (record == nullptr) {
        return;
    }
    record->SetRecordId(++recordId_);
    records_.insert(records_.begin(), std::move(record));
    RefreshMimeProp();
}

void PasteData::AddRecord(PasteDataRecord &record)
{
    this->AddRecord(std::make_shared<PasteDataRecord>(record));
    RefreshMimeProp();
}

std::vector<std::string> PasteData::GetMimeTypes()
{
    std::vector<std::string> mimeType;
    for (const auto &item : records_) {
        mimeType.push_back(item->GetMimeType());
    }
    return mimeType;
}

std::shared_ptr<std::string> PasteData::GetPrimaryHtml()
{
    for (const auto &item : records_) {
        if (item->GetHtmlText() != nullptr) {
            return item->GetHtmlText();
        }
    }
    return nullptr;
}

std::shared_ptr<PixelMap> PasteData::GetPrimaryPixelMap()
{
    for (const auto &item : records_) {
        if (item->GetPixelMap() != nullptr) {
            return item->GetPixelMap();
        }
    }
    return nullptr;
}

std::shared_ptr<OHOS::AAFwk::Want> PasteData::GetPrimaryWant()
{
    for (const auto &item : records_) {
        if (item->GetWant() != nullptr) {
            return item->GetWant();
        }
    }
    return nullptr;
}

std::shared_ptr<std::string> PasteData::GetPrimaryText()
{
    for (const auto &item : records_) {
        if (item->GetPlainText() != nullptr) {
            return item->GetPlainText();
        }
    }
    return nullptr;
}

std::shared_ptr<OHOS::Uri> PasteData::GetPrimaryUri()
{
    for (const auto &item : records_) {
        if (item->GetUri() != nullptr) {
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

std::shared_ptr<PasteDataRecord> PasteData::GetRecordAt(std::size_t index) const
{
    if (records_.size() > index) {
        return records_[index];
    } else {
        return nullptr;
    }
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
    for (auto &item : records_) {
        if (item->GetMimeType() == mimeType) {
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

void PasteData::SetOrginAuthority(const std::string &bundleName)
{
    orginAuthority_ = bundleName;
}

std::string PasteData::GetOrginAuthority() const
{
    return orginAuthority_;
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

void PasteData::SetTag(std::string &tag)
{
    props_.tag = tag;
}

std::string PasteData::GetTag()
{
    return props_.tag;
}

void PasteData::SetAdditions(AAFwk::WantParams &additions)
{
    props_.additions = additions;
}

void PasteData::SetAddition(const std::string &key, AAFwk::IInterface *value)
{
    props_.additions.SetParam(key, value);
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
        mimeTypes.insert(mimeTypes.end(), record->GetMimeType());
    }
    props_.mimeTypes = mimeTypes;
}

bool PasteData::Encode(std::vector<std::uint8_t> &buffer)
{
    Init(buffer);

    bool ret = Write(buffer, TAG_PROPS, props_);
    ret = Write(buffer, TAG_RECORDS, records_) && ret;
    ret = Write(buffer, TAG_DRAGGED_DATA_FLAG, isDraggedData_) && ret;
    ret = Write(buffer, TAG_LOCAL_PASTE_FLAG, isLocalPaste_) && ret;
    ret = Write(buffer, TAG_DELAY_DATA_FLAG, isDelayData_) && ret;
    ret = Write(buffer, TAG_DEVICE_ID, deviceId_) && ret;
    ret = Write(buffer, TAG_PASTE_ID, pasteId_) && ret;
    ret = Write(buffer, TAG_DELAY_RECORD_FLAG, isDelayRecord_) && ret;
    ret = Write(buffer, TAG_DATA_ID, dataId_) && ret;
    return ret;
}

void PasteData::InitDecodeMap()
{
    decodeMap_ = {
        { TAG_PROPS,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, props_, head);
            } },
        { TAG_RECORDS,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, records_, head);
            } },
        { TAG_DRAGGED_DATA_FLAG,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, isDraggedData_, head);
            } },
        { TAG_LOCAL_PASTE_FLAG,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, isLocalPaste_, head);
            } },
        { TAG_DELAY_DATA_FLAG,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, isDelayData_, head);
            } },
        { TAG_DEVICE_ID,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, deviceId_, head);
            } },
        { TAG_PASTE_ID,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, pasteId_, head);
            } },
        { TAG_DELAY_RECORD_FLAG,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, isDelayRecord_, head);
            } },
        { TAG_DATA_ID,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, dataId_, head);
            } },
    };
}

bool PasteData::Decode(const std::vector<std::uint8_t> &buffer)
{
    total_ = buffer.size();
    for (; IsEnough();) {
        TLVHead head{};
        bool ret = ReadHead(buffer, head);
        auto it = decodeMap_.find(head.tag);
        if (it == decodeMap_.end()) {
            ret = ret && Skip(head.len, buffer.size());
        } else {
            auto func = it->second;
            func(ret, buffer, head);
        }
        if (!ret) {
            PASTEBOARD_HILOGE(
                PASTEBOARD_MODULE_CLIENT, "read value,tag:%{public}u, len:%{public}u", head.tag, head.len);
            return false;
        }
    }
    return true;
}

size_t PasteData::Count()
{
    size_t expectSize = 0;
    expectSize += props_.Count() + sizeof(TLVHead);
    expectSize += TLVObject::Count(records_);
    expectSize += TLVObject::Count(isDraggedData_);
    expectSize += TLVObject::Count(isLocalPaste_);
    expectSize += TLVObject::Count(isDelayData_);
    expectSize += TLVObject::Count(deviceId_);
    expectSize += TLVObject::Count(pasteId_);
    expectSize += TLVObject::Count(isDelayRecord_);
    expectSize += TLVObject::Count(dataId_);
    return expectSize;
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

bool PasteData::Marshalling(Parcel &parcel) const
{
    std::vector<uint8_t> pasteDataTlv(0);
    if (!const_cast<PasteData *>(this)->Encode(pasteDataTlv)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Encode failed");
        return false;
    }
    if (!parcel.WriteUInt8Vector(pasteDataTlv)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteUInt8Vector failed");
        return false;
    }
    return true;
}

PasteData *PasteData::Unmarshalling(Parcel &parcel)
{
    PasteData *pasteData = new (std::nothrow) PasteData();
    if (pasteData != nullptr && !pasteData->ReadFromParcel(parcel)) {
        delete pasteData;
        pasteData = nullptr;
    }
    return pasteData;
}

bool PasteData::ReadFromParcel(Parcel &parcel)
{
    std::vector<uint8_t> pasteDataTlv(0);
    if (!parcel.ReadUInt8Vector(&pasteDataTlv)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "ReadUInt8Vector failed");
        return false;
    }
    if (pasteDataTlv.size() == 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "ReadFromParcel size = 0");
        return false;
    }
    if (!Decode(pasteDataTlv)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Decode failed");
        return false;
    }
    return true;
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

bool PasteDataProperty::Encode(std::vector<std::uint8_t> &buffer)
{
    bool ret = Write(buffer, TAG_ADDITIONS, ParcelUtil::Parcelable2Raw(&additions));
    ret = Write(buffer, TAG_MIMETYPES, mimeTypes) && ret;
    ret = Write(buffer, TAG_TAG, tag) && ret;
    ret = Write(buffer, TAG_LOCAL_ONLY, localOnly) && ret;
    ret = Write(buffer, TAG_TIMESTAMP, timestamp) && ret;
    ret = Write(buffer, TAG_SHAREOPTION, (int32_t &)shareOption) && ret;
    ret = Write(buffer, TAG_TOKENID, tokenId) && ret;
    ret = Write(buffer, TAG_ISREMOTE, isRemote) && ret;
    ret = Write(buffer, TAG_BUNDLENAME, bundleName) && ret;
    ret = Write(buffer, TAG_SETTIME, setTime) && ret;
    ret = Write(buffer, TAG_SCREENSTATUS, (int32_t &)screenStatus) && ret;
    return ret;
}

bool PasteDataProperty::Decode(const std::vector<std::uint8_t> &buffer)
{
    for (; IsEnough();) {
        if (!DecodeTag(buffer)) {
            return false;
        }
    }
    return true;
}

bool PasteDataProperty::DecodeTag(const std::vector<std::uint8_t> &buffer)
{
    TLVHead head{};
    bool ret = ReadHead(buffer, head);
    switch (head.tag) {
        case TAG_ADDITIONS: {
            RawMem rawMem{};
            ret = ret && ReadValue(buffer, rawMem, head);
            auto buff = ParcelUtil::Raw2Parcelable<AAFwk::WantParams>(rawMem);
            if (buff != nullptr) {
                additions = *buff;
            }
            break;
        }
        case TAG_MIMETYPES:
            ret = ret && ReadValue(buffer, mimeTypes, head);
            break;
        case TAG_TAG:
            ret = ret && ReadValue(buffer, tag, head);
            break;
        case TAG_LOCAL_ONLY:
            ret = ret && ReadValue(buffer, localOnly, head);
            break;
        case TAG_TIMESTAMP:
            ret = ret && ReadValue(buffer, timestamp, head);
            break;
        case TAG_SHAREOPTION:
            ret = ret && ReadValue(buffer, (int32_t &)shareOption, head);
            break;
        case TAG_TOKENID:
            ret = ret && ReadValue(buffer, tokenId, head);
            break;
        case TAG_ISREMOTE:
            ret = ret && ReadValue(buffer, isRemote, head);
            break;
        case TAG_BUNDLENAME:
            ret = ret && ReadValue(buffer, bundleName, head);
            break;
        case TAG_SETTIME:
            ret = ret && ReadValue(buffer, setTime, head);
            break;
        case TAG_SCREENSTATUS:
            ret = ret && ReadValue(buffer, (int32_t &)screenStatus, head);
            break;
        default:
            ret = ret && Skip(head.len, buffer.size());
            break;
    }
    return ret;
}

size_t PasteDataProperty::Count()
{
    size_t expectedSize = 0;
    expectedSize += TLVObject::Count(ParcelUtil::Parcelable2Raw(&additions));
    expectedSize += TLVObject::Count(mimeTypes);
    expectedSize += TLVObject::Count(tag);
    expectedSize += TLVObject::Count(localOnly);
    expectedSize += TLVObject::Count(timestamp);
    expectedSize += TLVObject::Count(shareOption);
    expectedSize += TLVObject::Count(tokenId);
    expectedSize += TLVObject::Count(isRemote);
    expectedSize += TLVObject::Count(bundleName);
    expectedSize += TLVObject::Count(setTime);
    expectedSize += TLVObject::Count(screenStatus);
    return expectedSize;
}

bool PasteData::WriteUriFd(MessageParcel &parcel, UriHandler &uriHandler, bool isClient)
{
    std::vector<uint32_t> uriIndexList;
    for (size_t i = 0; i < GetRecordCount(); ++i) {
        auto record = GetRecordAt(i);
        if (record == nullptr) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "record null");
            continue;
        }
        if (isLocalPaste_) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "isLocalPaste");
            continue;
        }
        if (record->NeedFd(uriHandler)) {
            uriIndexList.push_back(i);
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "write fd vector size:%{public}zu", uriIndexList.size());
    if (!parcel.WriteUInt32Vector(uriIndexList)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write fd index vector");
        return false;
    }
    for (auto index : uriIndexList) {
        auto record = GetRecordAt(index);
        if (record == nullptr || !record->WriteFd(parcel, uriHandler, isClient)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write fd");
            return false;
        }
    }
    return true;
}

bool PasteData::ReadUriFd(MessageParcel &parcel, UriHandler &uriHandler)
{
    std::vector<uint32_t> fdRecordMap;
    if (!parcel.ReadUInt32Vector(&fdRecordMap)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to read fd index vector");
        return false;
    }
    for (auto index : fdRecordMap) {
        auto record = GetRecordAt(index);
        if (record == nullptr || !record->ReadFd(parcel, uriHandler)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to read fd");
            return false;
        }
    }
    return true;
}

void PasteData::ReplaceShareUri(int32_t userId)
{
    auto count = GetRecordCount();
    for (size_t i = 0; i < count; ++i) {
        auto record = GetRecordAt(i);
        if (record == nullptr) {
            continue;
        }
        record->ReplaceShareUri(userId);
    }
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