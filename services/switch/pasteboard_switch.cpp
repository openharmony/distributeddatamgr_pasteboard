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
#include "pasteboard_event_ue.h"

#include <memory>
#include <string>

namespace OHOS::MiscServices {
using namespace UeReporter;
const constexpr char* DISTRIBUTED_PASTEDBOARD_SWITCH = "distributed_pasteboard_switch";
constexpr const char *SUPPORT_STATUS = "1";
PastedSwitch::PastedSwitch()
{
    switchObserver_ = new (std::nothrow) PastedSwitchObserver(
        [this]()-> void {
            SetSwitch();
        }
    );
}

void PastedSwitch::Init()
{
    DataShareDelegate::GetInstance().RegisterObserver(DISTRIBUTED_PASTEDBOARD_SWITCH, switchObserver_);
    SetSwitch();
    ReportUeSwitchEvent();
}

void PastedSwitch::SetSwitch()
{
    std::string value;
    DataShareDelegate::GetInstance().GetValue(DISTRIBUTED_PASTEDBOARD_SWITCH, value);
    if (value.empty()) {
        DevProfile::GetInstance().PutEnabledStatus(SUPPORT_STATUS);
        return;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "set switch status to %{public}s.", value.c_str());
    DevProfile::GetInstance().PutEnabledStatus(value);
}

void PastedSwitch::DeInit()
{
    DataShareDelegate::GetInstance().UnregisterObserver(DISTRIBUTED_PASTEDBOARD_SWITCH, switchObserver_);
}

void PastedSwitch::ReportUeSwitchEvent()
{
    std::string value;
    DataShareDelegate::GetInstance().GetValue(DISTRIBUTED_PASTEDBOARD_SWITCH, value);
    UE_SWITCH(UeReporter::UE_SWITCH_STATUS, UeReporter::UE_STATUS_TYPE,
        (value == SUPPORT_STATUS) ? UeReporter::SwitchStatus::SWITCH_OPEN
                                  : UeReporter::SwitchStatus::SWITCH_CLOSE);
}

void PastedSwitchObserver::OnChange()
{
    if (func_ != nullptr) {
        func_();
    }
}
} // namespace OHOS::MiscServices