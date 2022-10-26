/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

using namespace OHOS::MiscServices;

namespace OHOS {
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
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzPasteboardclient(data, size);
    return 0;
}
