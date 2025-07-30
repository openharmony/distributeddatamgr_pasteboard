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

#include "pasteboard_common.h"
#include "pasteboard_hilog.h"
#include "pasteboard_service_loader.h"

using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
constexpr int MAX_TEXT_LEN = 100 * 1024 * 1024;
constexpr uid_t ANCO_SERVICE_BROKER_UID = 5557;

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
    TAG_UDC_ENTRIES,
    TAG_DATA_ID,
    TAG_RECORD_ID,
    TAG_DELAY_RECORD_FLAG,
    TAG_FROM,
};

PasteDataRecord::Builder &PasteDataRecord::Builder::SetHtmlText(std::shared_ptr<std::string> htmlText)
{
    if (htmlText == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(*htmlText);
    record_->AddEntryByMimeType(MIMETYPE_TEXT_HTML, entry);
    return *this;
}

PasteDataRecord::Builder &PasteDataRecord::Builder::SetWant(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    if (want == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(std::move(want));
    record_->AddEntryByMimeType(MIMETYPE_TEXT_WANT, entry);
    return *this;
}

PasteDataRecord::Builder &PasteDataRecord::Builder::SetPlainText(std::shared_ptr<std::string> plainText)
{
    if (plainText == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(*plainText);
    record_->AddEntryByMimeType(MIMETYPE_TEXT_PLAIN, entry);
    return *this;
}
PasteDataRecord::Builder &PasteDataRecord::Builder::SetUri(std::shared_ptr<OHOS::Uri> uri)
{
    if (uri == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(uri->ToString());
    record_->AddEntryByMimeType(MIMETYPE_TEXT_URI, entry);
    return *this;
}

PasteDataRecord::Builder &PasteDataRecord::Builder::SetPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap)
{
    if (pixelMap == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(std::move(pixelMap));
    record_->AddEntryByMimeType(MIMETYPE_PIXELMAP, entry);
    return *this;
}

PasteDataRecord::Builder &PasteDataRecord::Builder::SetCustomData(std::shared_ptr<MineCustomData> customData)
{
    record_->customData_ = std::move(customData);
    return *this;
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::Builder::Build()
{
    if (record_->mimeType_.empty()) {
        return record_;
    }
    auto entries = record_->GetEntries();
    auto record = std::make_shared<PasteDataRecord>();
    for (size_t i = 0; i < entries.size(); ++i) {
        if (record_->mimeType_ == entries[i]->GetMimeType()) {
            record->AddEntry(entries[i]->GetUtdId(), entries[i]);
        }
    }
    for (size_t i = 0; i < entries.size(); ++i) {
        if (record_->mimeType_ != entries[i]->GetMimeType()) {
            record->AddEntry(entries[i]->GetUtdId(), entries[i]);
        }
    }
    record->customData_ = std::move(record_->customData_);
    record->mimeType_ = record_->mimeType_;
    return record;
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
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE((htmlText.length() < MAX_TEXT_LEN), nullptr,
        PASTEBOARD_MODULE_CLIENT, "record length not support, length=%{public}zu", htmlText.length());
    return Builder(MIMETYPE_TEXT_HTML).SetHtmlText(std::make_shared<std::string>(htmlText)).Build();
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    return Builder(MIMETYPE_TEXT_WANT).SetWant(std::move(want)).Build();
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewPlainTextRecord(const std::string &text)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE((text.length() < MAX_TEXT_LEN), nullptr,
        PASTEBOARD_MODULE_CLIENT, "PlainText length not support, length=%{public}zu", text.length());
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

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewMultiTypeRecord(
    std::shared_ptr<std::map<std::string, std::shared_ptr<EntryValue>>> values, const std::string &recordMimeType)
{
    auto record = std::make_shared<PasteDataRecord>();
    if (values == nullptr) {
        return record;
    }
    if (!recordMimeType.empty()) {
        auto recordDefaultIter = values->find(recordMimeType);
        if (recordDefaultIter != values->end() && recordDefaultIter->second != nullptr) {
            auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, recordMimeType);
            record->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, *(recordDefaultIter->second)));
        }
    }
    for (auto [mimeType, value] : *values) {
        if (mimeType == recordMimeType) {
            continue;
        }
        auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, mimeType);
        record->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, *value));
    }
    return record;
}

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewMultiTypeDelayRecord(
    std::vector<std::string> mimeTypes, const std::shared_ptr<UDMF::EntryGetter> entryGetter)
{
    auto record = std::make_shared<PasteDataRecord>();
    for (auto mimeType : mimeTypes) {
        auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, mimeType);
        auto entry = std::make_shared<PasteDataEntry>();
        entry->SetMimeType(mimeType);
        entry->SetUtdId(utdId);
        record->AddEntry(utdId, entry);
    }
    if (entryGetter != nullptr) {
        record->SetEntryGetter(entryGetter);
        record->SetDelayRecordFlag(true);
    }
    return record;
}

