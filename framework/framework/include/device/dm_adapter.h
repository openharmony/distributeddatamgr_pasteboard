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

#ifndef OHOS_PASTEBOARD_SERVICES_DEVICE_DM_ADAPTER_H
#define OHOS_PASTEBOARD_SERVICES_DEVICE_DM_ADAPTER_H
#include <mutex>
#include <string>
#include "api/visibility.h"
#include "common/concurrent_map.h"
namespace OHOS::MiscServices {
class API_EXPORT DMAdapter {
public:
    static constexpr size_t MAX_ID_LEN = 64;
    class DMObserver {
    public:
        virtual void Online(const std::string &device) = 0;
        virtual void Offline(const std::string &device) = 0;
    };
    static DMAdapter &GetInstance();
    bool Initialize(const std::string &pkgName);
    const std::string &GetLocalDevice();
    std::string GetDeviceName(const std::string &udid);
    void Register(DMObserver *observer);
    void Unregister(DMObserver *observer);

private:
    static constexpr const char *NAME_EX = "dm_adapter";
    static constexpr const char *DEVICE_INVALID_NAME = "unknown";
    DMAdapter();
    ~DMAdapter();

    std::string pkgName_;
    const std::string invalidDeviceId_{};
    mutable std::mutex mutex_{};
    std::string localDeviceId_{};
    ConcurrentMap<DMObserver *, DMObserver *> observers_;
};
} // namespace OHOS::MiscServices
#endif // OHOS_PASTEBOARD_SERVICES_DEVICE_DM_ADAPTER_H
