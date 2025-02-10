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

#include "pasteboard_common.h"

#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include <unistd.h>

#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
namespace PasteBoardCommon {
sptr<AppExecFwk::IBundleMgr> GetAppBundleManager(void)
{
    auto sysAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sysAbilityMgr == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, " Failed to get system ability manager.");
        return nullptr;
    }
    auto remoteObject = sysAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, " Failed to get bundle mgr service.");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
}

int32_t GetApiTargetVersionForSelf(void)
{
    static constexpr int32_t API_VERSION_MOD = 1000;
    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = GetAppBundleManager();
    if (bundleMgrProxy == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "Failed to get bundle manager proxy.");
        return 0;
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto flags = AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION;
    auto ret = bundleMgrProxy->GetBundleInfoForSelf(static_cast<int32_t>(flags), bundleInfo);
    if (ret != ERR_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "GetBundleInfoForSelf: bundleName get failed %{public}d.", ret);
        return 0;
    }
    auto targetApiVersion = bundleInfo.applicationInfo.apiTargetVersion % API_VERSION_MOD;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "Got target API version %{public}d.", targetApiVersion);
    return targetApiVersion;
}
} // namespace PasteBoardCommon
} // namespace MiscServices
} // namespace OHOS