PasteDataRecord::PasteDataRecord(std::string mimeType, std::shared_ptr<std::string> htmlText,
    std::shared_ptr<OHOS::AAFwk::Want> want, std::shared_ptr<std::string> plainText, std::shared_ptr<OHOS::Uri> uri)
    : mimeType_{ std::move(mimeType) }, htmlText_{ std::move(htmlText) }, want_{ std::move(want) },
      plainText_{ std::move(plainText) }, uri_{ std::move(uri) }
{
}

PasteDataRecord::PasteDataRecord()
{
}

PasteDataRecord::~PasteDataRecord()
{
    std::vector<std::shared_ptr<PasteDataEntry>>().swap(entries_);
}

PasteDataRecord::PasteDataRecord(const PasteDataRecord &record)
    : isDelay_(record.isDelay_), hasGrantUriPermission_(record.hasGrantUriPermission_), udType_(record.udType_),
      dataId_(record.dataId_), recordId_(record.recordId_), from_(record.from_), convertUri_(record.convertUri_),
      textContent_(record.textContent_), mimeType_(record.mimeType_), htmlText_(record.htmlText_),
      want_(record.want_), plainText_(record.plainText_), uri_(record.uri_), pixelMap_(record.pixelMap_),
      customData_(record.customData_), details_(record.details_), systemDefinedContents_(record.systemDefinedContents_),
      udmfValue_(record.udmfValue_), entries_(record.entries_), entryGetter_(record.entryGetter_)
{
    this->isConvertUriFromRemote = record.isConvertUriFromRemote;
}

std::shared_ptr<std::string> PasteDataRecord::GetHtmlTextV0() const
{
    for (const auto &entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_TEXT_HTML) {
            return entry->ConvertToHtml();
        }
    }
    return htmlText_;
}

std::shared_ptr<std::string> PasteDataRecord::GetHtmlText()
{
    auto htmlText = GetHtmlTextV0();
    if (htmlText) {
        return htmlText;
    }
    auto entry = GetEntryByMimeType(MIMETYPE_TEXT_HTML);
    if (entry == nullptr) {
        return htmlText_;
    }
    return entry->ConvertToHtml();
}

std::string PasteDataRecord::GetMimeType() const
{
    if (!mimeType_.empty()) {
        return mimeType_;
    }
    if (!entries_.empty()) {
        return entries_.front()->GetMimeType();
    }
    return this->mimeType_;
}

std::shared_ptr<std::string> PasteDataRecord::GetPlainTextV0() const
{
    for (const auto &entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_TEXT_PLAIN) {
            return entry->ConvertToPlainText();
        }
    }
    return plainText_;
}

std::shared_ptr<std::string> PasteDataRecord::GetPlainText()
{
    auto plainText = GetPlainTextV0();
    if (plainText) {
        return plainText;
    }
    auto entry = GetEntryByMimeType(MIMETYPE_TEXT_PLAIN);
    if (entry == nullptr) {
        return plainText_;
    }
    return entry->ConvertToPlainText();
}

