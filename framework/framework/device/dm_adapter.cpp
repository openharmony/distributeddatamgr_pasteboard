/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "device/dm_adapter.h"
#include "device_manager.h"
#include "device_manager_callback.h"
#include "dm_device_info.h"
namespace OHOS::MiscServices {
using namespace OHOS::DistributedHardware;
constexpr size_t DMAdapter::MAX_ID_LEN;
class DmStateObserver : public DeviceStateCallback {
public:
    DmStateObserver(std::function<void(const DmDeviceInfo &)> action) : online_(std::move(action))
    {
    }
    void OnDeviceOnline(const DmDeviceInfo &deviceInfo) override
    {
        if (online_ == nullptr) {
            return;
        }
        online_(deviceInfo);
    }

    void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override
    {
    }
    void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override
    {
    }
    void OnDeviceReady(const DmDeviceInfo &deviceInfo) override
    {
    }

private:
    std::function<void(const DmDeviceInfo &)> online_;
};

class DmDeath : public DmInitCallback, public std::enable_shared_from_this<DmDeath> {
public:
    DmDeath(std::shared_ptr<DmStateObserver> observer, std::string pkgName)
        : observer_(observer), pkgName_(std::move(pkgName))
    {
    }
    void OnRemoteDied() override
    {
        DeviceManager::GetInstance().InitDeviceManager(pkgName_, shared_from_this());
        DeviceManager::GetInstance().RegisterDevStateCallback(pkgName_, "", observer_);
    }

private:
    std::shared_ptr<DmStateObserver> observer_;
    std::string pkgName_;
};

DMAdapter::DMAdapter()
{

}

DMAdapter::~DMAdapter()
{
}

DMAdapter &DMAdapter::GetInstance()
{
    static DMAdapter instance;
    return instance;
}

bool DMAdapter::Initialize(const std::string &pkgName)
{
    auto stateObserver = std::make_shared<DmStateObserver>([this](const DmDeviceInfo &deviceInfo) {
        observers_.ForEach([&deviceInfo](auto &key, auto &value) {
            value->Online(deviceInfo.networkId);
            return false;
        });
    });
    pkgName_ = pkgName + NAME_EX;
    auto deathObserver = std::make_shared<DmDeath>(stateObserver, pkgName_);
    deathObserver->OnRemoteDied();
    return false;
}

const std::string &DMAdapter::GetLocalDevice()
{
    std::lock_guard<decltype(mutex_)> lockGuard(mutex_);
    if (!localDeviceId_.empty()) {
        return localDeviceId_;
    }

    DmDeviceInfo info;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(pkgName_, info);
    if (ret != 0) {
        return invalidDeviceId_;
    }
    DeviceManager::GetInstance().GetUdidByNetworkId(pkgName_, info.networkId, localDeviceId_);
    if (localDeviceId_.empty()) {
        return invalidDeviceId_;
    }
    return localDeviceId_;
}

std::string DMAdapter::GetDeviceName(const std::string &udid)
{
    std::vector<DmDeviceInfo> devices;
    (void)DeviceManager::GetInstance().GetTrustedDeviceList(pkgName_, "", devices);
    for (auto &device : devices) {
        std::string deviceId;
        (void)DeviceManager::GetInstance().GetUdidByNetworkId(pkgName_, device.networkId, deviceId);
        if (deviceId == udid) {
            return device.deviceName;
        }
    }
    return DEVICE_INVALID_NAME;
}

void DMAdapter::Register(DMObserver *observer)
{
    observers_.Insert(observer, observer);
}

void DMAdapter::Unregister(DMObserver *observer)
{
    observers_.Erase(observer);
}
} // namespace OHOS::MiscServices