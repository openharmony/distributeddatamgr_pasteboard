/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "pasteboard_switch.h"
#include "datashare_delegate.h"
#include "pasteboard_hilog.h"
#include "dev_profile.h"

#include <memory>
#include <string>

namespace OHOS::MiscServices {
const constexpr char* DISTRIBUTED_PASTEDBOARD_SWITCH = "distributed_pasteboard_switch";
PastedSwitch::PastedSwitch()
{
    switchObserver_ = std::make_shared<PastedSwitchObserver>(
        [this](const PastedSwitchObserver::ChangeInfo &changeInfo)-> void {
            SetSwitch();
        }
    );
}

void PastedSwitch::Init()
{
    DataShareDelegate::GetInstance().RegisterObserver(DISTRIBUTED_PASTEDBOARD_SWITCH, switchObserver_);
    SetSwitch();
}

void PastedSwitch::SetSwitch()
{
    std::string value;
    DataShareDelegate::GetInstance().GetValue(DISTRIBUTED_PASTEDBOARD_SWITCH, value);
    if (!value.empty()) {
        DevProfile::GetInstance().PutEnabledStatus(value);
    }
}

void PastedSwitch::DeInit()
{
    DataShareDelegate::GetInstance().UnregisterObserver(DISTRIBUTED_PASTEDBOARD_SWITCH, switchObserver_);
}

void PastedSwitchObserver::OnChange(const ChangeInfo &changeInfo)
{
    if (func_ != nullptr) {
        func_(changeInfo);
    }
}
} // namespace OHOS::MiscServices