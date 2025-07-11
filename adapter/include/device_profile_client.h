/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PASTEBOARD_DEVICE_PROFILE_CLIENT_H
#define PASTEBOARD_DEVICE_PROFILE_CLIENT_H

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>

#include "i_distributed_device_profile.h"
#include "iremote_object.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace DistributedDeviceProfile {
class DeviceProfileLoadCb : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};

class DeviceProfileClient {
public:
    static DeviceProfileClient &GetInstance();

    int32_t PutCharacteristicProfile(const CharacteristicProfile &characteristicProfile);
    int32_t GetCharacteristicProfile(const std::string &deviceId, const std::string &serviceName,
        const std::string &characteristicId, CharacteristicProfile &characteristicProfile);
    int32_t SubscribeDeviceProfile(const SubscribeInfo &subscribeInfo);
    int32_t UnSubscribeDeviceProfile(const SubscribeInfo &subscribeInfo);
    void SendSubscribeInfos();
    void ClearDeviceProfileService();

    void LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void LoadSystemAbilityFail();

private:
    sptr<IDistributedDeviceProfile> LoadDeviceProfileService();
    sptr<IDistributedDeviceProfile> GetDeviceProfileService();

    std::mutex serviceLock_;
    std::condition_variable proxyConVar_;
    sptr<IDistributedDeviceProfile> dpProxy_ = nullptr;

    std::mutex subscribeLock_;
    std::map<std::string, SubscribeInfo> subscribeInfos_;
};
} // namespace DistributedDeviceProfile
} // namespace OHOS
#endif // PASTEBOARD_DEVICE_PROFILE_CLIENT_H
