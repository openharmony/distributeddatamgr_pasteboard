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

#include "pasteboard_permission.h"


namespace OHOS {
namespace MiscServices {
namespace {
const std::int32_t USER_ID_CHANGE_VALUE = 1000000;
}
std::mutex PasteboardPermission::instanceLock_;
sptr<PasteboardPermission> PasteboardPermission::instance_;
sptr<AppExecFwk::IBundleMgr> PasteboardPermission::bundleMgrProxy_;

PasteboardPermission::PasteboardPermission()
{
}

PasteboardPermission::~PasteboardPermission()
{
}

sptr<PasteboardPermission> PasteboardPermission::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> autoLock(instanceLock_);
        if (instance_ == nullptr) {
            instance_ = new PasteboardPermission;
        }
    }
    return instance_;
}

bool PasteboardPermission::CheckSelfPermission(std::string permName)
{
    return true;
}

bool PasteboardPermission::CheckCallingPermission(int32_t uid, std::string permName)
{
    if (bundleMgrProxy_ == nullptr) {
        bundleMgrProxy_ = GetBundleManager();
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "get bundle mgr");
    }

    if (bundleMgrProxy_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "redo get bundle mgr failed");
        return false;
    }
    std::string bundleName;
    auto ret = bundleMgrProxy_->GetBundleNameForUid(uid, bundleName);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "get bundle name failed");
        return false;
    }
    auto userId = uid / USER_ID_CHANGE_VALUE;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "VerifyPermission bundleName %{public}s, permission %{public}s",
		bundleName.c_str(), permName.c_str());
    return MockPermission::VerifyPermission(bundleName, permName, userId);
}

sptr<AppExecFwk::IBundleMgr> PasteboardPermission::GetBundleManager()
{
    if (bundleMgrProxy_ == nullptr) {
        sptr<ISystemAbilityManager> systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (systemManager != nullptr) {
            bundleMgrProxy_ =
                iface_cast<AppExecFwk::IBundleMgr>(systemManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID));
        } else {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "fail to get SAMGR");
        }
    }
    return bundleMgrProxy_;
}
} // namespace MiscServices
} // namespace OHOS
