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
#include "dev_manager.h"

#include <thread>

#include "dev_profile.h"
#include "device_manager.h"
#include "distributed_module_config.h"
#include "pasteboard_hilog.h"
namespace OHOS {
namespace MiscServices {
constexpr const char *PKG_NAME = "pasteboard_service";
constexpr int32_t DM_OK = 0;
constexpr const int32_t DELAY_TIME = 200;
using namespace OHOS::DistributedHardware;

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
    DevManager::GetInstance().Online(deviceInfo.deviceId);
}

void PasteboardDevStateCallback::OnDeviceOffline(const DmDeviceInfo &deviceInfo)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevManager::GetInstance().Offline(deviceInfo.deviceId);
}
void PasteboardDevStateCallback::OnDeviceChanged(const DmDeviceInfo &deviceInfo)
{
}
void PasteboardDevStateCallback::OnDeviceReady(const DmDeviceInfo &deviceInfo)
{
}

void PasteboardDmInitCallback::OnRemoteDied()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "dm device manager died, init it again");
    DevManager::GetInstance().Init();
}

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
    return DM_OK;
}

DevManager &DevManager::GetInstance()
{
    static DevManager instance;
    return instance;
}

void DevManager::Online(const std::string &deviceId)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevProfile::GetInstance().SubscribeProfileEvent(deviceId);
    DistributedModuleConfig::Notify();
}

void DevManager::Offline(const std::string &deviceId)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevProfile::GetInstance().UnSubscribeProfileEvent(deviceId);
    DistributedModuleConfig::Notify();
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
std::vector<std::string> DevManager::GetDeviceIds()
{
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
    std::vector<std::string> deviceIds;
    for (auto &item : devices) {
        deviceIds.emplace_back(item.deviceId);
    }
    return deviceIds;
}
} // namespace MiscServices
} // namespace OHOS