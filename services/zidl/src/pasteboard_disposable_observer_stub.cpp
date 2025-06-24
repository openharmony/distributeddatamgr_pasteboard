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

#include "pasteboard_disposable_observer_stub.h"

#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_serv_ipc_interface_code.h"

using namespace OHOS::Security::PasteboardServ;
namespace OHOS {
namespace MiscServices {
PasteboardDisposableObserverStub::PasteboardDisposableObserverStub()
{
    memberFuncMap_[static_cast<uint32_t>(PasteboardDisposableInterfaceCode::ON_TEXT_RECEIVED)] =
        &PasteboardDisposableObserverStub::OnTextReceivedStub;
}

int32_t PasteboardDisposableObserverStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::u16string myDescriptor = PasteboardDisposableObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(myDescriptor == remoteDescriptor,
        static_cast<int32_t>(PasteboardError::CHECK_DESCRIPTOR_ERROR), PASTEBOARD_MODULE_SERVICE,
        "invalid descriptor, code=%{public}u", code);

    auto iter = memberFuncMap_.find(code);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(iter != memberFuncMap_.end(),
        static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR), PASTEBOARD_MODULE_SERVICE,
        "invalid code, code=%{public}u", code);
    auto memberFunc = iter->second;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(memberFunc != nullptr,
        static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR), PASTEBOARD_MODULE_SERVICE,
        "func is null, code=%{public}u", code);
    return (this->*memberFunc)(data, reply);
}

int32_t PasteboardDisposableObserverStub::OnTextReceivedStub(MessageParcel &data, MessageParcel &reply)
{
    std::string text = data.ReadString();
    int32_t errCode = 0;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(data.ReadInt32(errCode), ERR_INVALID_VALUE, PASTEBOARD_MODULE_CLIENT,
        "read errCode failed");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "ret=%{public}d, textLen=%{public}zu", errCode, text.length());
    OnTextReceived(text, errCode);
    return ERR_OK;
}
} // namespace MiscServices
} // namespace OHOS
