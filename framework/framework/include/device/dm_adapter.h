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

#ifndef OHOS_PASTEBOARD_SERVICES_DEVICE_DM_ADAPTER_H
#define OHOS_PASTEBOARD_SERVICES_DEVICE_DM_ADAPTER_H
#include <shared_mutex>
#include <string>

#include "api/visibility.h"
#include "common/concurrent_map.h"
#ifdef PB_DEVICE_MANAGER_ENABLE
#include "dm_device_info.h"
#endif

namespace OHOS::MiscServices {
#ifdef PB_DEVICE_MANAGER_ENABLE
using namespace OHOS::DistributedHardware;
#endif
class API_EXPORT DMAdapter {
public:
    static constexpr size_t MAX_ID_LEN = 64;
    static constexpr size_t RESULT_OK = 0;
    class DMObserver {
    public:
        virtual void Online(const std::string &device) = 0;
        virtual void Offline(const std::string &device) = 0;
        virtual void OnReady(const std::string &device) = 0;
    };
    static DMAdapter &GetInstance();
    bool Initialize(const std::string &pkgName);
    void DeInitialize();
    const std::string &GetLocalDeviceUdid();
    const std::string GetLocalNetworkId();
    std::string GetUdidByNetworkId(const std::string &networkId);
    std::vector<std::string> GetNetworkIds();
    int32_t GetLocalDeviceType();
    bool IsSameAccount(const std::string &networkId);
    void SetDevices();

#ifdef PB_DEVICE_MANAGER_ENABLE
    int32_t GetRemoteDeviceInfo(const std::string &networkId, DmDeviceInfo &remoteDevice);
    std::vector<DmDeviceInfo> GetDevices();
#endif
    std::string GetDeviceName(const std::string &networkId);
    void Register(DMObserver *observer);
    void Unregister(DMObserver *observer);

private:
    static constexpr const char *NAME_EX = "dm_adapter";
    static constexpr const char *DEVICE_INVALID_NAME = "unknown";
    DMAdapter();
    ~DMAdapter();

    std::string pkgName_;
    const std::string invalidDeviceUdid_{};
    const std::string invalidNetworkId_{};
    const std::string invalidUdid_{};
    mutable std::mutex mutex_{};
    std::string localDeviceUdid_{};
    ConcurrentMap<DMObserver *, DMObserver *> observers_;
#ifdef PB_DEVICE_MANAGER_ENABLE
    std::shared_mutex dmMutex_;
    std::vector<DmDeviceInfo> devices_;
#endif
};
} // namespace OHOS::MiscServices
#endif // OHOS_PASTEBOARD_SERVICES_DEVICE_DM_ADAPTER_H
