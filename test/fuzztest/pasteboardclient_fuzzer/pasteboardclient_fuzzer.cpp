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

using namespace OHOS::MiscServices;

namespace OHOS {
using namespace OHOS::Media;
using namespace OHOS::AAFwk;
constexpr size_t THRESHOLD = 10;
constexpr size_t OFFSET = 4;
constexpr size_t RANDNUM_ZERO = 0;
constexpr size_t RANDNUM_ONE = 1;

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
    switch (code) {
        case RANDNUM_ZERO:
            pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(str);
            pasteDataRecord = PasteboardClient::GetInstance()->CreatePlainTextRecord(str);
            break;
        case RANDNUM_ONE:
            pasteData = PasteboardClient::GetInstance()->CreateHtmlData(str);
            pasteDataRecord = PasteboardClient::GetInstance()->CreateHtmlTextRecord(str);
            break;
        default:
            pasteData = PasteboardClient::GetInstance()->CreateUriData(Uri(str));
            pasteDataRecord = PasteboardClient::GetInstance()->CreateUriRecord(Uri(str));
            break;
    }
    pasteData->AddRecord(pasteDataRecord);
    std::vector<uint8_t> buffer;
    pasteData->Encode(buffer);

    PasteData pasteData2;
    pasteData2.Decode(buffer);
    pasteData2.HasMimeType(std::string(reinterpret_cast<const char *>(rawData), size));
    pasteData2.RemoveRecordAt(code);
    pasteData2.ReplaceRecordAt(code, pasteDataRecord);
}

void FuzzPasteboardclientTest(const uint8_t *rawData, size_t size)
{
    if((rawData == nullptr) || (size < sizeof(int32_t))){
        return;
    }
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    std::shared_ptr<PasteDataRecord> pasteDataRecord = std::make_shared<PasteDataRecord>();
    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    std::string str(reinterpret_cast<const char *>(rawData), size);

    uint32_t color[100] = { code };
    InitialzationOptions opts = { { 5, 7}, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color)/sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);

    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    bool id = static_cast<bool>(*rawData);
    Want wantIn = want->SetParam(key, id);

    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { *rawData };
    std::string mimetype = "image/jpg";

    switch (code) {
        case RANDNUM_ZERO:
            pasteData = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMapIn);
            pasteDataRecord = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMapIn);
            break;
        case RANDNUM_ONE:
            pasteData = PasteboardClient::GetInstance()->CreateWantData(wantIn);
            pasteDataRecord = PasteboardClient::GetInstance()->CreateWantTextRecord(wantIn);
            break;
        default:
            pasteData = PasteboardClient::GetInstance()->CreateKvData(mimetype, arrayBuffer);
            pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimetype, arrayBuffer);
            break;
    }
    pasteData->AddRecord(pasteDataRecord);

    if (PasteboardClient::GetInstance()->HasPasteData())
    {
        PasteboardClient::GetInstance()->RemovePasteboardChangedObserver(nullptr);
        PasteboardClient::GetInstance()->RemovePasteboardEventObserver(nullptr);
        auto pasteboardObserver_ = new PasteboardObserver();
        auto pasteboardEventObserver_ = new PasteboardObserver();
        PasteboardClient::GetInstance()->AddPasteboardChangedObserver(pasteboardObserver_);
        PasteboardClient::GetInstance()->AddPasteboardChangedEventObserver(pasteboardEventObserver_);
    }

    PasteData pasteData2;
    pasteData2.SetRemote(static_cast<bool>(*rawData));
    pasteData2.SetOrginAuthority(str);
    pasteData2.SetBundleName(str);
    pasteData2.SetTag(str);
    pasteData2.SetTime(str);
    
    if (pasteData2.IsRemote())
    {
        pasteData2.IsValid();
        pasteData2.IsLocalPaste();
        pasteData2.GetLocalOnly();
        pasteData2.IsDraggedData();
        pasteData2.GetBundleName();
        pasteData2.GetOrginAuthority();
        pasteData2.GetTag();
        pasteData2.GetTime();
        PasteboardClient::GetInstance()->SetPasteData(pasteData2);
        PasteboardClient::GetInstance()->HasDataType(std::string(reinterpret_cast<const char *>(rawData), size));
        PasteboardClient::GetInstance()->IsRemoteData();
    }
    pasteData2.SetInvalid();

    sptr<IRemoteObject> remoteObject = nullptr;
    PasteboardClient::GetInstance()->LoadSystemAbility(remoteObject);

    const wptr<IRemoteObject> object;
    PasteboardSaDeathRecipient death;
    death.OnRemoteDied(object);
    PasteboardClient::GetInstance()->OnRemoteSaDied(object);

    PasteboardClient::GetInstance()->Clear();
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
    OHOS::FuzzPasteboardclientTest(data, size);
    return 0;
}
