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

#ifndef PASTE_BOARD_RECORD_H
#define PASTE_BOARD_RECORD_H

#include <memory>
#include <string>

#include "message_parcel.h"
#include "pixel_map.h"
#include "string_ex.h"
#include "tlv_object.h"
#include "unified_meta.h"
#include "uri.h"
#include "uri_handler.h"
#include "want.h"
namespace OHOS {
namespace MiscServices {
namespace {
const std::string MIMETYPE_PIXELMAP = "pixelMap";
const std::string MIMETYPE_TEXT_HTML = "text/html";
const std::string MIMETYPE_TEXT_PLAIN = "text/plain";
const std::string MIMETYPE_TEXT_URI = "text/uri";
const std::string MIMETYPE_TEXT_WANT = "text/want";
} // namespace

enum ResultCode : int32_t { OK = 0, IPC_NO_DATA, IPC_ERROR };

class API_EXPORT MineCustomData : public TLVObject {
public:
    MineCustomData() = default;
    std::map<std::string, std::vector<uint8_t>> GetItemData();
    void AddItemData(const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer);
    bool Encode(std::vector<std::uint8_t> &buffer) override;
    bool Decode(const std::vector<std::uint8_t> &buffer) override;
    size_t Count() override;

private:
    std::map<std::string, std::vector<uint8_t>> itemData_;
};

class FileDescriptor {
public:
    FileDescriptor() = default;
    ~FileDescriptor();
    void SetFd(int32_t fd);
    int32_t GetFd() const;

private:
    int32_t fd_ = -1;
};

class API_EXPORT PasteDataRecord : public TLVObject {
public:
    PasteDataRecord();
    ~PasteDataRecord();
    PasteDataRecord(const PasteDataRecord &record);
    PasteDataRecord(std::string mimeType, std::shared_ptr<std::string> htmlText,
        std::shared_ptr<OHOS::AAFwk::Want> want, std::shared_ptr<std::string> plainText,
        std::shared_ptr<OHOS::Uri> uri);

    static std::shared_ptr<PasteDataRecord> NewHtmlRecord(const std::string &htmlText);
    static std::shared_ptr<PasteDataRecord> NewWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want);
    static std::shared_ptr<PasteDataRecord> NewPlaintTextRecord(const std::string &text);
    static std::shared_ptr<PasteDataRecord> NewPixelMapRecord(std::shared_ptr<OHOS::Media::PixelMap> pixelMap);
    static std::shared_ptr<PasteDataRecord> NewUriRecord(const OHOS::Uri &uri);
    static std::shared_ptr<PasteDataRecord> NewKvRecord(
        const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer);

    bool isConvertUriFromRemote = false;
    std::string GetMimeType() const;
    std::shared_ptr<std::string> GetHtmlText() const;
    std::shared_ptr<std::string> GetPlainText() const;
    std::shared_ptr<OHOS::Media::PixelMap> GetPixelMap() const;
    void ClearPixelMap();
    std::shared_ptr<OHOS::Uri> GetUri() const;
    void SetUri(std::shared_ptr<OHOS::Uri> uri);
    std::shared_ptr<OHOS::Uri> GetOrginUri() const;
    std::shared_ptr<OHOS::AAFwk::Want> GetWant() const;
    std::shared_ptr<MineCustomData> GetCustomData() const;

    std::string ConvertToText() const;

    bool Encode(std::vector<std::uint8_t> &buffer) override;
    bool Decode(const std::vector<std::uint8_t> &buffer) override;
    size_t Count() override;
    bool WriteFd(MessageParcel &parcel, UriHandler &uriHandler, bool isClient);
    bool ReadFd(MessageParcel &parcel, UriHandler &uriHandler);
    bool NeedFd(const UriHandler &uriHandler);
    void ReplaceShareUri(int32_t userId);
    void SetConvertUri(const std::string &value);
    std::string GetConvertUri() const;
    void SetGrantUriPermission(bool hasPermission);
    bool HasGrantUriPermission();
    static std::shared_ptr<OHOS::Media::PixelMap> Vector2PixelMap(std::vector<std::uint8_t> &value);
    static std::vector<std::uint8_t> PixelMap2Vector(std::shared_ptr<OHOS::Media::PixelMap> &pixelMap);

    void SetTextContent(const std::string& content);
    std::string GetTextContent() const;
    void SetDetails(const Details& details);
    std::shared_ptr<Details> GetDetails() const;
    void SetSystemDefinedContent(const Details& contents);
    std::shared_ptr<Details> GetSystemDefinedContent() const;
    int32_t GetUDType() const;
    void SetUDType(int32_t type);

    class Builder {
    public:
        explicit Builder(const std::string &mimeType);
        Builder &SetHtmlText(std::shared_ptr<std::string> htmlText);
        Builder &SetWant(std::shared_ptr<OHOS::AAFwk::Want> want);
        Builder &SetPlainText(std::shared_ptr<std::string> plainText);
        Builder &SetUri(std::shared_ptr<OHOS::Uri> uri);
        Builder &SetPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap);
        Builder &SetCustomData(std::shared_ptr<MineCustomData> customData);
        Builder &SetMimeType(std::string mimeType);
        std::shared_ptr<PasteDataRecord> Build();

    private:
        std::shared_ptr<PasteDataRecord> record_ = nullptr;
    };

private:
    inline static bool CheckResult(ResultCode resultCode)
    {
        return resultCode == ResultCode::OK;
    }
    std::string GetPassUri();

    std::string mimeType_;
    std::shared_ptr<std::string> htmlText_;
    std::shared_ptr<OHOS::AAFwk::Want> want_;
    std::shared_ptr<std::string> plainText_;
    std::shared_ptr<OHOS::Uri> uri_;
    std::string convertUri_;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap_;
    std::shared_ptr<MineCustomData> customData_;
    bool hasGrantUriPermission_ = false;
    std::shared_ptr<FileDescriptor> fd_;
    using Func = std::function<void(bool &ret, const std::vector<std::uint8_t> &buffer, TLVHead &head)>;
    std::map<uint16_t, Func> decodeMap;
    void InitDecodeMap();

    int32_t udType_ = UDMF::UD_BUTT;
    std::shared_ptr<Details> details_;
    std::string textContent_;
    std::shared_ptr<Details> systemDefinedContents_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_RECORD_H
