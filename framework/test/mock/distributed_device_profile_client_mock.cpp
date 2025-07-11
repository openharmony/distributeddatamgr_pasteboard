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

#include "distributed_device_profile_client_mock.h"

namespace OHOS {
namespace DistributedDeviceProfile {

DeviceProfileClientMock::DeviceProfileClientMock()
{
    DeviceProfileClientMock::mock_ = this;
}

DeviceProfileClientMock::~DeviceProfileClientMock()
{
    DeviceProfileClientMock::mock_ = nullptr;
}

DeviceProfileClientMock *DeviceProfileClientMock::GetMock()
{
    return DeviceProfileClientMock::mock_;
}

void DeviceProfileLoadCb::OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    (void)systemAbilityId;
    DeviceProfileClient::GetInstance().LoadSystemAbilitySuccess(remoteObject);
}

void DeviceProfileLoadCb::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    (void)systemAbilityId;
    DeviceProfileClient::GetInstance().LoadSystemAbilityFail();
}

DeviceProfileClient &DeviceProfileClient::GetInstance()
{
    static DeviceProfileClient instance;
    return instance;
}

sptr<IDistributedDeviceProfile> DeviceProfileClient::GetDeviceProfileService()
{
    return nullptr;
}

sptr<IDistributedDeviceProfile> DeviceProfileClient::LoadDeviceProfileService()
{
    return nullptr;
}

void DeviceProfileClient::ClearDeviceProfileService()
{
}

void DeviceProfileClient::SendSubscribeInfos()
{
    DeviceProfileClientMock::GetMock()->SendSubscribeInfos();
}

int32_t DeviceProfileClient::PutCharacteristicProfile(const CharacteristicProfile &characteristicProfile)
{
    return DeviceProfileClientMock::GetMock()->PutCharacteristicProfile(characteristicProfile);
}

int32_t DeviceProfileClient::GetCharacteristicProfile(const std::string &deviceId, const std::string &serviceName,
    const std::string &characteristicId, CharacteristicProfile &characteristicProfile)
{
    return DeviceProfileClientMock::GetMock()->GetCharacteristicProfile(deviceId, serviceName, characteristicId,
        characteristicProfile);
}

int32_t DeviceProfileClient::SubscribeDeviceProfile(const SubscribeInfo &subscribeInfo)
{
    return DeviceProfileClientMock::GetMock()->SubscribeDeviceProfile(subscribeInfo);
}

int32_t DeviceProfileClient::UnSubscribeDeviceProfile(const SubscribeInfo &subscribeInfo)
{
    return DeviceProfileClientMock::GetMock()->UnSubscribeDeviceProfile(subscribeInfo);
}
} // namespace DistributedDeviceProfile
} // namespace OHOS
