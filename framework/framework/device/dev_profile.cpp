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
#include <cstring>
#include <thread>

#include "cJSON.h"
#include "dev_profile.h"
#include "distributed_module_config.h"
#include "dm_adapter.h"
#include "pasteboard_error.h"
#include "pasteboard_event_ue.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
using namespace OHOS::DistributedDeviceProfile;
using namespace UeReporter;
constexpr const int32_t HANDLE_OK = 0;
constexpr const int32_t PASTEBOARD_SA_ID = 3701;

constexpr const char *SERVICE_ID = "pasteboardService";
constexpr const char *STATIC_CHARACTER_ID = "static_capability";
constexpr const char *VERSION_ID = "PasteboardVersionId";
constexpr const char *CHARACTERISTIC_VALUE = "characteristicValue";
constexpr const char *SUPPORT_STATUS = "1";
constexpr const char *SWITCH_ID = "SwitchStatus_Key_Distributed_Pasteboard";
constexpr const char *CHARACTER_ID = "SwitchStatus";

DevProfile::SubscribeDPChangeListener::SubscribeDPChangeListener() {}

DevProfile::SubscribeDPChangeListener::~SubscribeDPChangeListener() {}

int32_t DevProfile::SubscribeDPChangeListener::OnTrustDeviceProfileAdd(const TrustDeviceProfile &profile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnTrustDeviceProfileAdd start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnTrustDeviceProfileDelete(const TrustDeviceProfile &profile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnTrustDeviceProfileDelete start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnTrustDeviceProfileUpdate(
    const TrustDeviceProfile &oldProfile, const TrustDeviceProfile &newProfile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnTrustDeviceProfileUpdate start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnDeviceProfileAdd(const DeviceProfile &profile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnDeviceProfileAdd start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnDeviceProfileDelete(const DeviceProfile &profile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnDeviceProfileDelete start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnDeviceProfileUpdate(
    const DeviceProfile &oldProfile, const DeviceProfile &newProfile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnDeviceProfileUpdate start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnServiceProfileAdd(const ServiceProfile &profile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnServiceProfileAdd start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnServiceProfileDelete(const ServiceProfile &profile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnServiceProfileDelete start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnServiceProfileUpdate(
    const ServiceProfile &oldProfile, const ServiceProfile &newProfile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnServiceProfileUpdate start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnCharacteristicProfileAdd(const CharacteristicProfile &profile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnCharacteristicProfileAdd start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnCharacteristicProfileDelete(const CharacteristicProfile &profile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnCharacteristicProfileDelete start.");
    return 0;
}

int32_t DevProfile::SubscribeDPChangeListener::OnCharacteristicProfileUpdate(
    const CharacteristicProfile &oldProfile, const CharacteristicProfile &newProfile)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnCharacteristicProfileUpdate start.");
    DevProfile::GetInstance().Notify(newProfile.GetCharacteristicValue() == SUPPORT_STATUS);
    return 0;
}
#endif

DevProfile::DevProfile() {}

DevProfile &DevProfile::GetInstance()
{
    static DevProfile instance;
    return instance;
}

void DevProfile::OnReady() {}

void DevProfile::PutEnabledStatus(const std::string &enabledStatus)
{
    Notify(enabledStatus == SUPPORT_STATUS);
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PutEnabledStatus, start");
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    auto ret = GetEnabledStatus(networkId);
    if (ret.first == static_cast<int32_t>(PasteboardError::E_OK) && (enabledStatus == ret.second)) {
        return;
    }
    std::string udid = DMAdapter::GetInstance().GetUdidByNetworkId(networkId);
    if (udid.empty()) {
        PASTEBOARD_HILOGE(
            PASTEBOARD_MODULE_SERVICE, "GetUdidByNetworkId failed, networkId is %{public}.5s", networkId.c_str());
        return;
    }
    UE_SWITCH(UeReporter::UE_SWITCH_OPERATION, UeReporter::UE_OPERATION_TYPE,
        (enabledStatus == SUPPORT_STATUS) ? UeReporter::SwitchStatus::SWITCH_OPEN
                                          : UeReporter::SwitchStatus::SWITCH_CLOSE);
    DistributedDeviceProfile::CharacteristicProfile profile;
    profile.SetDeviceId(udid);
    profile.SetServiceName(SWITCH_ID);
    profile.SetCharacteristicKey(CHARACTER_ID);
    profile.SetCharacteristicValue(enabledStatus);
    int32_t errNo = DistributedDeviceProfileClient::GetInstance().PutCharacteristicProfile(profile);
    if (errNo != HANDLE_OK && errNo != DistributedDeviceProfile::DP_CACHE_EXIST) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PutCharacteristicProfile failed, %{public}d", errNo);
        return;
    }
#else
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PB_DEVICE_INFO_MANAGER_ENABLE not defined");
    return;
#endif
}

std::pair<int32_t, std::string> DevProfile::GetEnabledStatus(const std::string &networkId)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    std::string udid = DMAdapter::GetInstance().GetUdidByNetworkId(networkId);
    if (udid.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetUdidByNetworkId failed");
        return std::make_pair(static_cast<int32_t>(PasteboardError::GET_LOCAL_DEVICE_ID_ERROR), "");
    }
    DistributedDeviceProfile::CharacteristicProfile profile;
    int32_t ret =
        DistributedDeviceProfileClient::GetInstance().GetCharacteristicProfile(udid, SWITCH_ID, CHARACTER_ID, profile);
    if (ret == HANDLE_OK && profile.GetCharacteristicValue() == SUPPORT_STATUS) {
        return std::make_pair(static_cast<int32_t>(PasteboardError::E_OK), SUPPORT_STATUS);
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Get status failed, %{public}.5s. ret:%{public}d", udid.c_str(), ret);
    if (ret == DP_LOAD_SERVICE_ERR) {
        return std::make_pair(static_cast<int32_t>(PasteboardError::DP_LOAD_SERVICE_ERROR), "");
    }
#else
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PB_DEVICE_INFO_MANAGER_ENABLE not defined");
#endif
    return std::make_pair(static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR), "");
}

void DevProfile::GetRemoteDeviceVersion(const std::string &networkId, uint32_t &versionId)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetRemoteDeviceVersion start.");
    std::string udid = DMAdapter::GetInstance().GetUdidByNetworkId(networkId);
    if (udid.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetUdidByNetworkId failed.");
        return;
    }
    DistributedDeviceProfile::CharacteristicProfile profile;
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetCharacteristicProfile(
        udid, SERVICE_ID, STATIC_CHARACTER_ID, profile);
    if (ret != HANDLE_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetCharacteristicProfile failed, %{public}.5s.", udid.c_str());
        return;
    }
    const auto &jsonData = profile.GetCharacteristicValue();
    cJSON *jsonObject = cJSON_Parse(jsonData.c_str());
    if (jsonObject == nullptr) {
        cJSON_Delete(jsonObject);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "json parse failed.");
        return;
    }
    if (cJSON_GetNumberValue(cJSON_GetObjectItem(jsonObject, VERSION_ID)) == FIRST_VERSION) {
        versionId = FIRST_VERSION;
    }
    cJSON_Delete(jsonObject);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetRemoteDeviceVersion success, versionId = %{public}d.", versionId);
#else
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PB_DEVICE_INFO_MANAGER_ENABLE not defined");
    return;
#endif
}

void DevProfile::SubscribeProfileEvent(const std::string &networkId)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start, networkId = %{public}.5s", networkId.c_str());
    std::string udid = DMAdapter::GetInstance().GetUdidByNetworkId(networkId);
    if (udid.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetUdidByNetworkId failed");
        return;
    }
    std::lock_guard<std::mutex> mutexLock(callbackMutex_);
    if (subscribeInfoCache_.find(udid) != subscribeInfoCache_.end()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "networkId = %{public}.5s already exists.", udid.c_str());
        return;
    }
    DistributedDeviceProfile::SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(PASTEBOARD_SA_ID);
    subscribeInfo.SetSubscribeKey(udid, SWITCH_ID, CHARACTER_ID, CHARACTERISTIC_VALUE);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_DELETE);
    sptr<IProfileChangeListener> subscribeDPChangeListener = new (std::nothrow) SubscribeDPChangeListener;
    subscribeInfo.SetListener(subscribeDPChangeListener);
    subscribeInfoCache_[udid] = subscribeInfo;
    int32_t errCode = DistributedDeviceProfileClient::GetInstance().SubscribeDeviceProfile(subscribeInfo);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SubscribeDeviceProfile result, errCode = %{public}d.", errCode);
#else
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PB_DEVICE_INFO_MANAGER_ENABLE not defined");
    return;
#endif
}

void DevProfile::UnSubscribeProfileEvent(const std::string &networkId)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start, networkId = %{public}.5s", networkId.c_str());
    std::string udid = DMAdapter::GetInstance().GetUdidByNetworkId(networkId);
    if (udid.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetUdidByNetworkId failed, %{public}.5s.", udid.c_str());
        return;
    }
    std::lock_guard<std::mutex> mutexLock(callbackMutex_);
    auto it = subscribeInfoCache_.find(udid);
    if (it == subscribeInfoCache_.end()) {
        return;
    }
    int32_t errCode = DistributedDeviceProfileClient::GetInstance().UnSubscribeDeviceProfile(it->second);
    subscribeInfoCache_.erase(it);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "UnsubscribeProfileEvent result, errCode = %{public}d.", errCode);
#else
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PB_DEVICE_INFO_MANAGER_ENABLE not defined");
    return;
#endif
}

void DevProfile::UnsubscribeAllProfileEvents()
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "UnsubscribeAllProfileEvents start.");
    std::lock_guard<std::mutex> mutexLock(callbackMutex_);
    for (auto it = subscribeInfoCache_.begin(); it != subscribeInfoCache_.end(); ++it) {
        int32_t ret = DistributedDeviceProfileClient::GetInstance().UnSubscribeDeviceProfile(it->second);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "errCode = %{public}d.", ret);
        it = subscribeInfoCache_.erase(it);
    }
#else
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PB_DEVICE_INFO_MANAGER_ENABLE not defined");
    return;
#endif
}

void DevProfile::Watch(Observer observer)
{
    observer_ = std::move(observer);
}

void DevProfile::Notify(bool isEnable)
{
    if (observer_ != nullptr) {
        observer_(isEnable);
    }
}
} // namespace MiscServices
} // namespace OHOS
