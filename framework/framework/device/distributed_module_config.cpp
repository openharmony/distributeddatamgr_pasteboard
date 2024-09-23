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
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include <thread>

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
    auto status = GetEnabledStatus();
    if (status == static_cast<int32_t>(PasteboardError::DP_LOAD_SERVICE_ERROR)) {
        if (!retrying_.exchange(true)) {
            GetRetryTask();
        }
        return;
    }
    bool newStatus = (status == static_cast<int32_t>(PasteboardError::E_OK));
    if (newStatus != status_) {
        status_ = newStatus;
        if (observer_ != nullptr) {
            observer_(newStatus);
        }
    }
}

void DistributedModuleConfig::GetRetryTask()
{
    std::thread remover([this]() {
        retrying_.store(true);
        uint32_t retry = 0;
        auto status = static_cast<int32_t>(PasteboardError::REMOTE_TASK_ERROR);
        while (retry < RETRY_TIMES) {
            ++retry;
            status = GetEnabledStatus();
            if (status == static_cast<int32_t>(PasteboardError::DP_LOAD_SERVICE_ERROR)) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "dp load err, retry:%{public}d, status_:%{public}d"
                    "newStatus:%{public}d", retry, status_, status);
                std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL));
                continue;
            }
            break;
        }
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Retry end. count:%{public}d, status_:%{public}d"
            "newStatus:%{public}d", retry, status_, status);
        bool newStatus = (status == static_cast<int32_t>(PasteboardError::E_OK));
        if (newStatus != status_) {
            status_ = newStatus;
            if (observer_ != nullptr) {
                observer_(newStatus);
            }
        }
        retrying_.store(false);
    });
    remover.detach();
}

size_t DistributedModuleConfig::GetDeviceNum()
{
    auto networkIds = DMAdapter::GetInstance().GetNetworkIds();
    return networkIds.size();
}

int32_t DistributedModuleConfig::GetEnabledStatus()
{
    auto localNetworkId = DMAdapter::GetInstance().GetLocalNetworkId();
    auto status = DevProfile::GetInstance().GetEnabledStatus(localNetworkId);
    if (status.first != static_cast<int32_t>(PasteboardError::E_OK) || status.second != SUPPORT_STATUS) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetLocalEnable false, status:%{public}d", status.first);
        return status.first;
    }
    auto networkIds = DMAdapter::GetInstance().GetNetworkIds();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "device online nums: %{public}zu", networkIds.size());
    for (auto &id : networkIds) {
        auto res = DevProfile::GetInstance().GetEnabledStatus(id);
        if (res.first == static_cast<int32_t>(PasteboardError::E_OK) && res.second == SUPPORT_STATUS) {
            return static_cast<int32_t>(PasteboardError::E_OK);
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "remoteEnabledStatus is false.");
    return static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR);
}

void DistributedModuleConfig::Online(const std::string &device)
{
    std::this_thread::sleep_for(std::chrono::milliseconds((int32_t(rand() % (RANDOM_MAX - RANDOM_MIN)))));
    DevProfile::GetInstance().SubscribeProfileEvent(device);
    Notify();
}

void DistributedModuleConfig::Offline(const std::string &device)
{
    std::this_thread::sleep_for(std::chrono::milliseconds((int32_t(rand() % (RANDOM_MAX - RANDOM_MIN)))));
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
    DevProfile::GetInstance().Watch([this](bool isEnable)-> void {
        Notify();
    });
}

void DistributedModuleConfig::DeInit()
{
    DMAdapter::GetInstance().Unregister(this);
}

} // namespace MiscServices
} // namespace OHOS