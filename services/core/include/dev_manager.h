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

#ifndef PASTE_BOARD_DEV_MANAGER_H
#define PASTE_BOARD_DEV_MANAGER_H

#include <string>

#include "device_manager_callback.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::DistributedHardware;
class DevManager {
public:
    static DevManager &GetInstance();
    void Online(const std::string &deviceId);
    void Offline(const std::string &deviceId);
    void RegisterDevCallback();
    void UnRegisterDevCallback();
    class PasteboardDevStateCallback : public DeviceStateCallback {
    public:
        void OnDeviceOnline(const DmDeviceInfo &deviceInfo) override;
        void OnDeviceOffline(const DmDeviceInfo &deviceInfo) override;
        void OnDeviceChanged(const DmDeviceInfo &deviceInfo) override;
        void OnDeviceReady(const DmDeviceInfo &deviceInfo) override;
    };
    class PasteboardDmInitCallback : public DmInitCallback {
    public:
        void OnRemoteDied() override;
    };
private:
    DevManager();
    ~DevManager() = default;
    int32_t init();
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_DEV_MANAGER_H
