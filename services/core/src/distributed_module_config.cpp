/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "distributed_module_config.h"

#include "dev_profile.h"
#include "device_manager.h"
#include "dm_device_info.h"
#include "pasteboard_hilog_wreapper.h"

namespace OHOS {
namespace MiscServices {
constexpr const char *PKG_NAME = "pasteboard_service";
using namespace DistributedHardware;
bool DistributedModuleConfig::isOn_ = false;
DistributedModuleConfig::Observer DistributedModuleConfig::observer_ = nullptr;
bool DistributedModuleConfig::IsOn()
{
    isOn_ = IsServiceOn();
    return isOn_;
}

void DistributedModuleConfig::Watch(Observer observer)
{
    observer_ = std::move(observer);
}

void DistributedModuleConfig::Notify()
{
    bool isOn = IsServiceOn();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Notify, isOn = %{public}d", isOn);
    if (isOn != isOn_) {
        isOn_ = isOn;
        if (observer_ != nullptr) {
            observer_(isOn);
        }
    }
}

bool DistributedModuleConfig::IsServiceOn()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "IsServiceOn start.");

    std::string localEnabledStatus = "false";
    DevProfile::GetInstance().GetDeviceProfile("", localEnabledStatus);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "localEnabledStatus = %{public}s.", localEnabledStatus.c_str());
    if (localEnabledStatus == "false") {
        return false;
    }

    std::vector<DmDeviceInfo> devList;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(PKG_NAME, "", devList);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetTrustedDeviceList failed!");
        return false;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "devList = %{public}d.", static_cast<uint32_t>(devList.size()));
    if (devList.empty()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no device online!");
        return false;
    }
    std::string externalEnabledStatus = "false";
    for (auto const &devInfo : devList) {
        DevProfile::GetInstance().GetDeviceProfile(devInfo.deviceId, externalEnabledStatus);
        if (externalEnabledStatus == "true") {
            return true;
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "externalEnabledStatus = %{public}s.", externalEnabledStatus.c_str());
    return false;
}
} // namespace MiscServices
} // namespace OHOS