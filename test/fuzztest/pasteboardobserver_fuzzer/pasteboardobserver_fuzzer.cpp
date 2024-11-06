/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "pasteboardobserver_fuzzer.h"

#include "message_parcel.h"

#include <map>

#include "pasteboard_error.h"
#include "pasteboard_observer.h"
#include "pasteboard_serv_ipc_interface_code.h"

namespace OHOS {
using namespace OHOS::Security::PasteboardServ;
using namespace OHOS::MiscServices;
constexpr size_t THRESHOLD = 2;
const std::u16string PASTEBOARDSERVICE_INTERFACE_TOKEN = u"ohos.miscservices.pasteboard.IPasteboardService";

template<class T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

bool FuzzPasteboardObserver(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(uint32_t)) {
        return true;
    }
    uint32_t code = TypeCast<uint32_t>(rawData);
    MessageParcel data;
    data.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    MessageParcel reply;
    MessageOption option;
    std::make_shared<PasteboardObserver>()->OnRemoteRequest(
        static_cast<uint32_t>(code % THRESHOLD), data, reply, option);
    return true;
}

bool FuzzPasteboardObserverOnPasteboardChangedStub(const uint8_t *rawData, size_t size)
{
    MessageParcel data;
    data.WriteInterfaceToken(PasteboardObserverStub::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    std::make_shared<PasteboardObserver>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardObserverInterfaceCode::ON_PASTE_BOARD_CHANGE), data, reply, option);
    return true;
}

bool FuzzPasteboardObserverOnPasteboardEventStub(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }

    MessageParcel data;
    data.WriteInterfaceToken(PasteboardObserverStub::GetDescriptor());
    MessageParcel reply;
    MessageOption option;
    int pos = 0;
    int32_t status = TypeCast<int32_t>(rawData, &pos);
    data.WriteString(std::string(reinterpret_cast<const char*>(rawData + pos), size - pos));
    data.WriteInt32(status);
    std::make_shared<PasteboardObserver>()->OnRemoteRequest(
        static_cast<uint32_t>(PasteboardObserverInterfaceCode::ON_PASTE_BOARD_EVENT), data, reply, option);
    return true;
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzPasteboardObserver(data, size);
    OHOS::FuzzPasteboardObserverOnPasteboardChangedStub(data, size);
    OHOS::FuzzPasteboardObserverOnPasteboardEventStub(data, size);
    return 0;
}