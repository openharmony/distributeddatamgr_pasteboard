/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>
#include <map>

#include "pasteboard_client.h"
#include "pasteboard_observer.h"
#include "paste_data.h"
#include "paste_data_record.h"

using namespace OHOS::MiscServices;

namespace OHOS {
using namespace OHOS::Media;
using namespace OHOS::AAFwk;
constexpr size_t THRESHOLD = 5;
constexpr size_t OFFSET = 4;
constexpr size_t RANDNUM_ZERO = 0;
constexpr size_t LENGTH = 46;

uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

void FuzzPasteboardclient(const uint8_t *rawData, size_t size)
{
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    std::shared_ptr<PasteDataRecord> pasteDataRecord = std::make_shared<PasteDataRecord>();
    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    std::string str(reinterpret_cast<const char *>(rawData), size);

    if (code == RANDNUM_ZERO) {
        pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(str);
        pasteDataRecord = PasteboardClient::GetInstance()->CreatePlainTextRecord(str);
    } else {
        pasteData = PasteboardClient::GetInstance()->CreateUriData(Uri(str));
        pasteDataRecord = PasteboardClient::GetInstance()->CreateUriRecord(Uri(str));
    }
    pasteData->AddRecord(pasteDataRecord);
    std::vector<uint8_t> buffer;
    pasteData->Encode(buffer);

    PasteData pasteData2;
    pasteData2.Decode(buffer);
    pasteData2.HasMimeType(std::string(reinterpret_cast<const char *>(rawData), size));
    pasteData2.RemoveRecordAt(code);
    pasteData2.ReplaceRecordAt(code, pasteDataRecord);

    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
    std::shared_ptr<PixelMap> pixelMap = std::make_shared<PixelMap>();
    const std::vector<uint8_t> arrayBuffer = {rawData, rawData + size};
    const OHOS::Uri uri = Uri(str);
    PasteboardClient::GetInstance()->CreateHtmlTextRecord(str);
    PasteboardClient::GetInstance()->CreateWantRecord(want);
    PasteboardClient::GetInstance()->CreatePlainTextRecord(str);
    PasteboardClient::GetInstance()->CreateUriRecord(uri);
    PasteboardClient::GetInstance()->CreateKvRecord(str, arrayBuffer);
    PasteboardClient::GetInstance()->CreateHtmlData(str);
    PasteboardClient::GetInstance()->CreateWantData(want);
    PasteboardClient::GetInstance()->CreatePlainTextData(str);
    PasteboardClient::GetInstance()->CreatePixelMapData(pixelMap);
    PasteboardClient::GetInstance()->CreateUriData(uri);
    PasteboardClient::GetInstance()->CreateKvData(str, arrayBuffer);
}

void FuzzPasteboardclient002(const uint8_t *rawData, size_t size)
{
    PasteData pasteData3;
    PasteboardClient::GetInstance()->GetPasteData(pasteData3);
    std::shared_ptr<GetDataParams> params;
    PasteboardClient::GetInstance()->GetDataWithProgress(pasteData3, params);
    UDMF::UnifiedData unifiedData;
    PasteboardClient::GetInstance()->GetUnifiedData(unifiedData);
    PasteboardClient::GetInstance()->HasPasteData();
    std::shared_ptr<PasteboardDelayGetter> delayGetter;
    PasteboardClient::GetInstance()->SetPasteData(pasteData3, delayGetter);
    PasteboardClient::GetInstance()->SetUnifiedData(unifiedData, delayGetter);
    PasteboardObserverType type = PasteboardObserverType::OBSERVER_LOCAL;
    sptr<PasteboardObserver> callback = new PasteboardObserver();
    PasteboardClient::GetInstance()->Subscribe(type, callback);
    PasteboardClient::GetInstance()->AddPasteboardChangedObserver(callback);
    PasteboardClient::GetInstance()->AddPasteboardEventObserver(callback);
    PasteboardClient::GetInstance()->Unsubscribe(type, callback);
    PasteboardClient::GetInstance()->RemovePasteboardChangedObserver(callback);
    PasteboardClient::GetInstance()->RemovePasteboardEventObserver(callback);
    const std::vector<uint32_t> tokenIds = {1, 2, 3};
    PasteboardClient::GetInstance()->GetGlobalShareOption(tokenIds);
    PasteboardClient::GetInstance()->RemoveGlobalShareOption(tokenIds);
    const ShareOption shareOptions = ShareOption::LocalDevice;
    PasteboardClient::GetInstance()->SetAppShareOptions(shareOptions);
    PasteboardClient::GetInstance()->Clear();
    uint32_t changeCount = 0;
    PasteboardClient::GetInstance()->GetChangeCount(changeCount);
}

void FuzzPasteboard(const uint8_t *rawData, size_t size)
{
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    std::shared_ptr<PasteDataRecord> pasteDataRecord = std::make_shared<PasteDataRecord>();
    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    std::string str(reinterpret_cast<const char *>(rawData), size);
    uint32_t color[100] = { code };
    InitializationOptions opts = { { 5, 7}, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color)/sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);

