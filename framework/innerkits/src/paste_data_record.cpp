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
#include <sys/stat.h>
#include <unistd.h>

#include "convert_utils.h"
#include "copy_uri_handler.h"
#include "parcel_util.h"
#include "paste_data_record.h"
#include "pasteboard_hilog.h"
#include "pixel_map_parcel.h"

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
    TAG_URI_PERMISSION,
    TAG_UDC_UDTYPE,
    TAG_UDC_DETAILS,
    TAG_UDC_TEXTCONTENT,
    TAG_UDC_SYSTEMCONTENTS,
    TAG_UDC_UDMFVALUE,
    TAG_UDC_ENTYIES,
    TAG_DATA_ID,
    TAG_RECORD_ID,
    TAG_DELAY_RECORD_FLAG,
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
        record_->convertUri_ = "";
        record_->pixelMap_ = nullptr;
        record_->customData_ = nullptr;
    }
}

void PasteDataRecord::AddUriEntry()
{
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI);
    if (uri_ != nullptr) {
        object->value_[UDMF::FILE_URI_PARAM] = uri_->ToString();
    }
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI);
    AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
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

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewKvRecord(
    const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer)
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
    InitDecodeMap();
}

PasteDataRecord::~PasteDataRecord()
{
    decodeMap.clear();
}

PasteDataRecord::PasteDataRecord(const PasteDataRecord &record)
    : mimeType_(record.mimeType_), htmlText_(record.htmlText_), want_(record.want_), plainText_(record.plainText_),
      uri_(record.uri_), convertUri_(record.convertUri_), pixelMap_(record.pixelMap_), customData_(record.customData_),
      hasGrantUriPermission_(record.hasGrantUriPermission_), fd_(record.fd_), udType_(record.udType_),
      details_(record.details_), textContent_(record.textContent_),
      systemDefinedContents_(record.systemDefinedContents_), udmfValue_(record.udmfValue_), entries_(record.entries_),
      dataId_(record.dataId_), recordId_(record.recordId_), isDelay_(record.isDelay_)
{
    this->isConvertUriFromRemote = record.isConvertUriFromRemote;
    InitDecodeMap();
}

void PasteDataRecord::InitDecodeMap()
{
    decodeMap = {
        { TAG_MIMETYPE,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, mimeType_, head);
            } },
        { TAG_HTMLTEXT,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, htmlText_, head);
            } },
        { TAG_WANT,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                RawMem rawMem{};
                ret = ret && ReadValue(buffer, rawMem, head);
                want_ = ParcelUtil::Raw2Parcelable<AAFwk::Want>(rawMem);
            } },
        { TAG_PLAINTEXT,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, plainText_, head);
            } },
        { TAG_URI,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                RawMem rawMem{};
                ret = ret && ReadValue(buffer, rawMem, head);
                uri_ = ParcelUtil::Raw2Parcelable<OHOS::Uri>(rawMem);
            } },
        { TAG_CONVERT_URI,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, convertUri_, head);
            } },
        { TAG_PIXELMAP,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                std::vector<std::uint8_t> value;
                ret = ret && ReadValue(buffer, value, head);
                pixelMap_ = Vector2PixelMap(value);
            } },
        { TAG_CUSTOM_DATA,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, customData_, head);
            } },
        { TAG_URI_PERMISSION,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, hasGrantUriPermission_, head);
            } },
        { TAG_UDC_UDTYPE,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, udType_, head);
            } },
        { TAG_UDC_DETAILS,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, details_, head);
            } },
        { TAG_UDC_TEXTCONTENT,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, textContent_, head);
            } },
        { TAG_UDC_SYSTEMCONTENTS,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, systemDefinedContents_, head);
            } },
        { TAG_UDC_UDMFVALUE,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, udmfValue_, head);
            } },
        { TAG_UDC_ENTYIES,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, entries_, head);
            } },
        { TAG_DATA_ID,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, dataId_, head);
            } },
        { TAG_RECORD_ID,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, recordId_, head);
            } },
        { TAG_DELAY_RECORD_FLAG,
            [&](bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head) -> void {
                ret = ret && ReadValue(buffer, isDelay_, head);
            } },
    };
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
        return GetOrginUri();
    }
    return std::make_shared<OHOS::Uri>(convertUri_);
}

