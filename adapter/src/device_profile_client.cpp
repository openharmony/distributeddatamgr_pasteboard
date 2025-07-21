/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "device_profile_client.h"

#include <chrono>

#include "distributed_device_profile_errors.h"
#include "distributed_device_profile_proxy.h"
#include "iservice_registry.h"
#include "pasteboard_hilog.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DistributedDeviceProfile {
using namespace OHOS::MiscServices;

constexpr int32_t LOAD_SA_TIMEOUT_MS = 10000;

void DeviceProfileLoadCb::OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    DeviceProfileClient::GetInstance().LoadSystemAbilitySuccess(remoteObject);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "on load system ability success");
}

void DeviceProfileLoadCb::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    DeviceProfileClient::GetInstance().LoadSystemAbilityFail();
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "on load system ability failed");
}

DeviceProfileClient &DeviceProfileClient::GetInstance()
{
    static DeviceProfileClient instance;
    return instance;
}

sptr<IDistributedDeviceProfile> DeviceProfileClient::GetDeviceProfileService()
{
    {
        std::lock_guard lock(serviceLock_);
        if (dpProxy_ != nullptr) {
            return dpProxy_;
        }

        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(samgr != nullptr, nullptr, PASTEBOARD_MODULE_COMMON,
            "get samgr failed");

        auto object = samgr->CheckSystemAbility(DISTRIBUTED_DEVICE_PROFILE_SA_ID);
        if (object != nullptr) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "get dp service success");
            dpProxy_ = new DistributedDeviceProfileProxy(object);
            return dpProxy_;
        }
    }

    PASTEBOARD_HILOGW(PASTEBOARD_MODULE_COMMON, "remoteObject is null");
    bool loadSucc = LoadDeviceProfileService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(loadSucc, nullptr, PASTEBOARD_MODULE_COMMON,
        "load dp service failed");

    std::lock_guard lock(serviceLock_);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(dpProxy_ != nullptr, nullptr, PASTEBOARD_MODULE_COMMON,
        "load dp service failed");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "load dp service success");
    return dpProxy_;
}

sptr<IDistributedDeviceProfile> DeviceProfileClient::LoadDeviceProfileService()
{
    sptr<DeviceProfileLoadCb> loadCallback = sptr<DeviceProfileLoadCb>::MakeSptr();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(loadCallback != nullptr, nullptr, PASTEBOARD_MODULE_COMMON,
        "loadCallback is null");

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(samgr != nullptr, nullptr, PASTEBOARD_MODULE_COMMON,
        "get samgr failed");

    int32_t ret = samgr->LoadSystemAbility(DISTRIBUTED_DEVICE_PROFILE_SA_ID, loadCallback);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == ERR_OK, nullptr, PASTEBOARD_MODULE_COMMON,
        "load dp service failed, ret=%{public}d", ret);

    std::unique_lock lock(serviceLock_);
    auto waitStatus = proxyConVar_.wait_for(lock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
        [this]() { return dpProxy_ != nullptr; });
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(waitStatus && dpProxy_ != nullptr, nullptr, PASTEBOARD_MODULE_COMMON,
        "load dp service timeout");
    
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "load dp service success");
    return dpProxy_;
}

void DeviceProfileClient::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    PASTEBOARD_CHECK_AND_RETURN_LOGE(remoteObject != nullptr, PASTEBOARD_MODULE_COMMON, "remoteObject is null");

    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "enter");
    std::lock_guard lock(serviceLock_);
    dpProxy_ = new DistributedDeviceProfileProxy(remoteObject);
    proxyConVar_.notify_one();
}

void DeviceProfileClient::LoadSystemAbilityFail()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "enter");
    std::lock_guard lock(serviceLock_);
    dpProxy_ = nullptr;
    proxyConVar_.notify_one();
}

void DeviceProfileClient::ClearDeviceProfileService()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "enter");
    std::lock_guard lock(serviceLock_);
    dpProxy_ = nullptr;
}

void DeviceProfileClient::SendSubscribeInfos()
{
    auto dpService = GetDeviceProfileService();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(dpService != nullptr, PASTEBOARD_MODULE_COMMON, "get dp service failed");
    std::lock_guard lock(subscribeLock_);
    PASTEBOARD_CHECK_AND_RETURN_LOGI(!subscribeInfos_.empty(), PASTEBOARD_MODULE_COMMON, "no subscribe info");
    dpService->SendSubscribeInfos(subscribeInfos_);
}

int32_t DeviceProfileClient::PutCharacteristicProfile(const CharacteristicProfile &characteristicProfile)
{
    auto dpService = GetDeviceProfileService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(dpService != nullptr, DP_GET_SERVICE_FAILED, PASTEBOARD_MODULE_COMMON,
        "get dp service failed");
    return dpService->PutCharacteristicProfile(characteristicProfile);
}

int32_t DeviceProfileClient::GetCharacteristicProfile(const std::string &deviceId, const std::string &serviceName,
    const std::string &characteristicId, CharacteristicProfile &characteristicProfile)
{
    auto dpService = GetDeviceProfileService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(dpService != nullptr, DP_GET_SERVICE_FAILED, PASTEBOARD_MODULE_COMMON,
        "get dp service failed");
    return dpService->GetCharacteristicProfile(deviceId, serviceName, characteristicId, characteristicProfile);
}

int32_t DeviceProfileClient::SubscribeDeviceProfile(const SubscribeInfo &subscribeInfo)
{
    auto dpService = GetDeviceProfileService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(dpService != nullptr, DP_GET_SERVICE_FAILED, PASTEBOARD_MODULE_COMMON,
        "get dp service failed");
    {
        std::lock_guard lock(subscribeLock_);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(subscribeInfos_.size() < MAX_SUBSCRIBE_INFO_SIZE,
            DP_EXCEED_MAX_SIZE_FAIL, PASTEBOARD_MODULE_COMMON,
            "subscribe info size=%{public}zu", subscribeInfos_.size());

        std::string subscribeTag =
            subscribeInfo.GetSubscribeKey() + SEPARATOR + std::to_string(subscribeInfo.GetSaId());
        subscribeInfos_[subscribeTag] = subscribeInfo;
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "subscribe info size=%{public}zu", subscribeInfos_.size());
    }
    return dpService->SubscribeDeviceProfile(subscribeInfo);
}

int32_t DeviceProfileClient::UnSubscribeDeviceProfile(const SubscribeInfo &subscribeInfo)
{
    auto dpService = GetDeviceProfileService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(dpService != nullptr, DP_GET_SERVICE_FAILED, PASTEBOARD_MODULE_COMMON,
        "get dp service failed");
    {
        std::lock_guard lock(subscribeLock_);
        std::string subscribeTag =
            subscribeInfo.GetSubscribeKey() + SEPARATOR + std::to_string(subscribeInfo.GetSaId());
        subscribeInfos_.erase(subscribeTag);
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "subscribe info size=%{public}zu", subscribeInfos_.size());
    }
    return dpService->UnSubscribeDeviceProfile(subscribeInfo);
}
} // namespace DistributedDeviceProfile
} // namespace OHOS
