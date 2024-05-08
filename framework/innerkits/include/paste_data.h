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
#ifndef PASTE_BOARD_DATA_H
#define PASTE_BOARD_DATA_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "paste_data_record.h"
#include "pasteboard_hilog.h"
#include "tlv_object.h"
#include "uri.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace MiscServices {
enum ShareOption : int32_t { InApp = 0, LocalDevice, CrossDevice };
struct API_EXPORT PasteDataProperty : public TLVObject {
    PasteDataProperty() = default;
    ~PasteDataProperty();
    explicit PasteDataProperty(const PasteDataProperty &property);
    PasteDataProperty& operator=(const PasteDataProperty &property);
    AAFwk::WantParams additions;
    std::vector<std::string> mimeTypes;
    std::string tag;
    std::int64_t timestamp;
    bool localOnly;
    ShareOption shareOption;
    uint32_t tokenId = 0;
    bool isRemote = false;
    std::string bundleName;
    std::string setTime;

    bool Encode(std::vector<std::uint8_t> &buffer) override;
    bool Decode(const std::vector<std::uint8_t> &buffer) override;
    bool DecodeTag(const std::vector<std::uint8_t> &buffer);
    size_t Count() override;
};

class API_EXPORT PasteData : public TLVObject {
public:
    static constexpr const std::uint32_t MAX_RECORD_NUM = 512;
    PasteData();
    ~PasteData();
    explicit PasteData(const PasteData &data);
    PasteData& operator=(const PasteData &data);
    explicit PasteData(std::vector<std::shared_ptr<PasteDataRecord>> records);

    void AddHtmlRecord(const std::string &html);
    void AddKvRecord(const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer);
    void AddPixelMapRecord(std::shared_ptr<OHOS::Media::PixelMap> pixelMap);
    void AddTextRecord(const std::string &text);
    void AddUriRecord(const OHOS::Uri &uri);
    void AddWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want);
    void AddRecord(std::shared_ptr<PasteDataRecord> record);
    void AddRecord(PasteDataRecord &record);
    std::vector<std::string> GetMimeTypes();
    std::shared_ptr<std::string> GetPrimaryHtml();
    std::shared_ptr<OHOS::Media::PixelMap> GetPrimaryPixelMap();
    std::shared_ptr<std::string> GetPrimaryText();
    std::shared_ptr<OHOS::Uri> GetPrimaryUri();
    std::shared_ptr<std::string> GetPrimaryMimeType();
    std::shared_ptr<OHOS::AAFwk::Want> GetPrimaryWant();
    std::shared_ptr<PasteDataRecord> GetRecordAt(std::size_t index) const;
    std::size_t GetRecordCount() const;
    bool RemoveRecordAt(std::size_t number);
    bool ReplaceRecordAt(std::size_t number, std::shared_ptr<PasteDataRecord> record);
    bool HasMimeType(const std::string &mimeType);
    PasteDataProperty GetProperty() const;
    void SetProperty(const PasteDataProperty &property);
    ShareOption GetShareOption();
    void SetShareOption(ShareOption shareOption);
    uint32_t GetTokenId();
    void SetTokenId(uint32_t tokenId);
    std::vector<std::shared_ptr<PasteDataRecord>> AllRecords() const;
    bool IsDraggedData() const;
    void SetDraggedDataFlag(bool isDraggedData);
    bool IsLocalPaste() const;
    void SetLocalPasteFlag(bool isLocalPaste);

    void SetBundleName(const std::string &bundleName);
    std::string GetBundleName() const;
    void SetOrginAuthority(const std::string &bundleName);
    std::string GetOrginAuthority() const;
    void SetRemote(bool isRemote);
    bool IsRemote();
    void SetTime(const std::string &time);
    std::string GetTime();
    void SetTag(std::string &tag);
    std::string GetTag();
    void SetAdditions(AAFwk::WantParams &additions);
    void SetAddition(const std::string &key, AAFwk::IInterface *value);
    void SetLocalOnly(bool localOnly);
    bool GetLocalOnly();

    bool Encode(std::vector<std::uint8_t> &buffer) override;
    bool Decode(const std::vector<std::uint8_t> &buffer) override;
    size_t Count() override;
    bool WriteUriFd(MessageParcel &parcel, UriHandler &uriHandler, bool isClient = true);
    bool ReadUriFd(MessageParcel &parcel, UriHandler &uriHandler);
    void ReplaceShareUri(int32_t userId);

    bool IsValid() const;
    void SetInvalid();

    void SetDelayData(bool isDelay);
    bool IsDelayData() const;

    static void ShareOptionToString(ShareOption shareOption, std::string &out);
    static std::string sharePath;
    static std::string WEBVIEW_PASTEDATA_TAG;
    static const std::string DISTRIBUTEDFILES_TAG;
    static const std::string PATH_SHARE;
    static const std::string FILE_SCHEME_PREFIX;
    static const std::string IMG_LOCAL_URI;
    static const std::string SHARE_PATH_PREFIX;
    static const std::string SHARE_PATH_PREFIX_ACCOUNT;
    static const std::string REMOTE_FILE_SIZE;

private:
    void RefreshMimeProp();

    PasteDataProperty props_;
    std::vector<std::shared_ptr<PasteDataRecord>> records_;
    std::string orginAuthority_;
    bool valid_ = true;
    bool isDraggedData_ = false;
    bool isLocalPaste_ = false; // local in app paste
    bool isDelayData_ = false;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_DATA_H
