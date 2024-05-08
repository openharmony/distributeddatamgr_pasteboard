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

#include <thread>

#include "pasteboard_dialog.h"
#include "ability_connect_callback_stub.h"
#include "extension_manager_client.h"
#include "iservice_registry.h"
#include "pasteboard_hilog.h"
#include "system_ability_definition.h"

namespace OHOS::MiscServices {
using namespace OHOS::AAFwk;
class DialogConnection : public AAFwk::AbilityConnectionStub {
public:
    explicit DialogConnection(PasteBoardDialog::Cancel cancel) : cancel_(std::move(cancel))
    {
    }
    DialogConnection(const DialogConnection &) = delete;
    DialogConnection &operator=(const DialogConnection &) = delete;
    DialogConnection(DialogConnection &&) = delete;
    DialogConnection &operator=(DialogConnection &&) = delete;
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override;

private:
    PasteBoardDialog::Cancel cancel_;
};

void DialogConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "dialog ability connected");
}

void DialogConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "dialog ability disconnect");
    if (cancel_ != nullptr) {
        cancel_();
    }
}

PasteBoardDialog &PasteBoardDialog::GetInstance()
{
    static PasteBoardDialog instance;
    return instance;
}

int32_t PasteBoardDialog::ShowToast(const ToastMessageInfo &message)
{
    Want want;
    want.SetAction("");
    want.SetElementName(PASTEBOARD_DIALOG_APP, PASTEBOARD_TOAST_ABILITY);
    want.SetParam("appName", message.appName);

    std::lock_guard<std::mutex> lock(connectionLock_);
    connection_ = new DialogConnection(nullptr);
    int32_t result = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(want, connection_,
        nullptr, -1);
    if (result != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "start pasteboard toast failed, result:%{public}d", result);
        return -1;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start pasteboard toast success.");
    std::thread thread([this]() mutable {
        std::this_thread::sleep_for(std::chrono::milliseconds(SHOW_TOAST_TIME));
        CancelToast();
    });
    thread.detach();
    return 0;
}

void PasteBoardDialog::CancelToast()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin");
    std::lock_guard<std::mutex> lock(connectionLock_);
    int32_t result = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(connection_);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "disconnect toast ability:%{public}d", result);
}
} // namespace OHOS::MiscServices
