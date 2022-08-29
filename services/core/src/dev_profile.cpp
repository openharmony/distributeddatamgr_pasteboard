/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "dev_profile.h"

#include <thread>

#include "device_manager.h"
#include "distributed_device_profile_client.h"
#include "distributed_module_config.h"
#include "nlohmann/json.hpp"
#include "pasteboard_hilog_wreapper.h"
#include "service_characteristic_profile.h"
#include "subscribe_info.h"
namespace OHOS {
namespace MiscServices {
using namespace DeviceProfile;
using namespace DistributedHardware;
constexpr int32_t HANDLE_OK = 0;
constexpr int32_t HANDLE_ERROR = -1;
constexpr const char *SERVICE_ID = "pasteboard_service";
constexpr const char *PKG_NAME = "pasteboard_service";

void DevProfile::PasteboardProfileEventCallback::OnSyncCompleted(const SyncResult &syncResults)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnSyncCompleted.");
}

void DevProfile::PasteboardProfileEventCallback::OnProfileChanged(const ProfileChangeNotification &changeNotification)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnProfileChanged start.");
    GetInstance().ProfileChanged();
}

void DevProfile::ProfileChanged()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ProfileChanged start.");
    DistributedModuleConfig::Notify();
}

DevProfile::DevProfile()
{
}

DevProfile &DevProfile::GetInstance()
{
    static DevProfile instance;
    return instance;
}

int32_t DevProfile::Init(const std::string &enabledStatus)
{
    ServiceCharacteristicProfile profile;
    profile.SetServiceId(SERVICE_ID);
    profile.SetServiceType(SERVICE_ID);
    nlohmann::json jsonObject;
    jsonObject["distributed_pasteboard_enabled"] = enabledStatus;
    profile.SetCharacteristicProfileJson(jsonObject.dump());
    int32_t errNo = DistributedDeviceProfileClient::GetInstance().PutDeviceProfile(profile);
    if (errNo == HANDLE_OK) {
        SyncDeviceProfile();
    }
    return errNo;
}

void DevProfile::PutDeviceProfile(const std::string &enabledStatus)
{
    PASTEBOARD_HILOGI(
        PASTEBOARD_MODULE_SERVICE, "PutDeviceProfile start, dpbEnableIn = %{public}s.", enabledStatus.c_str());
    int32_t errNo = Init(enabledStatus);
    if (errNo == HANDLE_OK) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PutDeviceProfile success.");
        return;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PutDeviceProfile failed.");
    std::thread th = std::thread([enabledStatus, this]() {
        constexpr int RETRY_TIMES = 300;
        int i = 0;
        int32_t errNo = HANDLE_ERROR;
        while (i++ < RETRY_TIMES) {
            errNo = Init(enabledStatus);
            if (errNo == HANDLE_OK) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        PASTEBOARD_HILOGI(
            PASTEBOARD_MODULE_SERVICE, "PutDeviceProfile exit now: %{public}d times, errNo: %{public}d", i, errNo);
    });
    th.detach();
}

void DevProfile::GetDeviceProfile(const std::string &deviceId, std::string &enabledStatus)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetDeviceProfile start.");
    ServiceCharacteristicProfile profile;
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetDeviceProfile(deviceId, SERVICE_ID, profile);
    if (ret != HANDLE_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "deviceId = %{public}s GetDeviceProfile faild.", deviceId.c_str());
        return;
    }
    const auto &jsonData = profile.GetCharacteristicProfileJson();
    nlohmann::json jsonObject = nlohmann::json::parse(jsonData, nullptr, false);
    if (jsonObject.is_discarded()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "json parse faild.");
        return;
    }

    enabledStatus = jsonObject["distributed_pasteboard_enabled"];
    PASTEBOARD_HILOGI(
        PASTEBOARD_MODULE_SERVICE, "GetDeviceProfile success, dpbEnableIn = %{public}s.", enabledStatus.c_str());
}

