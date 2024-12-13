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

#include "ability_connect_callback_stub.h"
#include "ability_manager_proxy.h"
#include "in_process_call_wrapper.h"
#include "iservice_registry.h"
#include "pasteboard_dialog.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "system_ability_definition.h"

namespace OHOS::MiscServices {
using namespace OHOS::AAFwk;
class DialogConnection : public AAFwk::AbilityConnectionStub {
public:
    explicit DialogConnection(PasteBoardDialog::Cancel cancel) : cancel_(std::move(cancel)) {}
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
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin, app:%{public}s", message.appName.c_str());
    auto abilityManager = GetAbilityManagerService();
    if (abilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get ability manager failed");
        return static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR);
    }
    Want want;
    want.SetAction("");
    want.SetElementName(PASTEBOARD_DIALOG_APP, PASTEBOARD_TOAST_ABILITY);
    want.SetParam("appName", message.appName);

    std::lock_guard<std::mutex> lock(connectionLock_);
    connection_ = new DialogConnection(nullptr);
    int32_t result = IN_PROCESS_CALL(
        abilityManager->ConnectAbility(want, iface_cast<AAFwk::IAbilityConnection>(connection_), nullptr));
    if (result != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "start pasteboard toast failed, result:%{public}d", result);
        return static_cast<int32_t>(PasteboardError::TASK_PROCESSING);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start pasteboard toast success.");
    std::thread thread([this]() mutable {
        std::this_thread::sleep_for(std::chrono::milliseconds(SHOW_TOAST_TIME));
        CancelToast();
    });
    thread.detach();
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteBoardDialog::CancelToast()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin");
    auto abilityManager = GetAbilityManagerService();
    if (abilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get ability manager failed");
        return;
    }
    std::lock_guard<std::mutex> lock(connectionLock_);
    int result = IN_PROCESS_CALL(abilityManager->DisconnectAbility(connection_));
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "disconnect toast ability:%{public}d", result);
}

int32_t PasteBoardDialog::ShowProgress(const ProgressMessageInfo &message)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "begin, app:%{public}s", message.appName.c_str());
    auto abilityManager = GetAbilityManagerService();
    if (abilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get ability manager failed");
        return static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR);
    }
 
    Want want;
    want.SetElementName(PASTEBOARD_DIALOG_APP, PASTEBOARD_PROGRESS_ABILITY);
    want.SetAction(PASTEBOARD_PROGRESS_ABILITY);
    want.SetParam("appName", message.appName);
    want.SetParam("promptText", message.promptText);
    want.SetParam("remoteDeviceName", message.remoteDeviceName);
    want.SetParam("progressKey", message.progressKey);
    want.SetParam("signalKey", message.signalKey);
    want.SetParam("isRemote", message.isRemote);
    want.SetParam("windowId", message.windowId);
    if (message.callerToken != nullptr) {
        want.SetParam("tokenKey", message.callerToken);
    }
    int32_t result = IN_PROCESS_CALL(abilityManager->StartAbility(want));
    if (result != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "start pasteboard progress failed, result:%{public}d", result);
        return static_cast<int32_t>(PasteboardError::TASK_PROCESSING);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start pasteboard progress success.");
    return static_cast<int32_t>(PasteboardError::E_OK);
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
