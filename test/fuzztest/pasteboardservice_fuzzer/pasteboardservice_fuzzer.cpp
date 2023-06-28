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

#include "pasteboardservice_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <map>

#include "copy_uri_handler.h"
#include "i_pasteboard_service.h"
#include "message_parcel.h"
#include "paste_data.h"
#include "pasteboard_service.h"
#include "pasteboard_serv_ipc_interface_code.h"

using namespace OHOS::Security::PasteboardServ;
using namespace OHOS::MiscServices;

namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr int32_t OFFSET = 4;
constexpr size_t RANDNUM_ZERO = 0;
constexpr size_t RANDNUM_ONE = 1;
const std::u16string PASTEBOARDSERVICE_INTERFACE_TOKEN = u"ohos.miscservices.pasteboard.IPasteboardService";

uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

bool FuzzPasteboardService(const uint8_t *rawData, size_t size)
{
    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::make_shared<PasteboardService>()->OnRemoteRequest(code, data, reply, option);

    return true;
}

bool FuzzPasteboardServiceOnSetPasteData(const uint8_t *rawData, size_t size)
{
    PasteData pasteData;
    std::shared_ptr<PasteDataRecord> pasteDataRecord = std::make_shared<PasteDataRecord>();
    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    std::string str(reinterpret_cast<const char *>(rawData), size);
    switch (code) {
        case RANDNUM_ZERO:
            pasteData.AddTextRecord(str);
            break;
        case RANDNUM_ONE:
            pasteData.AddHtmlRecord(str);
            break;
        default:
            pasteData.AddUriRecord(Uri(str));
            break;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    std::vector<uint8_t> pasteDataTlv(0);
    pasteData.Encode(pasteDataTlv);
    data.WriteInt32(pasteDataTlv.size());
    data.WriteRawData(pasteDataTlv.data(), pasteDataTlv.size());

    CopyUriHandler copyHandler;
    pasteData.WriteUriFd(data, copyHandler);

    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::SET_PASTE_DATA), data, reply, option);

    return true;
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzPasteboardService(data, size);
    OHOS::FuzzPasteboardServiceOnSetPasteData(data, size);
    return 0;
}
