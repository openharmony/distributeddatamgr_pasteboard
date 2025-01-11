/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "pasteboard_error.h"
#include "pasteboard_signal_stub.h"

namespace OHOS {
namespace MiscServices {
PasteboardSignalStub::PasteboardSignalStub()
{
    memberFuncMap_[static_cast<uint32_t>(PasteboardSignalStub::GET_PROGRESS_SIGNAL)] =
        &PasteboardSignalStub::OnGetProgressSignal;
}

int32_t PasteboardSignalStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescriptor = PasteboardSignalStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "end##descriptor checked fail");
        return static_cast<int32_t>(PasteboardError::CHECK_DESCRIPTOR_ERROR);
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE,
        "CallingPid = %{public}d, CallingUid = %{public}d, code = %{public}u", pid, uid, code);
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end##ret = %{public}d", ret);
    return ret;
}

int32_t PasteboardSignalStub::OnGetProgressSignal(MessageParcel &data, MessageParcel &reply)
{
    std::string signalValue = data.ReadString();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OnGetProgressSignal signalValue is %{public}s", signalValue.c_str());
    HandleProgressSignalValue(signalValue);
    return ERR_OK;
}

PasteboardSignalStub::~PasteboardSignalStub()
{
    memberFuncMap_.clear();
}
} // namespace MiscServices
} // namespace OHOS