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

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
sptr<AppExecFwk::IBundleMgr> PasteBoardCommon::GetAppBundleManager(void)
{
    auto sysAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        sysAbilityMgr != nullptr, nullptr, PASTEBOARD_MODULE_SERVICE, "Failed to get system ability manager.");
    auto remoteObject = sysAbilityMgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        remoteObject != nullptr, nullptr, PASTEBOARD_MODULE_SERVICE, "Failed to get bundle mgr service.");
    return iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
}

int32_t PasteBoardCommon::GetApiTargetVersionForSelf(void)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGI(
        apiTargetVersion_ <= 0, apiTargetVersion_, PASTEBOARD_MODULE_SERVICE, "apiTargetVersion valid.");
    static constexpr int32_t API_VERSION_MOD = 1000;
    sptr<AppExecFwk::IBundleMgr> bundleMgrProxy = GetAppBundleManager();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGI(
        bundleMgrProxy != nullptr, 0, PASTEBOARD_MODULE_SERVICE, "Failed to get bundle manager proxy.");
    AppExecFwk::BundleInfo bundleInfo;
    auto flags = AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION;
    auto ret = bundleMgrProxy->GetBundleInfoForSelf(static_cast<int32_t>(flags), bundleInfo);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGI(
        ret == ERR_OK, 0, PASTEBOARD_MODULE_SERVICE, "GetBundleInfoForSelf: bundleName get failed %{public}d.", ret);
    int32_t targetApiVersion = bundleInfo.applicationInfo.apiTargetVersion % API_VERSION_MOD;
    apiTargetVersion_ = targetApiVersion;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_COMMON, "Got target API version %{public}d.", targetApiVersion);
    return targetApiVersion;
}
} // namespace MiscServices
} // namespace OHOS
