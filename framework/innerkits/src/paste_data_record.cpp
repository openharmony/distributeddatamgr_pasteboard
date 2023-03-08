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
#include "paste_data_record.h"

#include <sys/stat.h>
#include <unistd.h>

#include "copy_uri_handler.h"
#include "parcel_util.h"
#include "paste_uri_handler.h"
#include "pasteboard_error.h"
#include "pixel_map_parcel.h"
#include "pasteboard_hilog.h"

using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
namespace {
constexpr int MAX_TEXT_LEN = 20 * 1024 * 1024;
}

PasteDataRecord::Builder &PasteDataRecord::Builder::SetMimeType(std::string mimeType)
{
    record_->mimeType_ = std::move(mimeType);
    return *this;
}
enum TAG_PASTEBOARD_RECORD : uint16_t {
    TAG_MIMETYPE = TAG_BUFF + 1,
    TAG_HTMLTEXT,
    TAG_WANT,
    TAG_PLAINTEXT,
    TAG_URI,
    TAG_PIXELMAP,
    TAG_CUSTOM_DATA,
    TAG_CONVERT_URI,
};

enum TAG_CUSTOMDATA : uint16_t {
    TAG_ITEM_DATA = TAG_BUFF + 1,
};

PasteDataRecord::Builder &PasteDataRecord::Builder::SetHtmlText(std::shared_ptr<std::string> htmlText)
{
    record_->htmlText_ = std::move(htmlText);
    return *this;
}
PasteDataRecord::Builder &PasteDataRecord::Builder::SetWant(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    record_->want_ = std::move(want);
    return *this;
}
PasteDataRecord::Builder &PasteDataRecord::Builder::SetPlainText(std::shared_ptr<std::string> plainText)
{
    record_->plainText_ = std::move(plainText);
    return *this;
}
PasteDataRecord::Builder &PasteDataRecord::Builder::SetUri(std::shared_ptr<OHOS::Uri> uri)
{
    record_->uri_ = std::move(uri);
    return *this;
}
PasteDataRecord::Builder &PasteDataRecord::Builder::SetPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap)
{
    record_->pixelMap_ = std::move(pixelMap);
    return *this;
}

