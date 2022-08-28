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
#include "dev_manager.h"

#include <thread>

#include "dev_profile.h"
#include "device_manager.h"
#include "distributed_module_config.h"
#include "pasteboard_hilog_wreapper.h"
namespace OHOS {
namespace MiscServices {
constexpr const char *PKG_NAME = "PasteboardService";
constexpr int32_t DM_OK = 0;
constexpr int32_t DM_ERROR = -1;
constexpr const char EMPTY_STR[] = { "" };

void DevManager::PasteboardDevStateCallback::OnDeviceOnline(const DmDeviceInfo &deviceInfo)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevManager::GetInstance().Online(deviceInfo.deviceId);
}

void DevManager::PasteboardDevStateCallback::OnDeviceOffline(const DmDeviceInfo &deviceInfo)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevManager::GetInstance().Offline(deviceInfo.deviceId);
}
void DevManager::PasteboardDevStateCallback::OnDeviceChanged(const DmDeviceInfo &deviceInfo)
{
}
void DevManager::PasteboardDevStateCallback::OnDeviceReady(const DmDeviceInfo &deviceInfo)
{
}

class PasteboardDmInitCallback : public DmInitCallback {
public:
    void OnRemoteDied() override;
};

void PasteboardDmInitCallback::OnRemoteDied()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "dm device manager died, init it again");
    DevManager::GetInstance().RegisterDevCallback();
}

DevManager::DevManager()
{
    //RegisterDevCallback();
}

void DevManager::UnRegisterDevCallback()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start");
    auto &deviceManager = DeviceManager::GetInstance();
    deviceManager.UnRegisterDevStateCallback(PKG_NAME);
    deviceManager.UnInitDeviceManager(PKG_NAME);

    DevProfile::GetInstance().UnRegisterAllProfileCallback();
}

int32_t DevManager::init()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start");
    auto &deviceManager = DeviceManager::GetInstance();
    auto deviceInitCallback = std::make_shared<PasteboardDmInitCallback>();
    auto deviceCallback = std::make_shared<PasteboardDevStateCallback>();
    int32_t errNo = deviceManager.InitDeviceManager(PKG_NAME, deviceInitCallback);
    if (errNo != DM_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "InitDeviceManager failed, errNo = %{public}d.", errNo);
        return errNo;
    }
    errNo = deviceManager.RegisterDevStateCallback(PKG_NAME, EMPTY_STR, deviceCallback);

    return errNo;
}

void DevManager::RegisterDevCallback()
{
    int32_t errNo = init();
    if (errNo == DM_OK) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "RegisterDevCallback success");
        return;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "register device failed, try again");
    std::thread th = std::thread([this]() {
        constexpr int RETRY_TIMES = 300;
        int i = 0;
        int32_t errNo = DM_ERROR;
        while (i++ < RETRY_TIMES) {
            errNo = init();
            if (errNo == DM_OK) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        PASTEBOARD_HILOGI(
            PASTEBOARD_MODULE_SERVICE, "reg device exit now: %{public}d times, errNo: %{public}d", i, errNo);
    });
    th.detach();
}

DevManager &DevManager::GetInstance()
{
    static DevManager instance;
    return instance;
}

void DevManager::Online(const std::string &deviceId)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevProfile::GetInstance().RegisterProfileCallback(deviceId);
    DistributedModuleConfig::Notify();
}

void DevManager::Offline(const std::string &deviceId)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    DevProfile::GetInstance().UnRegisterProfileCallback(deviceId);
    DistributedModuleConfig::Notify();
}
} // namespace MiscServices
} // namespace OHOS