std::shared_ptr<PixelMap> PasteDataRecord::GetPixelMapV0() const
{
    for (const auto &entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_PIXELMAP) {
            return entry->ConvertToPixelMap();
        }
    }
    return pixelMap_;
}

std::shared_ptr<PixelMap> PasteDataRecord::GetPixelMap()
{
    auto pixelMap = GetPixelMapV0();
    if (pixelMap) {
        return pixelMap;
    }
    auto entry = GetEntryByMimeType(MIMETYPE_PIXELMAP);
    if (entry == nullptr) {
        return pixelMap_;
    }
    return entry->ConvertToPixelMap();
}

std::shared_ptr<OHOS::Uri> PasteDataRecord::GetUriV0() const
{
    if (convertUri_.empty()) {
        return GetOriginUri();
    }
    return std::make_shared<OHOS::Uri>(convertUri_);
}

std::shared_ptr<OHOS::Uri> PasteDataRecord::GetUri()
{
    auto uri = GetUriV0();
    if (uri) {
        return uri;
    }
    auto entry = GetEntryByMimeType(MIMETYPE_TEXT_URI);
    if (entry == nullptr) {
        return GetUriV0();
    }
    return entry->ConvertToUri();
}

void PasteDataRecord::ClearPixelMap()
{
    this->pixelMap_ = nullptr;
    entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
        [](const auto &entry) {
            return entry != nullptr && entry->GetMimeType() == MIMETYPE_PIXELMAP;
        }), entries_.end());
}

void PasteDataRecord::SetUri(std::shared_ptr<OHOS::Uri> uri)
{
    if (uri == nullptr) {
        return;
    }

    for (auto &entry : entries_) {
        if (entry != nullptr && entry->GetMimeType() == MIMETYPE_TEXT_URI) {
            auto entryValue = entry->GetValue();
            if (std::holds_alternative<std::shared_ptr<Object>>(entryValue)) {
                auto object = std::get<std::shared_ptr<Object>>(entryValue);
                object->value_[UDMF::FILE_URI_PARAM] = uri->ToString();
            } else {
                entry->SetValue(uri->ToString());
            }
            return;
        }
    }

    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(uri->ToString());
    AddEntryByMimeType(MIMETYPE_TEXT_URI, entry);
}

std::shared_ptr<OHOS::Uri> PasteDataRecord::GetOriginUri() const
{
    for (const auto &entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_TEXT_URI) {
            return entry->ConvertToUri();
        }
    }
    return uri_;
}

std::shared_ptr<OHOS::AAFwk::Want> PasteDataRecord::GetWant() const
{
    for (const auto &entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_TEXT_WANT) {
            return entry->ConvertToWant();
        }
    }
    return want_;
}

std::shared_ptr<MineCustomData> PasteDataRecord::GetCustomData() const
{
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    if (customData_) {
        const std::map<std::string, std::vector<uint8_t>> &itemData = customData_->GetItemData();
        for (const auto &[key, value] : itemData) {
            customData->AddItemData(key, value);
        }
    }
    for (const auto &entry : entries_) {
        if (entry && entry->GetMimeType() == entry->GetUtdId()) {
            std::shared_ptr<MineCustomData> entryCustomData = entry->ConvertToCustomData();
            if (entryCustomData == nullptr) {
                continue;
            }
            const std::map<std::string, std::vector<uint8_t>> &itemData = entryCustomData->GetItemData();
            for (const auto &[key, value] : itemData) {
                customData->AddItemData(key, value);
            }
        }
    }
    return customData->GetItemData().empty() ? nullptr : customData;
}

std::string PasteDataRecord::ConvertToText() const
{
    auto htmlText = GetHtmlTextV0();
    if (htmlText != nullptr) {
        return *htmlText;
    }
    auto plainText = GetPlainTextV0();
    if (plainText != nullptr) {
        return *plainText;
    }
    auto originUri = GetOriginUri();
    if (originUri != nullptr) {
        return originUri->ToString();
    }
    return "";
}

