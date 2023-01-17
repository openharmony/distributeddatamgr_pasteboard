/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "pasteboard_observer_proxy.h"

#include "errors.h"
#include "message_option.h"
#include "message_parcel.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
PasteboardObserverProxy::PasteboardObserverProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IPasteboardChangedObserver>(object)
{
}

void PasteboardObserverProxy::OnPasteboardChanged()
{
    MessageParcel data, reply;
    MessageOption option;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "write descriptor failed!");
        return;
    }

    int ret = Remote()->SendRequest(static_cast<int>(ON_PASTE_BOARD_CHANGE), data, reply, option);
    if (ret != ERR_OK) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "SendRequest is failed, error code: %{public}d", ret);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return;
}

void PasteboardObserverProxy::OnPasteboardEvent(std::string bundleName, int32_t status)
{
    MessageParcel data, reply;
    MessageOption option;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write descriptor failed!");
        return;
    }
    data.WriteString(bundleName);
    data.WriteInt32(status);
    int ret = Remote()->SendRequest(static_cast<int>(ON_PASTE_BOARD_EVENT), data, reply, option);
    if (ret != ERR_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "SendRequest is failed, error code: %{public}d", ret);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return;
}
} // namespace MiscServices
} // namespace OHOS