void PasteDataRecord::ClearPixelMap()
{
    this->pixelMap_ = nullptr;
}

void PasteDataRecord::SetUri(std::shared_ptr<OHOS::Uri> uri)
{
    uri_ = std::move(uri);
    AddUriEntry();
}

std::shared_ptr<OHOS::Uri> PasteDataRecord::GetOrginUri() const
{
    if (uri_) {
        return uri_;
    }
    for (auto const &entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_TEXT_URI) {
            return entry->ConvertToUri();
        }
    }
    return nullptr;
}

std::shared_ptr<OHOS::AAFwk::Want> PasteDataRecord::GetWant() const
{
    return this->want_;
}

std::shared_ptr<MineCustomData> PasteDataRecord::GetCustomData() const
{
    return this->customData_;
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

bool PasteDataRecord::Encode(std::vector<std::uint8_t> &buffer)
{
    bool ret = Write(buffer, TAG_MIMETYPE, mimeType_);
    ret = Write(buffer, TAG_HTMLTEXT, htmlText_) && ret;
    ret = Write(buffer, TAG_WANT, ParcelUtil::Parcelable2Raw(want_.get())) && ret;
    ret = Write(buffer, TAG_PLAINTEXT, plainText_) && ret;
    ret = Write(buffer, TAG_URI, ParcelUtil::Parcelable2Raw(uri_.get())) && ret;
    ret = Write(buffer, TAG_CONVERT_URI, convertUri_) && ret;
    auto pixelVector = PixelMap2Vector(pixelMap_);
    ret = Write(buffer, TAG_PIXELMAP, pixelVector) && ret;
    ret = Write(buffer, TAG_CUSTOM_DATA, customData_) && ret;
    ret = Write(buffer, TAG_URI_PERMISSION, hasGrantUriPermission_) && ret;
    ret = Write(buffer, TAG_UDC_UDTYPE, udType_) && ret;
    ret = Write(buffer, TAG_UDC_DETAILS, details_) && ret;
    ret = Write(buffer, TAG_UDC_TEXTCONTENT, textContent_) && ret;
    ret = Write(buffer, TAG_UDC_SYSTEMCONTENTS, systemDefinedContents_) && ret;
    ret = Write(buffer, TAG_UDC_UDMFVALUE, udmfValue_) && ret;
    ret = Write(buffer, TAG_UDC_ENTYIES, entries_) && ret;
    ret = Write(buffer, TAG_DATA_ID, dataId_) && ret;
    ret = Write(buffer, TAG_RECORD_ID, recordId_) && ret;
    ret = Write(buffer, TAG_DELAY_RECORD_FLAG, isDelay_) && ret;
    return ret;
}

bool PasteDataRecord::Decode(const std::vector<std::uint8_t> &buffer)
{
    for (; IsEnough();) {
        TLVHead head{};
        bool ret = ReadHead(buffer, head);
        auto it = decodeMap.find(head.tag);
        if (it == decodeMap.end()) {
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

size_t PasteDataRecord::Count()
{
    size_t expectedSize = 0;
    expectedSize += TLVObject::Count(mimeType_);
    expectedSize += TLVObject::Count(htmlText_);
    expectedSize += TLVObject::Count(ParcelUtil::Parcelable2Raw(want_.get()));
    expectedSize += TLVObject::Count(plainText_);
    expectedSize += TLVObject::Count(ParcelUtil::Parcelable2Raw(uri_.get()));
    expectedSize += TLVObject::Count(convertUri_);
    auto pixelVector = PixelMap2Vector(pixelMap_);
    expectedSize += TLVObject::Count(pixelVector);
    expectedSize += TLVObject::Count(customData_);
    expectedSize += TLVObject::Count(hasGrantUriPermission_);
    expectedSize += TLVObject::Count(udType_);
    expectedSize += TLVObject::Count(details_);
    expectedSize += TLVObject::Count(textContent_);
    expectedSize += TLVObject::Count(systemDefinedContents_);
    expectedSize += TLVObject::Count(udmfValue_);
    expectedSize += TLVObject::Count(entries_);
    expectedSize += TLVObject::Count(dataId_);
    expectedSize += TLVObject::Count(recordId_);
    expectedSize += TLVObject::Count(isDelay_);
    return expectedSize;
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
    }
    if (!uriHandler.IsPaste()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Set fd, fd is %{public}d", fd);
        fd_->SetFd(fd);
    }
    return true;
}
bool PasteDataRecord::NeedFd(const UriHandler &uriHandler)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start");
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
    }
}
void PasteDataRecord::SetConvertUri(const std::string &value)
{
    convertUri_ = value;
}
std::string PasteDataRecord::GetConvertUri() const
{
    return convertUri_;
}
void PasteDataRecord::SetGrantUriPermission(bool hasPermission)
{
    hasGrantUriPermission_ = hasPermission;
}
bool PasteDataRecord::HasGrantUriPermission()
{
    return hasGrantUriPermission_;
}

void PasteDataRecord::SetTextContent(const std::string &content)
{
    this->textContent_ = content;
}

std::string PasteDataRecord::GetTextContent() const
{
    return this->textContent_;
}

void PasteDataRecord::SetDetails(const Details &details)
{
    this->details_ = std::make_shared<Details>(details);
}

std::shared_ptr<Details> PasteDataRecord::GetDetails() const
{
    return this->details_;
}

void PasteDataRecord::SetSystemDefinedContent(const Details &contents)
{
    this->systemDefinedContents_ = std::make_shared<Details>(contents);
}

std::shared_ptr<Details> PasteDataRecord::GetSystemDefinedContent() const
{
    return this->systemDefinedContents_;
}
int32_t PasteDataRecord::GetUDType() const
{
    return this->udType_;
}

void PasteDataRecord::SetUDType(int32_t type)
{
    this->udType_ = type;
}

std::vector<std::string> PasteDataRecord::GetValidTypes(const std::vector<std::string> &types) const
{
    std::vector<std::string> res;
    auto allTypes = GetUdtTypes();
    for (auto const &type : types) {
        if (allTypes.find(type) != allTypes.end()) {
            res.emplace_back(type);
        }
    }
    return res;
}

bool PasteDataRecord::IsEmpty() const
{
    if (udmfValue_ && !std::holds_alternative<std::monostate>(*udmfValue_)) {
        return false;
    }
    for (auto const &entry : entries_) {
        if (!std::holds_alternative<std::monostate>(entry->GetValue())) {
            return false;
        }
    }
    return true;
}

void PasteDataRecord::SetUDMFValue(const std::shared_ptr<EntryValue> &udmfValue)
{
    this->udmfValue_ = udmfValue;
}

std::shared_ptr<EntryValue> PasteDataRecord::GetUDMFValue()
{
    if (udmfValue_) {
        return this->udmfValue_;
    }
    if (mimeType_.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "mimetype is null");
        return nullptr;
    }
    auto object = std::make_shared<Object>();
    if (mimeType_ == MIMETYPE_TEXT_PLAIN) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::PLAIN_TEXT);
        if (plainText_ != nullptr) {
            object->value_[UDMF::CONTENT] = *plainText_;
        }
    } else if (mimeType_ == MIMETYPE_TEXT_HTML) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML);
        if (htmlText_ != nullptr) {
            object->value_[UDMF::HTML_CONTENT] = *htmlText_;
        }
    } else if (mimeType_ == MIMETYPE_PIXELMAP) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP);
        if (pixelMap_ != nullptr) {
            object->value_[UDMF::PIXEL_MAP] = pixelMap_;
        }
    } else if (mimeType_ == MIMETYPE_TEXT_URI) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI);
        if (uri_ != nullptr) {
            object->value_[UDMF::FILE_URI_PARAM] = uri_->ToString();
        }
    } else if (mimeType_ == MIMETYPE_TEXT_WANT) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "mimeType is want,udmf not surpport");
    } else {
        auto itemData = customData_->GetItemData();
        if (itemData.size() == 0) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "no customData");
            return udmfValue_;
        }
        if (itemData.size() != 1) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT,
                "not surrport u8 map, mimeType:%{public}s, customData.size:%{public}zu", mimeType_.c_str(),
                itemData.size());
        }
        udmfValue_ = std::make_shared<EntryValue>(itemData.begin()->second);
        return udmfValue_;
    }
    udmfValue_ = std::make_shared<EntryValue>(object);
    return udmfValue_;
}

