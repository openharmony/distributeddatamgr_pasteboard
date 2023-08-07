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

#include "pasteboard_dialog.h"

#include "ability_connect_callback_stub.h"
#include "in_process_call_wrapper.h"
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

int32_t PasteBoardDialog::ShowDialog(const MessageInfo &message, const Cancel &cancel)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin, app:%{public}s, device:%{public}s", message.appName.c_str(),
        message.deviceType.c_str());
    auto abilityManager = GetAbilityManagerService();
    if (abilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get ability manager failed");
        return -1;
    }
    Want want;
    want.SetAction("");
    want.SetElementName(PASTEBOARD_DIALOG_APP, PASTEBOARD_DIALOG_ABILITY);
    want.SetParam("appName", message.appName);
    want.SetParam("deviceType", message.deviceType);

    std::lock_guard<std::mutex> lock(connectionLock_);
    connection_ = new DialogConnection(cancel);
    int32_t result = IN_PROCESS_CALL(abilityManager->ConnectAbility(want, connection_, nullptr));
    if (result != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "start pasteboard dialog failed, result:%{public}d", result);
        return -1;
    }
    connectNum_.fetch_add(1);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start pasteboard dialog success. connectNum = %{public}s",
        std::to_string(connectNum_).c_str());
    return 0;
}

int32_t PasteBoardDialog::ShowToast(const ToastMessageInfo &message)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin, fromApp:%{public}s, toApp:%{public}s",
        message.fromAppName.c_str(), message.toAppName.c_str());
    auto abilityManager = GetAbilityManagerService();
    if (abilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get ability manager failed");
        return -1;
    }
    Want want;
    want.SetAction("");
    want.SetElementName(PASTEBOARD_DIALOG_APP, PASTEBOARD_TOAST_ABILITY);
    want.SetParam("fromAppName", message.fromAppName);
    want.SetParam("toAppName", message.toAppName);

    std::lock_guard<std::mutex> lock(toastConnectionLock_);
    toastConnection_ = new DialogConnection(nullptr);
    int32_t result = IN_PROCESS_CALL(abilityManager->ConnectAbility(want, toastConnection_, nullptr));
    if (result != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "start pasteboard toast failed, result:%{public}d", result);
        return -1;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start pasteboard toast success.");
    return 0;
}

void PasteBoardDialog::CancelDialog()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin");
    auto abilityManager = GetAbilityManagerService();
    if (abilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get ability manager failed");
        return;
    }
    std::lock_guard<std::mutex> lock(connectionLock_);
    while (connectNum_ > 0) {
        int result = IN_PROCESS_CALL(abilityManager->DisconnectAbility(connection_));
        if (result != 0) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "cancel pasteboard dialog failed, result:%{public}d", result);
            return;
        }
        connectNum_ = connectNum_ > 0 ? connectNum_ - 1 : 0;
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "disconnect dialog ability:%{public}d, connectNum = %{public}s",
            result, std::to_string(connectNum_).c_str());
    }
}

void PasteBoardDialog::CancelToast()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin");
    auto abilityManager = GetAbilityManagerService();
    if (abilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get ability manager failed");
        return;
    }
    std::lock_guard<std::mutex> lock(toastConnectionLock_);
    int result = IN_PROCESS_CALL(abilityManager->DisconnectAbility(toastConnection_));
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "disconnect toast ability:%{public}d", result);
}

sptr<IAbilityManager> PasteBoardDialog::GetAbilityManagerService()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "failed to get samgr");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (!remoteObject) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "failed to get ability manager service");
        return nullptr;
    }
    return iface_cast<IAbilityManager>(remoteObject);
}
} // namespace OHOS::MiscServices
