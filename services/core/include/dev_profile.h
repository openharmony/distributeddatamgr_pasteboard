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

#ifndef PASTE_BOARD_DEV_PROFILE_H
#define PASTE_BOARD_DEV_PROFILE_H

#include "distributed_device_profile_client.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::DeviceProfile;

class DevProfile {
public:
    static DevProfile &GetInstance();

    void GetDeviceProfile(const std::string &deviceId, std::string &dpbEnable);
    void PutDeviceProfile(const std::string &dpbEnable);
    int32_t PutDeviceProfileInit(const std::string &dpbEnable);
    void RegisterProfileCallback(const std::string &deviceId);
    void UnRegisterProfileCallback(const std::string &deviceId);
    void SubscribeProfileEvent(const std::string &deviceId);
    void UnSubscribeProfileEvent(const std::string &deviceId);
    void UnRegisterAllProfileCallback();
    void ProfileChanged();
    class PasteboardProfileEventCallback : public IProfileEventCallback {
public:
    void OnSyncCompleted(const SyncResult& syncResults) override;
    void OnProfileChanged(const ProfileChangeNotification& changeNotification) override;
};

private:
    DevProfile();
    ~DevProfile() = default;
    std::mutex callbackMapMutex_;
    std::map<std::string, std::shared_ptr<PasteboardProfileEventCallback>> callbackMap_;
};
} // namespace MiscServices
} // namespace OHOS

#endif //PASTE_BOARD_DEV_PROFILE_H
