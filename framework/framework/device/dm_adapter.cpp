/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#ifdef PB_DEVICE_MANAGER_ENABLE
#include "device_manager.h"
#include "device_manager_callback.h"
#endif
namespace OHOS::MiscServices {
constexpr size_t DMAdapter::MAX_ID_LEN;
constexpr const char *PKG_NAME = "pasteboard_service";

#ifdef PB_DEVICE_MANAGER_ENABLE
class DmStateObserver : public DeviceStateCallback {
public:
    DmStateObserver(const std::function<void(const DmDeviceInfo &)> online,
        const std::function<void(const DmDeviceInfo &)> onReady,
        const std::function<void(const DmDeviceInfo &)> offline)
        : online_(std::move(online)), onReady_(std::move(onReady)), offline_(std::move(offline))
    {
    }

    void OnDeviceOnline(const DmDeviceInfo &deviceInfo) override
    {
        if (online_ == nullptr || deviceInfo.authForm != IDENTICAL_ACCOUNT) {
            return;
        }
        online_(deviceInfo);
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "device on:%{public}.6s", deviceInfo.networkId);
    }

    void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override
    {
        if (offline_ == nullptr) {
            return;
        }
        offline_(deviceInfo);
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "device off:%{public}.6s", deviceInfo.networkId);
    }

    void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override
    {
        // authForm not vaild use networkId
        if (DeviceManager::GetInstance().IsSameAccount(deviceInfo.networkId)) {
            online_(deviceInfo);
        }
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "device config changed:%{public}.6s", deviceInfo.networkId);
    }

    void OnDeviceReady(const DmDeviceInfo &deviceInfo) override
    {
        if (onReady_ == nullptr || deviceInfo.authForm != IDENTICAL_ACCOUNT) {
            return;
        }
        onReady_(deviceInfo);
    }

private:
    std::function<void(const DmDeviceInfo &)> online_;
    std::function<void(const DmDeviceInfo &)> onReady_;
    std::function<void(const DmDeviceInfo &)> offline_;
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
#endif

DMAdapter::DMAdapter() {}

DMAdapter::~DMAdapter() {}

DMAdapter &DMAdapter::GetInstance()
{
    static DMAdapter instance;
    return instance;
}

bool DMAdapter::Initialize(const std::string &pkgName)
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    auto stateObserver = std::make_shared<DmStateObserver>(
        [this](const DmDeviceInfo &deviceInfo) {
            observers_.ForEachCopies([&deviceInfo](auto &key, auto &value) {
                DMAdapter::GetInstance().SetDevices();
                value->Online(deviceInfo.networkId);
                return false;
            });
        },
        [this](const DmDeviceInfo &deviceInfo) {
            observers_.ForEachCopies([&deviceInfo](auto &key, auto &value) {
                value->OnReady(deviceInfo.networkId);
                return false;
            });
        },
        [this](const DmDeviceInfo &deviceInfo) {
            observers_.ForEachCopies([&deviceInfo](auto &key, auto &value) {
                DMAdapter::GetInstance().SetDevices();
                value->Offline(deviceInfo.networkId);
                return false;
            });
        });
    pkgName_ = pkgName + NAME_EX;
    auto deathObserver = std::make_shared<DmDeath>(stateObserver, pkgName_);
    deathObserver->OnRemoteDied();
    SetDevices();
#endif
    return false;
}

void DMAdapter::DeInitialize()
{
    auto &deviceManager = DeviceManager::GetInstance();
    deviceManager.UnRegisterDevStateCallback(pkgName_);
    deviceManager.UnInitDeviceManager(pkgName_);
}

const std::string &DMAdapter::GetLocalDeviceUdid()
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    std::lock_guard<decltype(mutex_)> lockGuard(mutex_);
    if (!localDeviceUdid_.empty()) {
        return localDeviceUdid_;
    }

    DmDeviceInfo info;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(pkgName_, info);
    if (ret != 0) {
        return invalidDeviceUdid_;
    }
    DeviceManager::GetInstance().GetUdidByNetworkId(pkgName_, info.networkId, localDeviceUdid_);
    if (!localDeviceUdid_.empty()) {
        return localDeviceUdid_;
    }
#endif
    return invalidDeviceUdid_;
}

std::string DMAdapter::GetDeviceName(const std::string &networkId)
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    auto devices = GetDevices();
    for (auto &device : devices) {
        if (device.networkId == networkId) {
            return device.deviceName;
        }
    }
#endif
    return DEVICE_INVALID_NAME;
}

const std::string DMAdapter::GetLocalNetworkId()
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    DmDeviceInfo info;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceInfo(pkgName_, info);
    auto networkId = std::string(info.networkId);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ret: %{public}d, networkId:%{public}s", ret, networkId.c_str());
    if (ret == 0 && !networkId.empty()) {
        return networkId;
    }
#endif
    return invalidNetworkId_;
}

int32_t DMAdapter::GetRemoteDeviceInfo(const std::string &networkId, DmDeviceInfo &remoteDevice)
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    auto devices = GetDevices();
    for (auto &device : devices) {
        if (device.networkId == networkId) {
            remoteDevice = device;
            return static_cast<int32_t>(PasteboardError::E_OK);
        }
    }
#endif
    return static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR);
}

std::string DMAdapter::GetUdidByNetworkId(const std::string &networkId)
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    std::string udid;
    int32_t ret = DeviceManager::GetInstance().GetUdidByNetworkId(pkgName_, networkId, udid);
    if (ret == 0 && !udid.empty()) {
        return udid;
    }
#endif
    return invalidUdid_;
}

void DMAdapter::Register(DMObserver *observer)
{
    observers_.Insert(observer, observer);
}

void DMAdapter::Unregister(DMObserver *observer)
{
    observers_.Erase(observer);
}

std::vector<std::string> DMAdapter::GetNetworkIds()
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    auto devices = GetDevices();
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

int32_t DMAdapter::GetLocalDeviceType()
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    int32_t deviceType = DmDeviceType::DEVICE_TYPE_UNKNOWN;
    int32_t ret = DeviceManager::GetInstance().GetLocalDeviceType(PKG_NAME, deviceType);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get type failed, ret is %{public}d!", ret);
    }
    return deviceType;
#else
    return static_cast<int32_t>(PasteboardError::NO_DATA_ERROR);
#endif
}

bool DMAdapter::IsSameAccount(const std::string &networkId)
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    auto devices = GetDevices();
    for (auto &device : devices) {
        if (device.networkId == networkId) {
            return device.authForm == IDENTICAL_ACCOUNT;
        }
    }
#endif
    return false;
}

void DMAdapter::SetDevices()
{
    std::vector<DmDeviceInfo> devices;
    int32_t ret = DeviceManager::GetInstance().GetTrustedDeviceList(PKG_NAME, "", devices);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Get device list failed, errCode: %{public}d", ret);
        return;
    }
    std::vector<DmDeviceInfo> networkIds;
    for (auto &item : devices) {
        if (DeviceManager::GetInstance().IsSameAccount(item.networkId)) {
            networkIds.emplace_back(item);
        }
    }
    std::unique_lock<std::shared_mutex> lock(dmMutex_);
    devices_ = std::move(networkIds);
}

std::vector<DmDeviceInfo> DMAdapter::GetDevices()
{
    std::shared_lock<std::shared_mutex> lock(dmMutex_);
    return devices_;
}
} // namespace OHOS::MiscServices