std::set<std::string> PasteDataRecord::GetUdtTypes() const
{
    std::set<std::string> types;
    types.emplace(CommonUtils::Convert2UtdId(udType_, mimeType_));
    for (auto const &entry : entries_) {
        types.emplace(entry->GetUtdId());
    }
    return types;
}

void PasteDataRecord::AddEntry(const std::string &utdType, std::shared_ptr<PasteDataEntry> value)
{
    if (!value) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "entry is null");
        return;
    }
    if (utdType != value->GetUtdId()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type is diff. utdtype:%{public}s, UtdId:%{public}s",
            utdType.c_str(), value->GetUtdId().c_str());
        return;
    }
    // first entry save to record, or record type same
    if (mimeType_.empty() || utdType == CommonUtils::Convert2UtdId(udType_, mimeType_)) {
        mimeType_ = value->GetMimeType();
        auto udType = UDMF::UtdUtils::GetUtdEnumFromUtdId(utdType);
        udType_ = udType == UDMF::UDType::UD_BUTT ? UDMF::UDType::APPLICATION_DEFINED_RECORD : udType;
        udmfValue_ = std::make_shared<EntryValue>(value->GetValue());
        if (mimeType_ == MIMETYPE_PIXELMAP) {
            pixelMap_ = value->ConvertToPixelMap();
        } else if (mimeType_ == MIMETYPE_TEXT_HTML) {
            htmlText_ = value->ConvertToHtml();
        } else if (mimeType_ == MIMETYPE_TEXT_PLAIN) {
            plainText_ = value->ConvertToPlianText();
        } else if (mimeType_ == MIMETYPE_TEXT_URI) {
            uri_ = value->ConvertToUri();
        } else if (mimeType_ == MIMETYPE_TEXT_WANT) {
            want_ = value->ConvertToWant();
        } else {
            customData_ = value->ConvertToCustomData();
        }
        return;
    }
    // not firest entry
    bool has = false;
    for (auto &entry : entries_) {
        if (entry->GetUtdId() == utdType) {
            entry = value;
            has = true;
            break;
        }
    }
    if (!has) {
        entries_.emplace_back(value);
    }
}

