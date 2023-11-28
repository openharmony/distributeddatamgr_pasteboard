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

    #ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    class PasteboardProfileEventCallback : public DeviceProfile::IProfileEventCallback {
    public:
        void OnSyncCompleted(const DeviceProfile::SyncResult &syncResults) override;
        void OnProfileChanged(const DeviceProfile::ProfileChangeNotification &changeNotification) override;
    };
    #endif

private:
    DevProfile();
    ~DevProfile() = default;
    static void ParameterChange(const char *key, const char *value, void *context);
    void SyncEnabledStatus();
    std::mutex callbackMutex_;
    bool localEnable_ = false;

    #ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    std::map<std::string, std::shared_ptr<PasteboardProfileEventCallback>> callback_;
    #endif // PB_DEVICE_INFO_MANAGER_ENABLE

};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_DEV_PROFILE_H
