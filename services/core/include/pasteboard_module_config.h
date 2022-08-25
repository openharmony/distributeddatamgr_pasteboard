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

#ifndef PASTE_BOARD_DISTRIBUTE_MODULE_CONFIG_H
#define PASTE_BOARD_DISTRIBUTE_MODULE_CONFIG_H

#include <functional>

namespace OHOS {
namespace MiscServices {
class PasteboardModuleConfig {
public:
    using SwitchObserver = std::function<void(bool isSwitchOn)>;
    static bool IsDistributedModuleOn();
    static void WatchDistributedModuleSwitch(SwitchObserver observer);
    static void NotifySwitchObserver(bool isSwitchOn);

private:
    static SwitchObserver observer_;
    static bool isSwitchOn_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_DISTRIBUTE_MODULE_CONFIG_H