std::shared_ptr<PasteDataEntry> PasteDataRecord::GetEntry(const std::string &utdType) const
{
    if (udmfValue_ && CommonUtils::Convert2UtdId(udType_, mimeType_) == utdType) {
        return std::make_shared<PasteDataEntry>(utdType, *udmfValue_);
    }
    for (auto const &entry : entries_) {
        if (entry->GetUtdId() == utdType) {
            return entry;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<PasteDataEntry>> PasteDataRecord::GetEntries() const
{
    std::vector<std::shared_ptr<PasteDataEntry>> entries = entries_;
    if (udmfValue_) {
        entries.insert(entries.begin(),
            std::make_shared<PasteDataEntry>(CommonUtils::Convert2UtdId(udType_, mimeType_), *udmfValue_));
    }
    return entries;
}

void PasteDataRecord::SetDataId(uint32_t dataId)
{
    dataId_ = dataId;
}

uint32_t PasteDataRecord::GetDataId() const
{
    return dataId_;
}

void PasteDataRecord::SetRecordId(uint32_t recordId)
{
    recordId_ = recordId;
}

uint32_t PasteDataRecord::GetRecordId() const
{
    return recordId_;
}

void PasteDataRecord::SetDelayRecordFlag(bool isDelay)
{
    isDelay_ = isDelay;
}

bool PasteDataRecord::IsDelayRecord() const
{
    return isDelay_;
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