    std::vector<uint8_t> kvData(LENGTH);
    kvData = { *rawData };
    std::string mimetype = "image/jpg";

    if (code == RANDNUM_ZERO) {
        pasteData = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMapIn);
        pasteDataRecord = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMapIn);
    } else {
        pasteData = PasteboardClient::GetInstance()->CreateKvData(mimetype, kvData);
        pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimetype, kvData);
    }

    pasteData->AddRecord(pasteDataRecord);
    if (PasteboardClient::GetInstance()->HasPasteData()) {
        PasteboardClient::GetInstance()->RemovePasteboardChangedObserver(nullptr);
    }
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    uint32_t changeCount = 0;
    PasteboardClient::GetInstance()->GetChangeCount(changeCount);
    std::set<Pattern> patternsToCheck = {Pattern::URL, Pattern::EmailAddress, static_cast<Pattern>(code)};
    PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck);
}

void FuzzPastedata(const uint8_t *rawData, size_t size)
{
    std::string str(reinterpret_cast<const char *>(rawData), size);
    PasteData pasteData2;
    pasteData2.SetRemote(static_cast<bool>(*rawData));
    pasteData2.SetLocalPasteFlag(static_cast<bool>(*rawData));
    pasteData2.SetDraggedDataFlag(static_cast<bool>(*rawData));
    pasteData2.SetOrginAuthority(str);
    pasteData2.SetBundleName(str);
    pasteData2.SetTag(str);
    pasteData2.SetTime(str);
    pasteData2.SetDelayData(false);
    pasteData2.IsDelayData();
    pasteData2.IsValid();
    pasteData2.IsRemote();
    pasteData2.IsLocalPaste();
    pasteData2.GetLocalOnly();
    pasteData2.IsDraggedData();
    pasteData2.GetDeviceId();
    pasteData2.ReplaceShareUri(1);
    pasteData2.SetLocalOnly(false);
    AAFwk::WantParams additions;
    pasteData2.SetAdditions(additions);
    pasteData2.GetTag();
    ScreenEvent screenStatus = ScreenEvent::ScreenLocked;
    pasteData2.SetScreenStatus(screenStatus);
    pasteData2.GetScreenStatus();
    pasteData2.GetTime();
    pasteData2.SetOrginAuthority(str);
    pasteData2.GetOrginAuthority();
    pasteData2.SetBundleName(str);
    pasteData2.GetBundleName();
    pasteData2.GetDeviceId();
    pasteData2.IsDraggedData();
    pasteData2.GetLocalOnly();
    pasteData2.AllRecords();
    pasteData2.SetTokenId(0);
    pasteData2.GetTokenId();
    pasteData2.GetRecordAt(0);
}

