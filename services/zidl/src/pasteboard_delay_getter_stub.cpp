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

#include "ipc_skeleton.h"
#include "pasteboard_delay_getter_stub.h"
#include "pasteboard_error.h"
#include "paste_uri_handler.h"

namespace OHOS {
namespace MiscServices {
const PasteboardDelayGetterStub::Handler PasteboardDelayGetterStub::HANDLERS[TRANS_BUTT] = {
    &PasteboardDelayGetterStub::OnGetPasteData,
    &PasteboardDelayGetterStub::OnGetUnifiedData,
};

int PasteboardDelayGetterStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "code:%{public}u, callingPid:%{public}d", code,
                      IPCSkeleton::GetCallingPid());
    std::u16string localDescriptor = PasteboardDelayGetterStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (remoteDescriptor != localDescriptor) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "remote descriptor is not equal to local descriptor");
        return -1;
    }
    if (TRANS_HEAD > code || code >= TRANS_BUTT || HANDLERS[code] == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "not support code:%{public}u, max:%{public}d", code, TRANS_BUTT);
        return -1;
    }
    return (this->*HANDLERS[code])(data, reply);
}

int32_t PasteboardDelayGetterStub::OnGetPasteData(MessageParcel &data, MessageParcel &reply)
{
    PasteData pasteData;
    std::string dataType = data.ReadString();
    GetPasteData(dataType, pasteData);
    std::vector<uint8_t> pasteDataTlv(0);
    bool ret = pasteData.Encode(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "fail encode paste data");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(pasteDataTlv.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "fail write data size");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteRawData(pasteDataTlv.data(), pasteDataTlv.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "fail  write raw data");
        return ERR_INVALID_VALUE;
    }
    PasteUriHandler pasteUriHandler;
    if (!pasteData.WriteUriFd(reply, pasteUriHandler, false)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "fail  write uri fd");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t PasteboardDelayGetterStub::OnGetUnifiedData(MessageParcel &data, MessageParcel &reply)
{
    return ERR_OK;
}
} // namespace MiscServices
} // namespace OHOS