void DevProfile::SubscribeProfileEvent(const std::string &deviceId)
{
    PASTEBOARD_HILOGD(
        PASTEBOARD_MODULE_SERVICE, "SubscribeProfileEvent start, deviceId = %{public}s", deviceId.c_str());
    std::list<std::string> serviceIds = { SERVICE_ID };
    ExtraInfo extraInfo;
    extraInfo["deviceId"] = deviceId;
    extraInfo["serviceIds"] = serviceIds;

    SubscribeInfo subscribeInfo;
    subscribeInfo.profileEvent = ProfileEvent::EVENT_PROFILE_CHANGED;
    subscribeInfo.extraInfo = std::move(extraInfo);
    int32_t errCode =
        DistributedDeviceProfileClient::GetInstance().SubscribeProfileEvent(subscribeInfo, callbackMap_[deviceId]);

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SubscribeProfileEvent result, errCode = %{public}d.", errCode);
}

void DevProfile::RegisterProfileCallback(const std::string &deviceId)
{
    if (deviceId.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "deviceId is empty.");
        return;
    }
    {
        std::lock_guard<std::mutex> mutexLock(callbackMapMutex_);
        if (callbackMap_.find(deviceId) != callbackMap_.end()) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "deviceId = %{public}s already exists.", deviceId.c_str());
            return;
        }
        auto pasteboardProfileEventCallback = std::make_shared<PasteboardProfileEventCallback>();
        callbackMap_[deviceId] = pasteboardProfileEventCallback;
    }
    SubscribeProfileEvent(deviceId);
}

void DevProfile::UnSubscribeProfileEvent(const std::string &deviceId)
{
    PASTEBOARD_HILOGD(
        PASTEBOARD_MODULE_SERVICE, "UnSubscribeProfileEvent start, deviceId = %{public}s", deviceId.c_str());
    ProfileEvent profileEvent = EVENT_PROFILE_CHANGED;
    int32_t errCode =
        DistributedDeviceProfileClient::GetInstance().UnsubscribeProfileEvent(profileEvent, callbackMap_[deviceId]);

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "UnsubscribeProfileEvent result, errCode = %{public}d.", errCode);
}

void DevProfile::UnRegisterProfileCallback(const std::string &deviceId)
{
    if (deviceId.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "deviceId is empty.");
        return;
    }
    UnSubscribeProfileEvent(deviceId);

    std::lock_guard<std::mutex> mutexLock(callbackMapMutex_);
    if (callbackMap_.find(deviceId) != callbackMap_.end()) {
        callbackMap_.erase(deviceId);
    }
}

void DevProfile::SyncDeviceProfile()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SyncDeviceProfile start.");
    std::vector<DmDeviceInfo> devList;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(PKG_NAME, "", devList);
    if (ret != HANDLE_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetTrustedDeviceList failed!");
        return;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "devList = %{public}d.", static_cast<uint32_t>(devList.size()));
    if (devList.empty()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no device online!");
        return;
    }

    auto pasteboardSyncCallback = std::make_shared<PasteboardProfileEventCallback>();
    SyncOptions syncOptions;
    for (auto &devInfo : devList) {
        syncOptions.AddDevice(devInfo.deviceId);
    }
    syncOptions.SetSyncMode(SyncMode::PUSH_PULL);
    int32_t errCode =
        DistributedDeviceProfileClient::GetInstance().SyncDeviceProfile(syncOptions, pasteboardSyncCallback);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SyncDeviceProfile, ret = %{public}d.", errCode);
}

void DevProfile::UnRegisterAllProfileCallback()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "UnRegisterAllProfileCallback start.");
    std::lock_guard<std::mutex> mutexLock(callbackMapMutex_);
    for (auto const &callback : callbackMap_) {
        UnSubscribeProfileEvent(callback.first);
        callbackMap_.erase(callback.first);
    }
}
} // namespace MiscServices
} // namespace OHOS