void FuzzPasteData002(const uint8_t *rawData, size_t size)
{
    std::string str(reinterpret_cast<const char *>(rawData), size);
    PasteData pasteData2;
    pasteData2.GetPrimaryText();
    pasteData2.GetPrimaryWant();
    pasteData2.GetPrimaryPixelMap();
    pasteData2.GetPrimaryHtml();
    std::shared_ptr<OHOS::AAFwk::Want> wantPtr = std::make_shared<OHOS::AAFwk::Want>();
    pasteData2.AddWantRecord(wantPtr);
    std::shared_ptr<PixelMap> pixelMap = std::make_shared<PixelMap>();
    pasteData2.AddPixelMapRecord(pixelMap);
    pasteData2.AddHtmlRecord(str);
    PasteDataProperty property;
    pasteData2.SetProperty(property);
    pasteData2.GetProperty();
    std::shared_ptr<PasteDataRecord> pasteDataRecord = std::make_shared<PasteDataRecord>();
    pasteData2.AddRecord(pasteDataRecord);

    PasteData pasteData1 = pasteData2;
    PasteboardClient::GetInstance()->SetPasteData(pasteData2);
    uint32_t changeCount = 0;
    PasteboardClient::GetInstance()->GetChangeCount(changeCount);
    PasteboardClient::GetInstance()->GetMimeTypes();
    PasteboardClient::GetInstance()->HasDataType(std::string(reinterpret_cast<const char *>(rawData), size));
    PasteboardClient::GetInstance()->IsRemoteData();
    std::string bundlename = pasteData2.GetBundleName();
    PasteboardClient::GetInstance()->GetPasteData(pasteData2);
    std::shared_ptr<GetDataParams> params = std::make_shared<GetDataParams>();
    PasteboardClient::GetInstance()->GetDataWithProgress(pasteData2, params);
    PasteboardClient::GetInstance()->GetDataSource(bundlename);

    std::string shareoption1;
    PasteData::ShareOptionToString(ShareOption::InApp, shareoption1);
    PasteData::ShareOptionToString(ShareOption::LocalDevice, shareoption1);
    PasteData::ShareOptionToString(ShareOption::CrossDevice, shareoption1);
    std::vector<std::uint8_t> buffer = {rawData, rawData + size};
    pasteData2.Decode(buffer);
    pasteData2.SetInvalid();
    sptr<IRemoteObject> remoteObject = nullptr;
    PasteboardClient::GetInstance()->LoadSystemAbilitySuccess(remoteObject);
    PasteboardClient::GetInstance()->LoadSystemAbilityFail();
    const wptr<IRemoteObject> object;
    PasteboardClient::GetInstance()->OnRemoteSaDied(object);
    PasteboardClient::GetInstance()->Clear();
    PasteboardClient::GetInstance()->GetChangeCount(changeCount);
}

void FuzzPastedataProperty(const uint8_t *rawData, size_t size)
{
    std::string str(reinterpret_cast<const char *>(rawData), size);
    PasteDataProperty property1;
    property1.tag ="tag1";
    PasteDataProperty property2(property1);

    std::vector<std::uint8_t> buffer = {rawData, rawData + size};
    property1.DecodeTag(buffer);
    property1.Decode(buffer);
}

void FuzzPastedataRecord(const uint8_t *rawData, size_t size)
{
    std::string str(reinterpret_cast<const char *>(rawData), size);
    std::vector<std::uint8_t> buffer = {rawData, rawData + size};
    PasteDataRecord pasteDataRecord;

    std::shared_ptr<std::string> htmlText = std::make_shared<std::string>(str);
    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = std::make_shared<PixelMap>();
    std::shared_ptr<OHOS::Uri> uri = std::make_shared<Uri>(str);
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();

    pasteDataRecord.SetUri(uri);
    const std::string htmlText2(reinterpret_cast<const char *>(rawData), size);
    pasteDataRecord.NewHtmlRecord(htmlText2);
    pasteDataRecord.NewWantRecord(want);
    pasteDataRecord.NewPlainTextRecord(str);
    pasteDataRecord.NewPixelMapRecord(pixelMap);
    pasteDataRecord.GetHtmlText();
    pasteDataRecord.GetMimeType();
    pasteDataRecord.GetPlainText();
    pasteDataRecord.GetPixelMap();
    pasteDataRecord.GetOrginUri();
    pasteDataRecord.GetWant();
    pasteDataRecord.GetCustomData();
    pasteDataRecord.GetUri();
    pasteDataRecord.ClearPixelMap();
    pasteDataRecord.ConvertToText();
    pasteDataRecord.Encode(buffer);
    pasteDataRecord.Decode(buffer);
    pasteDataRecord.Count();
    pasteDataRecord.ReplaceShareUri(1);
    pasteDataRecord.SetConvertUri(str),
    pasteDataRecord.GetConvertUri();
    pasteDataRecord.SetGrantUriPermission(false);
    pasteDataRecord.HasGrantUriPermission();
    pasteDataRecord.SetTextContent(str);
    pasteDataRecord.GetTextContent();
}

