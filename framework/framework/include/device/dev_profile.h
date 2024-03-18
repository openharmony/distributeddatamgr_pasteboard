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

#ifndef PASTE_BOARD_DEV_PROFILE_H
#define PASTE_BOARD_DEV_PROFILE_H

#include "api/visibility.h"

#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
#include "distributed_device_profile_client.h"
#include "profile_change_listener_stub.h"
#endif // PB_DEVICE_INFO_MANAGER_ENABLE

namespace OHOS {
namespace MiscServices {
class API_EXPORT DevProfile {
public:
    static DevProfile &GetInstance();
    void GetEnabledStatus(const std::string &networkId, std::string &enabledStatus);
    void Init();
    void OnReady();
    void PutEnabledStatus(const std::string &enabledStatus);
    void GetRemoteDeviceVersion(const std::string &networkId, uint32_t &deviceVersion);
    void SubscribeProfileEvent(const std::string &networkId);
    void UnSubscribeProfileEvent(const std::string &networkId);
    void UnsubscribeAllProfileEvents();
    bool GetLocalEnable();
    static constexpr const uint32_t FIRST_VERSION = 4;
    #ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    class SubscribeDPChangeListener : public DistributedDeviceProfile::ProfileChangeListenerStub {
    public:
        SubscribeDPChangeListener();
        ~SubscribeDPChangeListener();
        int32_t OnTrustDeviceProfileAdd(const DistributedDeviceProfile::TrustDeviceProfile &profile) override;
        int32_t OnTrustDeviceProfileDelete(const DistributedDeviceProfile::TrustDeviceProfile &profile) override;
        int32_t OnTrustDeviceProfileUpdate(const DistributedDeviceProfile::TrustDeviceProfile &oldProfile,
            const DistributedDeviceProfile::TrustDeviceProfile &newProfile) override;
        int32_t OnDeviceProfileAdd(const DistributedDeviceProfile::DeviceProfile &profile) override;
        int32_t OnDeviceProfileDelete(const DistributedDeviceProfile::DeviceProfile &profile) override;
        int32_t OnDeviceProfileUpdate(const DistributedDeviceProfile::DeviceProfile &oldProfile,
            const DistributedDeviceProfile::DeviceProfile &newProfile) override;
        int32_t OnServiceProfileAdd(const DistributedDeviceProfile::ServiceProfile &profile) override;
        int32_t OnServiceProfileDelete(const DistributedDeviceProfile::ServiceProfile &profile) override;
        int32_t OnServiceProfileUpdate(const DistributedDeviceProfile::ServiceProfile &oldProfile,
            const DistributedDeviceProfile::ServiceProfile &newProfile) override;
        int32_t OnCharacteristicProfileAdd(const DistributedDeviceProfile::CharacteristicProfile &profile) override;
        int32_t OnCharacteristicProfileDelete(const DistributedDeviceProfile::CharacteristicProfile &profile) override;
        int32_t OnCharacteristicProfileUpdate(const DistributedDeviceProfile::CharacteristicProfile &oldProfile,
            const DistributedDeviceProfile::CharacteristicProfile &newProfile) override;
    };
    #endif

private:
    DevProfile();
    ~DevProfile() = default;
    static void ParameterChange(const char *key, const char *value, void *context);
    std::mutex callbackMutex_;
    bool localEnable_ = false;

    #ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    std::map<std::string, DistributedDeviceProfile::SubscribeInfo> subscribeInfoCache_;
    #endif // PB_DEVICE_INFO_MANAGER_ENABLE

};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_DEV_PROFILE_H