bool PasteDataRecord::EncodeTLVLocal(WriteOnlyBuffer &buffer) const
{
    bool ret = buffer.Write(TAG_MIMETYPE, mimeType_);
    ret = ret && buffer.Write(TAG_HTMLTEXT, htmlText_);
    ret = ret && buffer.Write(TAG_WANT, TLVUtils::Parcelable2Raw(want_.get()));
    ret = ret && buffer.Write(TAG_PLAINTEXT, plainText_);
    ret = ret && buffer.Write(TAG_URI, TLVUtils::Parcelable2Raw(uri_.get()));
    ret = ret && buffer.Write(TAG_CONVERT_URI, convertUri_);
    ret = ret && buffer.Write(TAG_PIXELMAP, pixelMap_);
    ret = ret && buffer.Write(TAG_CUSTOM_DATA, customData_);
    ret = ret && buffer.Write(TAG_URI_PERMISSION, hasGrantUriPermission_);
    ret = ret && buffer.Write(TAG_UDC_UDTYPE, udType_);
    ret = ret && buffer.Write(TAG_UDC_DETAILS, details_);
    ret = ret && buffer.Write(TAG_UDC_TEXTCONTENT, textContent_);
    ret = ret && buffer.Write(TAG_UDC_SYSTEMCONTENTS, systemDefinedContents_);
    ret = ret && buffer.Write(TAG_UDC_UDMFVALUE, udmfValue_);
    ret = ret && buffer.Write(TAG_UDC_ENTRIES, entries_);
    ret = ret && buffer.Write(TAG_DATA_ID, dataId_);
    ret = ret && buffer.Write(TAG_RECORD_ID, recordId_);
    ret = ret && buffer.Write(TAG_DELAY_RECORD_FLAG, isDelay_);
    ret = ret && buffer.Write(TAG_FROM, from_);
    return ret;
}

bool PasteDataRecord::EncodeTLVRemote(WriteOnlyBuffer &buffer) const
{
    bool ret = true;

    auto remoteValue = Local2Remote();
    if (remoteValue != nullptr) {
        ret = ret && buffer.Write(TAG_MIMETYPE, remoteValue->mimeType_);
        ret = ret && buffer.Write(TAG_UDC_UDTYPE, remoteValue->udType_);
        ret = ret && buffer.Write(TAG_HTMLTEXT, remoteValue->htmlText_);
        ret = ret && buffer.Write(TAG_PLAINTEXT, remoteValue->plainText_);
        ret = ret && buffer.Write(TAG_PIXELMAP, remoteValue->pixelMap_);
        ret = ret && buffer.Write(TAG_WANT, TLVUtils::Parcelable2Raw(remoteValue->want_.get()));
        ret = ret && buffer.Write(TAG_URI, TLVUtils::Parcelable2Raw(remoteValue->uri_.get()));
        ret = ret && buffer.Write(TAG_UDC_UDMFVALUE, remoteValue->udmfValue_);
        ret = ret && buffer.Write(TAG_UDC_ENTRIES, remoteValue->entries_);
    }

    ret = ret && buffer.Write(TAG_CONVERT_URI, convertUri_);
    ret = ret && buffer.Write(TAG_CUSTOM_DATA, customData_);
    ret = ret && buffer.Write(TAG_URI_PERMISSION, hasGrantUriPermission_);
    ret = ret && buffer.Write(TAG_UDC_DETAILS, details_);
    ret = ret && buffer.Write(TAG_UDC_TEXTCONTENT, textContent_);
    ret = ret && buffer.Write(TAG_UDC_SYSTEMCONTENTS, systemDefinedContents_);
    ret = ret && buffer.Write(TAG_DATA_ID, dataId_);
    ret = ret && buffer.Write(TAG_RECORD_ID, recordId_);
    ret = ret && buffer.Write(TAG_DELAY_RECORD_FLAG, isDelay_);
    ret = ret && buffer.Write(TAG_FROM, from_);
    return ret;
}