void FuzzPastedataRecord002(const uint8_t *rawData, size_t size)
{
    std::string str(reinterpret_cast<const char *>(rawData), size);
    std::vector<std::uint8_t> buffer = {rawData, rawData + size};
    PasteDataRecord pasteDataRecord;

    std::shared_ptr<std::string> htmlText = std::make_shared<std::string>(str);
    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = std::make_shared<PixelMap>();
    std::shared_ptr<OHOS::Uri> uri = std::make_shared<Uri>(str);
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();

    Details details;
    pasteDataRecord.SetDetails(details);
    pasteDataRecord.GetDetails();
    pasteDataRecord.SetSystemDefinedContent(details);
    pasteDataRecord.GetSystemDefinedContent();
    pasteDataRecord.SetUDType(0);
    pasteDataRecord.GetUDType();

    std::vector<std::uint8_t> value = {rawData, rawData + size};
    std::shared_ptr<PixelMap> pixelMap2 = std::make_shared<PixelMap>();
    pasteDataRecord.Vector2PixelMap(value);
    pasteDataRecord.PixelMap2Vector(pixelMap2);
    std::string mimeType(reinterpret_cast<const char *>(rawData), size);
    std::vector<uint8_t> arrayBuffer;
    PasteDataRecord::NewKvRecord(mimeType, arrayBuffer);

    PasteDataRecord::Builder builder(MIMETYPE_TEXT_HTML);
    std::shared_ptr<std::string> htmlText3 = std::make_shared<std::string>(str);
    builder.SetHtmlText(htmlText3);
    builder.SetMimeType(str);
    builder.SetPlainText(htmlText3);
    builder.SetUri(uri);
    builder.SetPixelMap(pixelMap);
    builder.SetWant(want);
    builder.SetCustomData(customData);
    builder.Build();
}

void FuzzMinecustomData(const uint8_t *rawData, size_t size)
{
    std::string str(reinterpret_cast<const char *>(rawData), size);
    std::string mimeType(reinterpret_cast<const char *>(rawData), size);
    std::vector<uint8_t> arrayBuffer;
    MineCustomData customData;

    std::vector<std::uint8_t> buffer = {rawData, rawData + size};
    customData.Encode(buffer);
    customData.Decode(buffer);
    customData.Count();
    customData.GetItemData();
    customData.AddItemData(mimeType, arrayBuffer);
}

void FuzzPasteboardclientcreateData(const uint8_t *rawData, size_t size)
{
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    std::shared_ptr<PasteDataRecord> pasteDataRecord = std::make_shared<PasteDataRecord>();
    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    std::string str(reinterpret_cast<const char *>(rawData), size);

    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    bool id = static_cast<bool>(*rawData);
    Want wantIn = want->SetParam(key, id);

    if (code == RANDNUM_ZERO) {
        pasteData = PasteboardClient::GetInstance()->CreateHtmlData(str);
        pasteDataRecord = PasteboardClient::GetInstance()->CreateHtmlTextRecord(str);
    } else {
        pasteData = PasteboardClient::GetInstance()->CreateWantData(std::make_shared<Want>(wantIn));
        pasteDataRecord = PasteboardClient::GetInstance()->CreateWantRecord(std::make_shared<Want>(wantIn));
    }
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    uint32_t changeCount = 0;
    PasteboardClient::GetInstance()->GetChangeCount(changeCount);
    std::set<Pattern> patternsToCheck = {Pattern::URL, Pattern::Number, static_cast<Pattern>(code)};
    PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck);
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzPasteboardclient(data, size);
    OHOS::FuzzPasteboardclient002(data, size);
    OHOS::FuzzPasteboard(data, size);
    OHOS::FuzzPastedata(data, size);
    OHOS::FuzzPasteData002(data, size);
    OHOS::FuzzMinecustomData(data, size);
    OHOS::FuzzPastedataProperty(data, size);
    OHOS::FuzzPastedataRecord(data, size);
    OHOS::FuzzPastedataRecord002(data, size);
    OHOS::FuzzPasteboardclientcreateData(data, size);
    return 0;
}