PasteDataRecord::Builder &PasteDataRecord::Builder::SetCustomData(std::shared_ptr<MineCustomData> customData)
{
    record_->customData_ = std::move(customData);
    return *this;
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::Builder::Build()
{
    return record_;
}
PasteDataRecord::Builder::Builder(const std::string &mimeType)
{
    record_ = std::make_shared<PasteDataRecord>();
    if (record_ != nullptr) {
        record_->mimeType_ = mimeType;
        record_->htmlText_ = nullptr;
        record_->want_ = nullptr;
        record_->plainText_ = nullptr;
        record_->uri_ = nullptr;
        record_->pixelMap_ = nullptr;
        record_->customData_ = nullptr;
    }
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewHtmlRecord(const std::string &htmlText)
{
    if (htmlText.length() >= MAX_TEXT_LEN) {
        return nullptr;
    }
    return Builder(MIMETYPE_TEXT_HTML).SetHtmlText(std::make_shared<std::string>(htmlText)).Build();
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    return Builder(MIMETYPE_TEXT_WANT).SetWant(std::move(want)).Build();
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewPlaintTextRecord(const std::string &text)
{
    if (text.length() >= MAX_TEXT_LEN) {
        return nullptr;
    }
    return Builder(MIMETYPE_TEXT_PLAIN).SetPlainText(std::make_shared<std::string>(text)).Build();
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewPixelMapRecord(std::shared_ptr<PixelMap> pixelMap)
{
    return Builder(MIMETYPE_PIXELMAP).SetPixelMap(std::move(pixelMap)).Build();
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewUriRecord(const OHOS::Uri &uri)
{
    return Builder(MIMETYPE_TEXT_URI).SetUri(std::make_shared<OHOS::Uri>(uri)).Build();
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewKvRecord(const std::string &mimeType,
    const std::vector<uint8_t> &arrayBuffer)
{
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    customData->AddItemData(mimeType, arrayBuffer);
    return Builder(mimeType).SetCustomData(std::move(customData)).Build();
}

PasteDataRecord::PasteDataRecord(std::string mimeType, std::shared_ptr<std::string> htmlText,
    std::shared_ptr<OHOS::AAFwk::Want> want, std::shared_ptr<std::string> plainText, std::shared_ptr<OHOS::Uri> uri)
    : mimeType_{ std::move(mimeType) }, htmlText_{ std::move(htmlText) }, want_{ std::move(want) },
      plainText_{ std::move(plainText) }, uri_{ std::move(uri) }
{
}

PasteDataRecord::PasteDataRecord()
{
    fd_ = std::make_shared<FileDescriptor>();
}

std::shared_ptr<std::string> PasteDataRecord::GetHtmlText() const
{
    return this->htmlText_;
}

std::string PasteDataRecord::GetMimeType() const
{
    return this->mimeType_;
}

std::shared_ptr<std::string> PasteDataRecord::GetPlainText() const
{
    return this->plainText_;
}

std::shared_ptr<PixelMap> PasteDataRecord::GetPixelMap() const
{
    return this->pixelMap_;
}

std::shared_ptr<OHOS::Uri> PasteDataRecord::GetUri() const
{
    if (convertUri_.empty()) {
        return uri_;
    }
    return std::make_shared<OHOS::Uri>(convertUri_);
}

std::shared_ptr<OHOS::AAFwk::Want> PasteDataRecord::GetWant() const
{
    return this->want_;
}

std::shared_ptr<MineCustomData> PasteDataRecord::GetCustomData() const
{
    return this->customData_;
}

std::map<std::string, std::vector<uint8_t>> MineCustomData::GetItemData()
{
    return this->itemData_;
}

void MineCustomData::AddItemData(const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer)
{
    itemData_.insert(std::make_pair(mimeType, arrayBuffer));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "itemData_.size = %{public}zu", itemData_.size());
}

std::string PasteDataRecord::ConvertToText() const
{
    if (this->htmlText_) {
        return *this->htmlText_;
    } else if (this->plainText_) {
        return *this->plainText_;
    } else if (this->uri_) {
        return this->uri_->ToString();
    } else {
        return "";
    }
}

bool PasteDataRecord::Marshalling(Parcel &parcel, std::shared_ptr<std::string> item)
{
    if (!parcel.WriteBool(item != nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteBool failed.");
        return false;
    }
    if (item == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "no data provide.");
        return true;
    }
    return parcel.WriteString16(Str8ToStr16(*item));
}

bool PasteDataRecord::Marshalling(Parcel &parcel, std::shared_ptr<Parcelable> item)
{
    if (!parcel.WriteBool(item != nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteBool failed.");
        return false;
    }
    if (item == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "no data provide.");
        return true;
    }
    return parcel.WriteParcelable(item.get());
}

bool PasteDataRecord::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(mimeType_))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write mimeType failed, mimeType = %{public}s.", mimeType_.c_str());
        return false;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "mimeType = %{public}s,", mimeType_.c_str());
    bool ret = Marshalling(parcel, htmlText_);
    ret = Marshalling(parcel, want_) && ret;
    ret = Marshalling(parcel, plainText_) && ret;
    ret = Marshalling(parcel, uri_) && ret;
    ret = Marshalling(parcel, pixelMap_) && ret;
    ret = Marshalling(parcel, customData_) && ret;
    return ret;
}

template<typename T> ResultCode PasteDataRecord::UnMarshalling(Parcel &parcel, std::shared_ptr<T> &item)
{
    if (!parcel.ReadBool()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "no data provide.");
        return ResultCode::IPC_NO_DATA;
    }
    std::shared_ptr<T> parcelAble(parcel.ReadParcelable<T>());
    if (!parcelAble) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "ReadParcelable failed.");
        return ResultCode::IPC_ERROR;
    }
    item = move(parcelAble);
    return ResultCode::OK;
}

