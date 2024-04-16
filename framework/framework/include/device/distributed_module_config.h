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

#ifndef PASTE_BOARD_DISTRIBUTE_MODULE_CONFIG_H
#define PASTE_BOARD_DISTRIBUTE_MODULE_CONFIG_H

#include "api/visibility.h"
#include <functional>
#include "device/dm_adapter.h"

namespace OHOS {
namespace MiscServices {
class API_EXPORT DistributedModuleConfig : protected DMAdapter::DMObserver {
public:
    using Observer = std::function<void(bool isOn)>;
    static bool IsOn();
    static void Watch(Observer observer);
    void Init();
    void DeInit();
protected:
    void Online(const std::string &device) override;
    void Offline(const std::string &device) override;
    void OnReady(const std::string &device) override;
private:
    static bool GetEnabledStatus();
    static void ForceNotify();
    static void Notify();
    static void GetDeviceNum();
    static Observer observer_;
    static bool status_;
    static size_t deviceNums_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_DISTRIBUTE_MODULE_CONFIG_H
