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
#include "dev_manager.h"

#include <thread>
#include <nlohmann/json.hpp>

#include "dev_profile.h"
#include "distributed_module_config.h"
#include "pasteboard_hilog.h"
#ifdef PB_DEVICE_MANAGER_ENABLE
#include "device_manager.h"
#include "device_manager_callback.h"
#endif
namespace OHOS {
namespace MiscServices {
constexpr const char *PKG_NAME = "pasteboard_service";
constexpr int32_t DM_OK = 0;
constexpr const int32_t DELAY_TIME = 200;
constexpr const uint32_t FIRST_VERSION = 4;
static constexpr int32_t OH_OS_TYPE_VERSION = 10;
static constexpr const char *DEVICE_OS_TYPE = "OS_TYPE";
#ifdef PB_DEVICE_MANAGER_ENABLE
using namespace OHOS::DistributedHardware;
using json = nlohmann::json;
class PasteboardDevStateCallback : public DistributedHardware::DeviceStateCallback {
public:
    void OnDeviceOnline(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override;
    void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override;
    void OnDeviceReady(const DmDeviceInfo &deviceInfo) override;
};

class PasteboardDmInitCallback : public DistributedHardware::DmInitCallback {
public:
    void OnRemoteDied() override;
};

void PasteboardDevStateCallback::OnDeviceOnline(const DmDeviceInfo &deviceInfo)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    int32_t osType = 0;
    if (!DevManager::GetInstance().ConvertOsType(deviceInfo.extraData, DEVICE_OS_TYPE, osType)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get os type fail");
        return;
    }
    if (osType != OH_OS_TYPE_VERSION) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "not oh device");
        return;
    }
    DevManager::GetInstance().Online(deviceInfo.networkId);
}

void PasteboardDevStateCallback::OnDeviceOffline(const DmDeviceInfo &deviceInfo)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    int32_t osType = 0;
    if (!DevManager::GetInstance().ConvertOsType(deviceInfo.extraData, DEVICE_OS_TYPE, osType)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get os type fail");
        return;
    }
    if (osType != OH_OS_TYPE_VERSION) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "not oh device");
        return;
    }
    DevManager::GetInstance().Offline(deviceInfo.networkId);
}

void PasteboardDevStateCallback::OnDeviceChanged(const DmDeviceInfo &deviceInfo)
{
}

void PasteboardDevStateCallback::OnDeviceReady(const DmDeviceInfo &deviceInfo)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    int32_t osType = 0;
    if (!DevManager::GetInstance().ConvertOsType(deviceInfo.extraData, DEVICE_OS_TYPE, osType)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get os type fail");
        return;
    }
    if (osType != OH_OS_TYPE_VERSION) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "not oh device");
        return;
    }
    (void)deviceInfo;
    DevManager::GetInstance().OnReady();
}

void PasteboardDmInitCallback::OnRemoteDied()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "dm device manager died, init it again");
    DevManager::GetInstance().Init();
}
#endif

DevManager::DevManager()
{
}

void DevManager::UnregisterDevCallback()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start");
    auto &deviceManager = DeviceManager::GetInstance();
    deviceManager.UnRegisterDevStateCallback(PKG_NAME);
    deviceManager.UnInitDeviceManager(PKG_NAME);
    DevProfile::GetInstance().UnsubscribeAllProfileEvents();
}

int32_t DevManager::Init()
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start");
    RetryInBlocking([]() -> bool {
        auto initCallback = std::make_shared<PasteboardDmInitCallback>();
        int32_t errNo = DeviceManager::GetInstance().InitDeviceManager(PKG_NAME, initCallback);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "InitDeviceManager ret %{public}d", errNo);
        return errNo == DM_OK;
    });
    RetryInBlocking([]() -> bool {
        auto stateCallback = std::make_shared<PasteboardDevStateCallback>();
        auto errNo = DeviceManager::GetInstance().RegisterDevStateCallback(PKG_NAME, "", stateCallback);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "RegisterDevStateCallback ret %{public}d", errNo);
        return errNo == DM_OK;
    });
#endif
    return DM_OK;
}

DevManager &DevManager::GetInstance()
{
    static DevManager instance;
    return instance;
}

void DevManager::Online(const std::string &networkId)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevProfile::GetInstance().SubscribeProfileEvent(networkId);
    DistributedModuleConfig::ForceNotify();
    DistributedModuleConfig::Notify();
}

void DevManager::Offline(const std::string &networkId)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevProfile::GetInstance().UnSubscribeProfileEvent(networkId);
    DistributedModuleConfig::Notify();
}

void DevManager::OnReady()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevProfile::GetInstance().OnReady();
}

void DevManager::RetryInBlocking(DevManager::Function func) const
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "retry start");
    constexpr int32_t RETRY_TIMES = 300;
    for (int32_t i = 0; i < RETRY_TIMES; ++i) {
        if (func()) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "retry result: %{public}d times", i);
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(DELAY_TIME));
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "retry failed");
}

bool DevManager::ConvertOsType(const std::string &jsonStr, const std::string &key, int32_t value)
{
    json jsonObj = json::parse(jsonStr, nullptr, false);
    if (jsonObj.is_discarded()) {
        // if the string size is less than 1, means the string is invalid.
        if (jsonStr.empty()) {
            return false;
        }
        // drop first char to adapt A's value;
        jsonObj = json::parse(jsonStr.substr(1), nullptr, false);
        if (jsonObj.is_discarded()) {
            return false;
        }
    }
    if (key.empty()) {
        return false;
    }
    auto it = jsonObj.find(key);
    if (it == jsonObj.end() || it->is_null() || !it->is_number_integer()) {
        return false;
    }
    it->get_to(value);
    return true;
}

std::vector<std::string> DevManager::GetNetworkIds()
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    std::vector<DmDeviceInfo> devices;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(PKG_NAME, "", devices);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetTrustedDeviceList failed!");
        return {};
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "devicesNums = %{public}zu.", devices.size());
    if (devices.empty()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no device online!");
        return {};
    }
    std::vector<std::string> networkIds;
    for (auto &item : devices) {
        networkIds.emplace_back(item.networkId);
    }
    return networkIds;
#else
    return {};
#endif
}

std::vector<std::string> DevManager::GetOldNetworkIds()
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    std::vector<std::string> networkIds = GetNetworkIds();
    if (networkIds.empty()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no device online!");
        return {};
    }
    std::vector<std::string> oldNetworkIds;
    for (auto &item : networkIds) {
        uint32_t versionId = 3;
        DevProfile::GetInstance().GetRemoteDeviceVersion(item, versionId);
        if (versionId >= FIRST_VERSION) {
            continue;
        }
        oldNetworkIds.emplace_back(item);
    }
    return oldNetworkIds;
#else
    return {};
#endif
}
} // namespace MiscServices
} // namespace OHOS