bool PasteDataRecord::EncodeTLV(WriteOnlyBuffer &buffer) const
{
    return IsRemoteEncode() ? EncodeTLVRemote(buffer) : EncodeTLVLocal(buffer);
}

bool PasteDataRecord::DecodeItem1(uint16_t tag, ReadOnlyBuffer &buffer, TLVHead &head)
{
    switch (tag) {
        case TAG_MIMETYPE:
            return buffer.ReadValue(mimeType_, head);
        case TAG_HTMLTEXT:
            return buffer.ReadValue(htmlText_, head);
        case TAG_WANT:
            return buffer.ReadValue(want_, head);
        case TAG_PLAINTEXT:
            return buffer.ReadValue(plainText_, head);
        case TAG_URI:
            return buffer.ReadValue(uri_, head);
        case TAG_CONVERT_URI:
            return buffer.ReadValue(convertUri_, head);
        case TAG_PIXELMAP:
            return buffer.ReadValue(pixelMap_, head);
        case TAG_CUSTOM_DATA:
            return buffer.ReadValue(customData_, head);
        case TAG_URI_PERMISSION:
            return buffer.ReadValue(hasGrantUriPermission_, head);
        default:
            return DecodeItem2(tag, buffer, head);
    }
}

bool PasteDataRecord::DecodeItem2(uint16_t tag, ReadOnlyBuffer &buffer, TLVHead &head)
{
    switch (tag) {
        case TAG_UDC_UDTYPE:
            return buffer.ReadValue(udType_, head);
        case TAG_UDC_DETAILS:
            return buffer.ReadValue(details_, head);
        case TAG_UDC_TEXTCONTENT:
            return buffer.ReadValue(textContent_, head);
        case TAG_UDC_SYSTEMCONTENTS:
            return buffer.ReadValue(systemDefinedContents_, head);
        case TAG_UDC_UDMFVALUE:
            return buffer.ReadValue(udmfValue_, head);
        case TAG_UDC_ENTRIES:
            return buffer.ReadValue(entries_, head);
        case TAG_DATA_ID:
            return buffer.ReadValue(dataId_, head);
        case TAG_RECORD_ID:
            return buffer.ReadValue(recordId_, head);
        case TAG_DELAY_RECORD_FLAG:
            return buffer.ReadValue(isDelay_, head);
        case TAG_FROM:
            return buffer.ReadValue(from_, head);
        default:
            return buffer.Skip(head.len);
    }
}

bool PasteDataRecord::DecodeTLV(ReadOnlyBuffer &buffer)
{
    for (; buffer.IsEnough();) {
        TLVHead head{};
        bool ret = buffer.ReadHead(head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "read head failed");
        ret = DecodeItem1(head.tag, buffer, head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON,
            "read value failed, tag=%{public}hu, len=%{public}u", head.tag, head.len);
    }

    auto entry = Remote2Local();
    if (entry != nullptr) {
        entries_.insert(entries_.begin(), std::move(entry));
    }
    udmfValue_ = nullptr;
    plainText_ = (getuid() == ANCO_SERVICE_BROKER_UID) ? GetPlainTextV0() : nullptr;
    htmlText_ = nullptr;
    pixelMap_ = nullptr;
    want_ = nullptr;
    uri_ = nullptr;
    return true;
}

