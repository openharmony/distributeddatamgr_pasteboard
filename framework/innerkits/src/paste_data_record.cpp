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
#include "pasteboard_common.h"

using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
namespace {
constexpr int MAX_TEXT_LEN = 500 * 1024;
}

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

PasteDataRecord::PasteDataRecord(std::string mimeType,
                                 std::shared_ptr<std::string> htmlText,
                                 std::shared_ptr<OHOS::AAFwk::Want> want,
                                 std::shared_ptr<std::string> plainText,
                                 std::shared_ptr<OHOS::Uri> uri)
    : mimeType_ {std::move(mimeType)},
      htmlText_ {std::move(htmlText)},
      want_ {std::move(want)},
      plainText_ {std::move(plainText)},
      uri_ {std::move(uri)} {}

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
    return this->uri_;
}

std::shared_ptr<OHOS::AAFwk::Want> PasteDataRecord::GetWant() const
{
    return this->want_;
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

bool PasteDataRecord::MarshallingString(Parcel &parcel, std::shared_ptr<std::string> item, uint32_t symbol)
{
    if (!parcel.WriteBool(item != nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteBool failed.");
        return false;
    }
    if (item == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "no need Marshalling, symbol = %{public}d.", symbol);
        return true;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "MarshallingString, symbol = %{public}d.", symbol);
    return parcel.WriteString(*item);
}

bool PasteDataRecord::MarshallingParcelable(Parcel &parcel, std::shared_ptr<Parcelable> item, uint32_t symbol)
{
    if (!parcel.WriteBool(item != nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteBool failed.");
        return false;
    }
    if (item == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "no need Marshalling, symbol = %{public}d.", symbol);
        return true;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "MarshallingParcelable, symbol = %{public}d.", symbol);
    return parcel.WriteParcelable(item.get());
}

bool PasteDataRecord::Marshalling(Parcel &parcel) const
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start, mimeType: %{public}s,", mimeType_.c_str());
    if (!parcel.WriteString(mimeType_)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write mimeType failed.");
        return false;
    }
    bool ret = MarshallingString(parcel, htmlText_, MimeType_Html);
    ret = MarshallingParcelable(parcel, want_, MimeType_Want) && ret;
    ret = MarshallingString(parcel, plainText_, MimeType_Plain) && ret;
    ret = MarshallingParcelable(parcel, uri_, MimeType_Uri) && ret;
    ret = MarshallingParcelable(parcel, pixelMap_, MimeType_PixelMap) && ret;
    return ret;
}

bool PasteDataRecord::ParcelableReadFromParcel(Parcel &parcel, uint32_t symbol)
{
    if (!parcel.ReadBool()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "no need ReadFromParcel, symbol = %{public}d.", symbol);
        return true;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "ReadFromParcel, symbol = %{public}d.", symbol);
    switch (symbol) {
        case MimeType_Uri: {
            std::shared_ptr<OHOS::Uri> uri(parcel.ReadParcelable<OHOS::Uri>());
            if (!uri) {
                return false;
            }
            uri_ = uri;
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, " read uri success.");
            break;
        }
        case MimeType_Want: {
            std::shared_ptr<OHOS::AAFwk::Want> want(parcel.ReadParcelable<OHOS::AAFwk::Want>());
            if (!want) {
                return false;
            }
            want_ = want;
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "read want success.");
            break;
        }
        case MimeType_PixelMap: {
            std::shared_ptr<PixelMap> pixelMap(parcel.ReadParcelable<PixelMap>());
            if (!pixelMap) {
                return false;
            }
            pixelMap_ = pixelMap;
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "read pixelMap success: width = %{public}d.", pixelMap_->GetWidth());
            break;
        }
        default: {
            return false;
        }
    }
    return true;
}


bool PasteDataRecord::StringReadFromParcel(Parcel &parcel, uint32_t symbol)
{
    if (!parcel.ReadBool()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "no need ReadFromParcel, symbol = %{public}d.", symbol);
        return true;
    }

    std::shared_ptr<std::string> result = std::make_shared<std::string>(parcel.ReadString());
    if (!result) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "ReadFromParcel failed, symbol = %{public}d.", symbol);
        return false;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "ReadFromParcel success, symbol = %{public}d.", symbol);

    switch (symbol) {
        case MimeType_Html: {
            htmlText_ = result;
            break;
        }
        case MimeType_Plain: {
            plainText_ = result;
            break;
        }
        default: {
            return false;
        }
    }
    return true;
}

bool PasteDataRecord::ReadFromParcel(Parcel &parcel)
{
    mimeType_ = parcel.ReadString();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "mimeType_0806: %{public}s,", mimeType_.c_str());

    bool ret = StringReadFromParcel(parcel, MimeType_Html);
    ret = ParcelableReadFromParcel(parcel, MimeType_Want) && ret;
    ret = StringReadFromParcel(parcel, MimeType_Plain) && ret;
    ret = ParcelableReadFromParcel(parcel, MimeType_Uri) && ret;
    ret = ParcelableReadFromParcel(parcel, MimeType_PixelMap) && ret;
    return ret;
}

PasteDataRecord *PasteDataRecord::Unmarshalling(Parcel &parcel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    auto *pasteDataRecord = new PasteDataRecord();

    if (pasteDataRecord && !pasteDataRecord->ReadFromParcel(parcel)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "delete end.");
        delete pasteDataRecord;
        pasteDataRecord = nullptr;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return pasteDataRecord;
}
} // MiscServices
} // OHOS