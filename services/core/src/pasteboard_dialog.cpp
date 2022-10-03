/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "iservice_registry.h"
#include "pasteboard_hilog_wreapper.h"
#include "system_ability_definition.h"
namespace OHOS::MiscServices {
PasteBoardDialog &PasteBoardDialog::GetInstance()
{
    static PasteBoardDialog instance;
    return instance;
}

int32_t PasteBoardDialog::ShowDialog(const MessageInfo &message, const Cancel &cancel)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "begin");
    auto abilityManager = GetAbilityManagerService();
    if (abilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get ability manager failed");
        return -1;
    }
    AAFwk::Want want;
    want.SetAction("");
    want.SetElementName(PASTEBOARD_DIALOG_APP, PASTEBOARD_DIALOG_ABILITY);
    want.SetParam("appName", message.appName);
    want.SetParam("deviceType", message.deviceType);
    int32_t result = abilityManager->StartAbility(want);
    if (result != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "start pasteboard dialog failed, result:%{public}d", result);
        return -1;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start pasteboard dialog success.");
    return 0;
}

sptr<AAFwk::IAbilityManager> PasteBoardDialog::GetAbilityManagerService()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "begin");
    auto systemAbilityManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "failed to get samgr");
        return nullptr;
    }

    sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (!remoteObject) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "failed to get ability manager service");
        return nullptr;
    }
    return iface_cast<AAFwk::IAbilityManager>(remoteObject);
}
} // namespace OHOS::MiscServicesNapi