ResultCode PasteDataRecord::UnMarshalling(Parcel &parcel, std::shared_ptr<std::string> &item)
{
    if (!parcel.ReadBool()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "no data provide.");
        return ResultCode::IPC_NO_DATA;
    }
    item = std::make_shared<std::string>(Str16ToStr8(parcel.ReadString16()));
    if (!item) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "ReadString16 failed.");
        return ResultCode::IPC_ERROR;
    }
    return ResultCode::OK;
}

PasteDataRecord *PasteDataRecord::Unmarshalling(Parcel &parcel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    auto pasteDataRecord = new (std::nothrow) PasteDataRecord();

    if (pasteDataRecord == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "pasteDataRecord is nullptr.");
        return pasteDataRecord;
    }

    pasteDataRecord->mimeType_ = Str16ToStr8(parcel.ReadString16());
    if (pasteDataRecord->mimeType_.empty()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "ReadString16 failed.");
        delete pasteDataRecord;
        pasteDataRecord = nullptr;
        return pasteDataRecord;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "mimeType = %{public}s,", pasteDataRecord->mimeType_.c_str());

    ResultCode resultCode = UnMarshalling(parcel, pasteDataRecord->htmlText_);
    auto ret = CheckResult(resultCode);
    resultCode = UnMarshalling(parcel, pasteDataRecord->want_);
    ret = CheckResult(resultCode) || ret;
    resultCode = UnMarshalling(parcel, pasteDataRecord->plainText_);
    ret = CheckResult(resultCode) || ret;
    resultCode = UnMarshalling(parcel, pasteDataRecord->uri_);
    ret = CheckResult(resultCode) || ret;
    resultCode = UnMarshalling(parcel, pasteDataRecord->pixelMap_);
    ret = CheckResult(resultCode) || ret;
    resultCode = UnMarshalling(parcel, pasteDataRecord->customData_);
    ret = CheckResult(resultCode) || ret;
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "UnMarshalling failed.");
        delete pasteDataRecord;
        pasteDataRecord = nullptr;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return pasteDataRecord;
}

bool MineCustomData::Marshalling(Parcel &parcel) const
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "begin.");
    auto len = itemData_.size();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "itemData_len = %{public}zu,", len);
    if (!parcel.WriteUint32(static_cast<uint32_t>(len))) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "len Marshalling failed.");
        return false;
    }
    for (const auto &item : itemData_) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "dataLen = %{public}zu!", item.second.size());
        if (!parcel.WriteString(item.first) || !parcel.WriteUInt8Vector(item.second)) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "write failed.");
            return false;
        }
    }

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return true;
}

MineCustomData *MineCustomData::Unmarshalling(Parcel &parcel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "begin.");
    auto *mineCustomData = new (std::nothrow) MineCustomData();

    if (mineCustomData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "mineCustomData is nullptr.");
        return mineCustomData;
    }

    uint32_t failedNums = 0;
    auto len = parcel.ReadUint32();
    if (len <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "length error");
        delete mineCustomData;
        mineCustomData = nullptr;
        return mineCustomData;
    }
    for (uint32_t i = 0; i < len; i++) {
        std::string mimeType = parcel.ReadString();
        std::vector<uint8_t> arrayBuffer;
        if (!parcel.ReadUInt8Vector(&arrayBuffer) || arrayBuffer.empty()) {
            failedNums++;
            continue;
        }
        mineCustomData->AddItemData(mimeType, arrayBuffer);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "dataLen = %{public}zu.", arrayBuffer.size());
    }

    if (failedNums == len) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "mineCustomData is nullptr.");
        delete mineCustomData;
        mineCustomData = nullptr;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return mineCustomData;
}

