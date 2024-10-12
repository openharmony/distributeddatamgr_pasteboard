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
using namespace std;

namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr int32_t OFFSET = 4;
const int32_t SHAREOPTIONSIZE = 3;
const std::u16string PASTEBOARDSERVICE_INTERFACE_TOKEN = u"ohos.miscservices.pasteboard.IPasteboardService";

uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

template<class T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

bool FuzzPasteboardService(const uint8_t *rawData, size_t size)
{
    uint32_t code = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    data.WriteInt32(size);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::make_shared<PasteboardService>()->OnRemoteRequest(code, data, reply, option);

    return true;
}

bool FuzzPasteboardServiceOnSetPasteData(const uint8_t *rawData, size_t size)
{
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    data.WriteInt32(size);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::SET_PASTE_DATA), data, reply, option);

    MessageParcel source;
    data.RewindRead(0);
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::HAS_PASTE_DATA), source, reply, option);
    data.RewindRead(0);
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_PASTE_DATA), source, reply, option);
    return true;
}

bool FuzzPasteboardServiceOnClearPasteData(const uint8_t *rawData, size_t size)
{
    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::CLEAR_ALL), data, reply, option);
    return true;
}

bool FuzzPasteOnIsRemoteData(const uint8_t *rawData, size_t size)
{
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    data.WriteInt32(size);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::IS_REMOTE_DATA), data, reply, option);
    data.RewindRead(0);
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_DATA_SOURCE), data, reply, option);
    data.RewindRead(0);
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::HAS_DATA_TYPE), data, reply, option);

    return true;
}

bool FuzzPasteOnSubscribeObserver(const uint8_t *rawData, size_t size)
{
    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    data.WriteUint32(ConvertToUint32(rawData));
    sptr<IRemoteObject> obj = new (std::nothrow) IPCObjectStub();
    data.WriteRemoteObject(obj);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::SUBSCRIBE_OBSERVER), data, reply, option);
    data.RewindRead(0);
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::UNSUBSCRIBE_OBSERVER), data, reply, option);
    data.RewindRead(0);
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::UNSUBSCRIBE_ALL_OBSERVER), data, reply, option);

    return true;
}

bool FuzzPasteOnSetGlobalShareOption(const uint8_t *rawData, size_t size)
{
    uint32_t tokenId = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    data.WriteInt32(1);
    data.WriteUint32(tokenId);
    data.WriteInt32(static_cast<int32_t>(tokenId % SHAREOPTIONSIZE));

    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::SET_GLOBAL_SHARE_OPTION), data, reply, option);
    return true;
}

bool FuzzPasteOnGetGlobalShareOption(const uint8_t *rawData, size_t size)
{
    uint32_t tokenId = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;

    std::vector<uint32_t> vec;
    vec.push_back(tokenId);
    tokenId = ConvertToUint32(rawData);
    vec.push_back(tokenId);

    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    data.WriteUInt32Vector(vec);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_GLOBAL_SHARE_OPTION), data, reply, option);
    data.RewindRead(0);
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::REMOVE_GLOBAL_SHARE_OPTION), data, reply, option);
    return true;
}

bool FuzzPasteOnSetAppShareOptions(const uint8_t *rawData, size_t size)
{
    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    int32_t shareOptions = TypeCast<int32_t>(rawData);
    data.WriteInt32(shareOptions);
    data.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::SET_APP_SHARE_OPTIONS), data, reply, option);
    return true;
}

bool FuzzPasteOnRemoveAppShareOptions(const uint8_t *rawData, size_t size)
{
    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::REMOVE_APP_SHARE_OPTIONS), data, reply, option);
    return true;
}

bool FuzzPasteOnPasteStart(const uint8_t *rawData, size_t size)
{
    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    data.WriteString(string(reinterpret_cast<const char*>(rawData), size));
    MessageParcel reply;
    MessageOption option;
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::PASTE_START), data, reply, option);
    return true;
}

bool FuzzPasteOnPasteComplete(const uint8_t *rawData, size_t size)
{
    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    int len = size >> 1;
    data.WriteString(string(reinterpret_cast<const char*>(rawData), len));
    data.WriteString(string(reinterpret_cast<const char*>(rawData + len), size - len));
    MessageParcel reply;
    MessageOption option;
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::PASTE_COMPLETE), data, reply, option);
    return true;
}

bool FuzzPasteOnRegisterClientDeathObserver(const uint8_t *rawData, size_t size)
{
    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    sptr<IRemoteObject> obj = new (std::nothrow) IPCObjectStub();
    data.WriteRemoteObject(obj);
    MessageParcel reply;
    MessageOption option;
    std::make_shared<PasteboardService>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardServiceInterfaceCode::REGISTER_CLIENT_DEATH_OBSERVER), data, reply, option);
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
    OHOS::FuzzPasteboardServiceOnClearPasteData(data, size);
    OHOS::FuzzPasteOnIsRemoteData(data, size);
    OHOS::FuzzPasteOnSubscribeObserver(data, size);
    OHOS::FuzzPasteOnSetGlobalShareOption(data, size);
    OHOS::FuzzPasteOnGetGlobalShareOption(data, size);
    OHOS::FuzzPasteOnSetAppShareOptions(data, size);
    OHOS::FuzzPasteOnRemoveAppShareOptions(data, size);
    OHOS::FuzzPasteOnPasteStart(data, size);
    OHOS::FuzzPasteOnRegisterClientDeathObserver(data, size);
    return 0;
}
