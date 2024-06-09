/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
bool DistributedModuleConfig::IsOn()
{
    if (GetDeviceNum() != 0) {
        Notify();
    }
    return status_;
}

void DistributedModuleConfig::Watch(Observer observer)
{
    observer_ = std::move(observer);
}

void DistributedModuleConfig::Notify()
{
    bool newStatus = GetEnabledStatus();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Notify, status:%{public}d, newStatus:%{public}d",
        status_, newStatus);
    if (newStatus != status_) {
        status_ = newStatus;
        if (observer_ != nullptr) {
            observer_(newStatus);
        }
    }
}

size_t DistributedModuleConfig::GetDeviceNum()
{
    auto networkIds = DMAdapter::GetInstance().GetNetworkIds();
    return networkIds.size();
}

bool DistributedModuleConfig::GetEnabledStatus()
{
    auto localNetworkId = DMAdapter::GetInstance().GetLocalNetworkId();
    if (!DevProfile::GetInstance().GetEnabledStatus(localNetworkId)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetLocalEnable false.");
        return false;
    }
    auto networkIds = DMAdapter::GetInstance().GetNetworkIds();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "device online nums: %{public}zu", networkIds.size());
    for (auto &id : networkIds) {
        if (DevProfile::GetInstance().GetEnabledStatus(id)) {
            return true;
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "remoteEnabledStatus is false.");
    return false;
}

void DistributedModuleConfig::Online(const std::string &device)
{
    DevProfile::GetInstance().SubscribeProfileEvent(device);
    Notify();
}

void DistributedModuleConfig::Offline(const std::string &device)
{
    DevProfile::GetInstance().UnSubscribeProfileEvent(device);
    Notify();
}

void DistributedModuleConfig::OnReady(const std::string &device)
{
    DevProfile::GetInstance().OnReady();
}

void DistributedModuleConfig::Init()
{
    DMAdapter::GetInstance().Register(this);
    GetDeviceNum();
}

void DistributedModuleConfig::DeInit()
{
    DMAdapter::GetInstance().Unregister(this);
}

} // namespace MiscServices
} // namespace OHOS