bool MineCustomData::Encode(std::vector<std::uint8_t> &buffer)
{
    return TLVObject::Write(buffer, TAG_ITEM_DATA, itemData_);
}

bool MineCustomData::Decode(const std::vector<std::uint8_t> &buffer)
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

bool PasteDataRecord::Encode(std::vector<std::uint8_t> &buffer)
{
    bool ret = Write(buffer, TAG_MIMETYPE, mimeType_);
    ret = Write(buffer, TAG_HTMLTEXT, htmlText_) && ret;
    ret = Write(buffer, TAG_WANT, ParcelUtil::Parcelable2Raw(want_.get())) && ret;
    ret = Write(buffer, TAG_PLAINTEXT, plainText_) && ret;
    ret = Write(buffer, TAG_URI, ParcelUtil::Parcelable2Raw(uri_.get())) && ret;
    ret = Write(buffer, TAG_CONVERT_URI, convertUri_) && ret;
    ret = Write(buffer, TAG_PIXELMAP, PixelMap2Raw(pixelMap_)) && ret;
    ret = Write(buffer, TAG_CUSTOM_DATA, customData_) && ret;
    return ret;
}

bool PasteDataRecord::Decode(const std::vector<std::uint8_t> &buffer)
{
    for (; IsEnough();) {
        TLVHead head{};
        bool ret = ReadHead(buffer, head);
        switch (head.tag) {
            case TAG_MIMETYPE:
                ret = ret && ReadValue(buffer, mimeType_, head);
                break;
            case TAG_HTMLTEXT:
                ret = ret && ReadValue(buffer, htmlText_, head);
                break;
            case TAG_WANT: {
                RawMem rawMem{};
                ret = ret && ReadValue(buffer, rawMem, head);
                want_ = ParcelUtil::Raw2Parcelable<AAFwk::Want>(rawMem);
                break;
            }
            case TAG_PLAINTEXT:
                ret = ret && ReadValue(buffer, plainText_, head);
                break;
            case TAG_URI: {
                RawMem rawMem{};
                ret = ret && ReadValue(buffer, rawMem, head);
                uri_ = ParcelUtil::Raw2Parcelable<OHOS::Uri>(rawMem);
                break;
            }
            case TAG_CONVERT_URI: {
                ret = ret && ReadValue(buffer, convertUri_, head);
                break;
            }
            case TAG_PIXELMAP: {
                RawMem rawMem{};
                ret = ret && ReadValue(buffer, rawMem, head);
                pixelMap_ = Raw2PixelMap(rawMem);
                break;
            }
            case TAG_CUSTOM_DATA:
                ret = ret && ReadValue(buffer, customData_, head);
                break;
            default:
                ret = ret && Skip(head.len, buffer.size());
                break;
        }
        if (!ret) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "read value,tag:%{public}u, len:%{public}u",
                head.tag, head.len);
            return false;
        }
    }
    return true;
}
size_t PasteDataRecord::Count()
{
    size_t expectedSize = 0;
    expectedSize += TLVObject::Count(mimeType_);
    expectedSize += TLVObject::Count(htmlText_);
    expectedSize += TLVObject::Count(ParcelUtil::Parcelable2Raw(want_.get()));
    expectedSize += TLVObject::Count(plainText_);
    expectedSize += TLVObject::Count(ParcelUtil::Parcelable2Raw(uri_.get()));
    expectedSize += TLVObject::Count(convertUri_);
    expectedSize += TLVObject::Count(PixelMap2Raw(pixelMap_));
    expectedSize += TLVObject::Count(customData_);
    return expectedSize;
}