size_t PasteDataRecord::CountTLVLocal() const
{
    size_t expectedSize = 0;
    expectedSize += TLVCountable::Count(mimeType_);
    expectedSize += TLVCountable::Count(htmlText_);
    expectedSize += TLVCountable::Count(TLVUtils::Parcelable2Raw(want_.get()));
    expectedSize += TLVCountable::Count(plainText_);
    expectedSize += TLVCountable::Count(TLVUtils::Parcelable2Raw(uri_.get()));
    expectedSize += TLVCountable::Count(convertUri_);
    expectedSize += TLVCountable::Count(pixelMap_);
    expectedSize += TLVCountable::Count(customData_);
    expectedSize += TLVCountable::Count(hasGrantUriPermission_);
    expectedSize += TLVCountable::Count(udType_);
    expectedSize += TLVCountable::Count(details_);
    expectedSize += TLVCountable::Count(textContent_);
    expectedSize += TLVCountable::Count(systemDefinedContents_);
    expectedSize += TLVCountable::Count(udmfValue_);
    expectedSize += TLVCountable::Count(entries_);
    expectedSize += TLVCountable::Count(dataId_);
    expectedSize += TLVCountable::Count(recordId_);
    expectedSize += TLVCountable::Count(isDelay_);
    expectedSize += TLVCountable::Count(from_);
    return expectedSize;
}

size_t PasteDataRecord::CountTLVRemote() const
{
    size_t expectedSize = 0;
    auto remoteValue = Local2Remote();
    if (remoteValue != nullptr) {
        expectedSize += TLVCountable::Count(remoteValue->mimeType_);
        expectedSize += TLVCountable::Count(remoteValue->udType_);
        expectedSize += TLVCountable::Count(remoteValue->htmlText_);
        expectedSize += TLVCountable::Count(remoteValue->plainText_);
        expectedSize += TLVCountable::Count(remoteValue->pixelMap_);
        expectedSize += TLVCountable::Count(TLVUtils::Parcelable2Raw(remoteValue->want_.get()));
        expectedSize += TLVCountable::Count(TLVUtils::Parcelable2Raw(remoteValue->uri_.get()));
        expectedSize += TLVCountable::Count(remoteValue->udmfValue_);
        expectedSize += TLVCountable::Count(remoteValue->entries_);
    }

    expectedSize += TLVCountable::Count(convertUri_);
    expectedSize += TLVCountable::Count(customData_);
    expectedSize += TLVCountable::Count(hasGrantUriPermission_);
    expectedSize += TLVCountable::Count(details_);
    expectedSize += TLVCountable::Count(textContent_);
    expectedSize += TLVCountable::Count(systemDefinedContents_);
    expectedSize += TLVCountable::Count(dataId_);
    expectedSize += TLVCountable::Count(recordId_);
    expectedSize += TLVCountable::Count(isDelay_);
    expectedSize += TLVCountable::Count(from_);
    return expectedSize;
}

size_t PasteDataRecord::CountTLV() const
{
    return IsRemoteEncode() ? CountTLVRemote() : CountTLVLocal();
}

std::shared_ptr<PasteDataEntry> PasteDataRecord::Remote2Local() const
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGD(!mimeType_.empty(), nullptr, PASTEBOARD_MODULE_COMMON, "mimeType empty");

    auto entry = std::make_shared<PasteDataEntry>();
    auto utdId = CommonUtils::Convert2UtdId(udType_, mimeType_);
    entry->SetUtdId(utdId);
    entry->SetMimeType(mimeType_);

    if (udmfValue_ != nullptr) {
        if (std::holds_alternative<std::shared_ptr<Object>>(*udmfValue_)) {
            auto object = std::get<std::shared_ptr<Object>>(*udmfValue_);
            if (object != nullptr && !object->value_.empty()) {
                entry->SetValue(object);
                return entry;
            }
        } else if (std::holds_alternative<std::vector<uint8_t>>(*udmfValue_)) {
            auto array = std::get<std::vector<uint8_t>>(*udmfValue_);
            entry->SetValue(array);
            return entry;
        }
    }

    auto object = std::make_shared<Object>();
    if (mimeType_ == MIMETYPE_TEXT_PLAIN && plainText_ != nullptr) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::PLAIN_TEXT);
        object->value_[UDMF::CONTENT] = *plainText_;
    } else if (mimeType_ == MIMETYPE_TEXT_HTML && htmlText_ != nullptr) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML);
        object->value_[UDMF::HTML_CONTENT] = *htmlText_;
        if (plainText_ != nullptr) {
            object->value_[UDMF::PLAIN_CONTENT] = *plainText_;
        }
    } else if (mimeType_ == MIMETYPE_TEXT_URI && uri_ != nullptr) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI);
        object->value_[UDMF::FILE_URI_PARAM] = uri_->ToString();
    } else if (mimeType_ == MIMETYPE_PIXELMAP && pixelMap_ != nullptr) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP);
        object->value_[UDMF::PIXEL_MAP] = std::move(pixelMap_);
    } else if (mimeType_ == MIMETYPE_TEXT_WANT && want_ != nullptr) {
        entry->SetValue(std::move(want_));
        return entry;
    } else {
        return nullptr;
    }

    entry->SetValue(object);
    return entry;
}

