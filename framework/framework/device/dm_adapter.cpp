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
constexpr const char *DMAdapter::PKG_NAME;
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
    DmDeath(std::shared_ptr<DmStateObserver> observer) : observer_(observer)
    {
    }
    void OnRemoteDied() override
    {
        DeviceManager::GetInstance().InitDeviceManager(DMAdapter::PKG_NAME, shared_from_this());
        DeviceManager::GetInstance().RegisterDevStateCallback(DMAdapter::PKG_NAME, "", observer_);
    }

private:
    std::shared_ptr<DmStateObserver> observer_;
};

DMAdapter::DMAdapter()
{
    auto stateObserver = std::make_shared<DmStateObserver>([this](const DmDeviceInfo &deviceInfo) {
        observers_.ForEach([&deviceInfo](auto &key, auto &value) {
            value->Online(deviceInfo.networkId);
            return false;
        });
    });
    auto deathObserver = std::make_shared<DmDeath>(stateObserver);
    DeviceManager::GetInstance().InitDeviceManager(PKG_NAME, deathObserver);
    DeviceManager::GetInstance().RegisterDevStateCallback(PKG_NAME, "", stateObserver);
}

DMAdapter::~DMAdapter()
{
    DeviceManager::GetInstance().UnRegisterDevStateCallback(PKG_NAME);
    DeviceManager::GetInstance().UnInitDeviceManager(PKG_NAME);
}

DMAdapter &DMAdapter::GetInstance()
{
    static DMAdapter instance;
    return instance;
}

const std::string &DMAdapter::GetLocalDevice()
{
    std::lock_guard<decltype(mutex_)> lockGuard(mutex_);
    if (!localDeviceId_.empty()) {
        return localDeviceId_;
    }

    DmDeviceInfo info;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(PKG_NAME, info);
    if (ret != 0) {
        return invalidDeviceId_;
    }
    DeviceManager::GetInstance().GetUdidByNetworkId(PKG_NAME, info.networkId, localDeviceId_);
    if (localDeviceId_.empty()) {
        return invalidDeviceId_;
    }
    return localDeviceId_;
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