std::shared_ptr<PixelMap> PasteDataRecord::Raw2PixelMap(const RawMem &rawMem)
{
    if (rawMem.buffer == 0 || rawMem.bufferLen == 0) {
        return nullptr;
    }
    MessageParcel data;
    if (!ParcelUtil::Raw2Parcel(rawMem, data)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "raw to parcel failed");
        return nullptr;
    }
    return PixelMapParcel::CreateFromParcel(data);
}

RawMem PasteDataRecord::PixelMap2Raw(const std::shared_ptr<PixelMap> &pixelMap)
{
    RawMem rawMem{ 0 };
    if (pixelMap == nullptr) {
        return rawMem;
    }
    auto data = std::make_shared<MessageParcel>();
    if (!PixelMapParcel::WriteToParcel(pixelMap.get(), *data)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "parcel to raw failed");
        return rawMem;
    }
    rawMem.parcel = data;
    rawMem.buffer = data->GetData();
    rawMem.bufferLen = data->GetDataSize();
    return rawMem;
}

bool PasteDataRecord::WriteFd(MessageParcel &parcel, UriHandler &uriHandler, bool isClient)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "isClient: %{public}d", isClient);
    if (fd_->GetFd() >= 0) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "write fd_, fd_ is %{public}d", fd_->GetFd());
        return parcel.WriteFileDescriptor(fd_->GetFd());
    }
    std::string tempUri = GetPassUri();
    if (tempUri.empty()) {
        return false;
    }
    int32_t fd = uriHandler.ToFd(tempUri, isClient);
    bool ret = parcel.WriteFileDescriptor(fd);
    uriHandler.ReleaseFd(fd);

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "ret is %{public}d", ret);
    return ret;
}
bool PasteDataRecord::ReadFd(MessageParcel &parcel, UriHandler &uriHandler)
{
    int32_t fd = parcel.ReadFileDescriptor();
    if (fd >= 0) {
        convertUri_ = uriHandler.ToUri(fd);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "convertUri_:%{public}s", convertUri_.c_str());
    }
    if (!uriHandler.IsPaste()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Set fd, fd is %{public}d", fd);
        fd_->SetFd(fd);
    }
    return true;
}
bool PasteDataRecord::NeedFd(const UriHandler &uriHandler)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start");
    std::string tempUri = GetPassUri();
    if (tempUri.empty()) {
        return false;
    }
    if (!uriHandler.IsFile(tempUri) && fd_->GetFd() < 0) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "invalid file uri, fd:%{public}d", fd_->GetFd());
        return false;
    }
    return true;
}
std::string PasteDataRecord::GetPassUri()
{
    std::string tempUri;
    if (uri_ != nullptr) {
        tempUri = uri_->ToString();
    }
    if (!convertUri_.empty()) {
        tempUri = convertUri_;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "tempUri:%{public}s", tempUri.c_str());
    return tempUri;
}
void PasteDataRecord::ReplaceShareUri(int32_t userId)
{
    if (convertUri_.empty()) {
        return;
    }

    // convert uri format: /mnt/hmdfs/100/account/merge_view/services/psteboard_service/.share/xxx.txt
    constexpr const char *SHARE_PATH_PREFIX = "/mnt/hmdfs/";
    auto frontPos = convertUri_.find(SHARE_PATH_PREFIX);
    auto rearPos = convertUri_.find("/account/");
    if (frontPos == 0 && rearPos != std::string::npos) {
        convertUri_ = SHARE_PATH_PREFIX + std::to_string(userId) + convertUri_.substr(rearPos);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "replace uri:%{public}s", convertUri_.c_str());
    }
}
void PasteDataRecord::SetConvertUri(const std::string &value)
{
    convertUri_ = value;
}
FileDescriptor::~FileDescriptor()
{
    if (fd_ >= 0) {
        close(fd_);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "close fd_: %{public}d", fd_);
    }
}
void FileDescriptor::SetFd(int32_t fd)
{
    fd_ = fd;
}
int32_t FileDescriptor::GetFd() const
{
    return fd_;
}
} // namespace MiscServices
} // namespace OHOS