std::shared_ptr<RemoteRecordValue> PasteDataRecord::Local2Remote() const
{
    auto value = std::make_shared<RemoteRecordValue>();
    value->mimeType_ = mimeType_;
    if (entries_.empty()) {
        return value;
    }

    auto firstEntry = entries_.front();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(firstEntry != nullptr, value, PASTEBOARD_MODULE_COMMON, "firstEntry is null");

    auto entryValue = firstEntry->GetValue();
    std::string mimeType = firstEntry->GetMimeType();
    std::string utdId = firstEntry->GetUtdId();
    value->udType_ = UDMF::UtdUtils::GetUtdEnumFromUtdId(utdId);
    value->mimeType_ = mimeType;

    if (mimeType == MIMETYPE_TEXT_PLAIN) {
        value->plainText_ = firstEntry->ConvertToPlainText();
    } else if (mimeType == MIMETYPE_TEXT_WANT) {
        value->want_ = firstEntry->ConvertToWant();
    } else if (mimeType == MIMETYPE_TEXT_URI) {
        value->uri_ = firstEntry->ConvertToUri();
    } else if (mimeType == MIMETYPE_PIXELMAP) {
        value->pixelMap_ = firstEntry->ConvertToPixelMap();
    } else if (mimeType == MIMETYPE_TEXT_HTML) {
        value->htmlText_ = firstEntry->ConvertToHtml();
        if (std::holds_alternative<std::shared_ptr<Object>>(entryValue)) {
            auto object = std::get<std::shared_ptr<Object>>(entryValue);
            std::string plainText;
            object->GetValue(UDMF::PLAIN_CONTENT, plainText);
            if (!plainText.empty()) {
                value->plainText_ = std::make_shared<std::string>(plainText);
            }
        }
    }

    value->udmfValue_ = entryValue;
    value->entries_.assign(entries_.begin() + 1, entries_.end());
    return value;
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

std::vector<std::string> PasteDataRecord::GetValidMimeTypes(const std::vector<std::string> &mimeTypes) const
{
    std::vector<std::string> res;
    auto allTypes = GetMimeTypes();
    for (auto const& type : mimeTypes) {
        if (allTypes.find(type) != allTypes.end()) {
            res.emplace_back(type);
        }
    }
    return res;
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

bool PasteDataRecord::HasEmptyEntry() const
{
    for (auto const &entry : GetEntries()) {
        if (std::holds_alternative<std::monostate>(entry->GetValue())) {
            return true;
        }
    }
    return false;
}

std::set<std::string> PasteDataRecord::GetUdtTypes() const
{
    std::set<std::string> types;
    if (!mimeType_.empty()) {
        types.emplace(CommonUtils::Convert2UtdId(udType_, mimeType_));
    }
    for (auto const &entry : entries_) {
        types.emplace(entry->GetUtdId());
    }
    return types;
}

std::set<std::string> PasteDataRecord::GetMimeTypes() const
{
    std::set<std::string> types;
    if (!mimeType_.empty()) {
        types.emplace(mimeType_);
    }
    for (auto const& entry: entries_) {
        types.emplace(entry->GetMimeType());
    }
    return types;
}

void PasteDataRecord::AddEntryByMimeType(const std::string &mimeType, std::shared_ptr<PasteDataEntry> value)
{
    PASTEBOARD_CHECK_AND_RETURN_LOGE(value != nullptr, PASTEBOARD_MODULE_CLIENT, "value is null");
    auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, mimeType);
    value->SetUtdId(utdId);
    value->SetMimeType(mimeType);
    AddEntry(utdId, value);
}

void PasteDataRecord::AddEntry(const std::string &utdType, std::shared_ptr<PasteDataEntry> value)
{
    PASTEBOARD_CHECK_AND_RETURN_LOGE(value != nullptr, PASTEBOARD_MODULE_CLIENT, "Entry value is null");
    if (utdType != value->GetUtdId()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Type is diff, UtdType:%{public}s, UtdId:%{public}s",
            utdType.c_str(), value->GetUtdId().c_str());
        return;
    }

    bool has = false;
    for (auto &entry : entries_) {
        if (entry->GetUtdId() == utdType ||
            (entry->GetMimeType() == MIMETYPE_TEXT_URI && value->GetMimeType() == MIMETYPE_TEXT_URI)) {
            entry = value;
            has = true;
            break;
        }
    }

    PASTEBOARD_CHECK_AND_RETURN_LOGD(!has, PASTEBOARD_MODULE_COMMON, "replace entry, type=%{public}s", utdType.c_str());
    if (entries_.empty()) {
        auto udType = UDMF::UtdUtils::GetUtdEnumFromUtdId(utdType);
        udType_ = udType == UDMF::UDType::UD_BUTT ? UDMF::UDType::APPLICATION_DEFINED_RECORD : udType;
    }
    entries_.emplace_back(value);
}

