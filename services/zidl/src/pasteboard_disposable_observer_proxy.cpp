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

#include "pasteboard_disposable_observer_proxy.h"
#include "errors.h"
#include "message_option.h"
#include "message_parcel.h"
#include "pasteboard_hilog.h"
#include "pasteboard_serv_ipc_interface_code.h"

using namespace OHOS::Security::PasteboardServ;
namespace OHOS {
namespace MiscServices {
PasteboardDisposableObserverProxy::PasteboardDisposableObserverProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IPasteboardDisposableObserver>(object)
{
}

void PasteboardDisposableObserverProxy::OnTextReceived(const std::string &text, int32_t errCode)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "ret=%{public}d, textLen=%{public}zu", errCode, text.length());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    PASTEBOARD_CHECK_AND_RETURN_LOGE(data.WriteInterfaceToken(GetDescriptor()), PASTEBOARD_MODULE_SERVICE,
        "write descriptor failed");
    PASTEBOARD_CHECK_AND_RETURN_LOGE(data.WriteString(text), PASTEBOARD_MODULE_SERVICE, "write text failed");
    PASTEBOARD_CHECK_AND_RETURN_LOGE(data.WriteInt32(errCode), PASTEBOARD_MODULE_SERVICE, "write errCode failed");
    int32_t ret = Remote()->SendRequest(static_cast<uint32_t>(PasteboardDisposableInterfaceCode::ON_TEXT_RECEIVED),
        data, reply, option);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(ret == ERR_OK, PASTEBOARD_MODULE_SERVICE,
        "send request failed, ret=%{public}d", ret);
}
} // namespace MiscServices
} // namespace OHOS
