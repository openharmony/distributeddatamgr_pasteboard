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

bool PasteDataRecord::Marshalling(Parcel &parcel) const
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start, mimeType: %{public}s,", mimeType_.c_str());
    if (!parcel.WriteString16(Str8ToStr16(mimeType_))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write mimeType failed.");
        return false;
    }
    if ((mimeType_ == MIMETYPE_TEXT_PLAIN) && (plainText_ != nullptr)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Write plainText, plainText_: %{public}s,", (*plainText_).c_str());
        if (!parcel.WriteString16(Str8ToStr16(*plainText_))) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write plainText failed.");
            return false;
        }
    } else if ((mimeType_ == MIMETYPE_TEXT_HTML) && (htmlText_ != nullptr)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Write htmlText, htmlText_: %{public}s,", (*htmlText_).c_str());
        if (!parcel.WriteString16(Str8ToStr16(*htmlText_))) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write htmlText failed.");
            return false;
        }
    } else if ((mimeType_ == MIMETYPE_TEXT_URI) && (uri_ != nullptr)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Write uri, uri_: %{public}s,", uri_->ToString().c_str());
        if (!parcel.WriteParcelable(uri_.get())) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write uri failed.");
            return false;
        }
    } else if ((mimeType_ == MIMETYPE_TEXT_WANT) && (want_ != nullptr)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Write want");
        if (!parcel.WriteParcelable(want_.get())) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write want failed.");
            return false;
        }
    } else if ((mimeType_ == MIMETYPE_PIXELMAP) && (pixelMap_ != nullptr)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Write pixelMap");
        if (!parcel.WriteParcelable(pixelMap_.get())) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write pixelMap failed.");
            return false;
        }
    } else {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write failed, mimeType: %{public}s.", mimeType_.c_str());
        return false;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "end.");
    return true;
}

bool PasteDataRecord::ReadFromParcel(Parcel &parcel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    mimeType_ = Str16ToStr8(parcel.ReadString16());
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "mimeType_: %{public}s,", mimeType_.c_str());

    if (mimeType_ == MIMETYPE_TEXT_HTML) {
        htmlText_ = std::make_shared<std::string>(Str16ToStr8(parcel.ReadString16()));
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "htmlText_: %{public}s,", (*htmlText_).c_str());
    } else if (mimeType_ == MIMETYPE_TEXT_PLAIN) {
        plainText_ = std::make_shared<std::string>(Str16ToStr8(parcel.ReadString16()));
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "plainText_: %{public}s,", (*plainText_).c_str());
    } else if (mimeType_ == MIMETYPE_TEXT_URI) {
        std::unique_ptr<OHOS::Uri> uri(parcel.ReadParcelable<OHOS::Uri>());
        if (!uri) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "uri nullptr.");
            return false;
        }
        uri_ = std::make_shared<OHOS::Uri>(*uri);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "uri_: %{public}s,", uri_->ToString().c_str());
    } else if (mimeType_ == MIMETYPE_TEXT_WANT) {
        std::unique_ptr<OHOS::AAFwk::Want> want(parcel.ReadParcelable<OHOS::AAFwk::Want>());
        if (!want) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "want nullptr.");
            return false;
        }
        want_ = std::make_shared<OHOS::AAFwk::Want>(*want);
    } else if (mimeType_ == MIMETYPE_PIXELMAP) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "read pixelMap.");
        std::shared_ptr<PixelMap> pixelMap(parcel.ReadParcelable<PixelMap>());
        if (!pixelMap) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "pixelMap nullptr.");
            return false;
        }
        pixelMap_ = pixelMap;
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Unkonw MimeType: %{public}s.", mimeType_.c_str());
        return false;
    }

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return true;
}

PasteDataRecord *PasteDataRecord::Unmarshalling(Parcel &parcel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    PasteDataRecord *pasteDataRecord = new PasteDataRecord();

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