std::shared_ptr<PasteDataEntry> PasteDataRecord::GetEntryByMimeType(const std::string &mimeType)
{
    auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, mimeType);
    std::shared_ptr<PasteDataEntry> entry = GetEntry(utdId);
    if (entry == nullptr && customData_ != nullptr) {
        const std::map<std::string, std::vector<uint8_t>> &itemData = customData_->GetItemData();
        for (const auto &[key, value] : itemData) {
            if (mimeType == key) {
                entry = std::make_shared<PasteDataEntry>(utdId, mimeType, value);
                return entry;
            }
        }
    }
    if (entry == nullptr && mimeType == MIMETYPE_TEXT_PLAIN) {
        utdId = CommonUtils::Convert2UtdId(UDMF::UDType::HYPERLINK, mimeType);
        entry = GetEntry(utdId);
    }
    return entry;
}

std::shared_ptr<PasteDataEntry> PasteDataRecord::GetEntry(const std::string &utdType)
{
    for (auto const &entry : entries_) {
        if (entry->GetUtdId() == utdType ||
            (CommonUtils::IsFileUri(utdType) && CommonUtils::IsFileUri(entry->GetUtdId()))) {
            if (isDelay_ && !entry->HasContent(utdType) && !PasteBoardCommon::IsPasteboardService()) {
                PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "get delay entry value, dataId=%{public}u, "
                    "recordId=%{public}u, type=%{public}s", dataId_, recordId_, utdType.c_str());
                PasteboardServiceLoader::GetInstance().GetRecordValueByType(dataId_, recordId_, *entry);
            }
            if (CommonUtils::IsFileUri(utdType) && GetUriV0() != nullptr) {
                return std::make_shared<PasteDataEntry>(utdType, GetUriV0()->ToString());
            }
            return entry;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<PasteDataEntry>> PasteDataRecord::GetEntries() const
{
    return entries_;
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

void PasteDataRecord::SetEntryGetter(const std::shared_ptr<UDMF::EntryGetter> entryGetter)
{
    entryGetter_ = std::move(entryGetter);
}

void PasteDataRecord::SetFrom(uint32_t from)
{
    from_ = from;
}

uint32_t PasteDataRecord::GetFrom() const
{
    return from_;
}

std::shared_ptr<UDMF::EntryGetter> PasteDataRecord::GetEntryGetter()
{
    return entryGetter_;
}
} // namespace MiscServices
} // namespace OHOS
