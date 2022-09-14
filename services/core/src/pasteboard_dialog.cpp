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

#include "pasteboard_dialog.h"

#include "display_manager.h"
#include "pasteboard_errcode.h"
#include "pasteboard_hilog_wreapper.h"
#include "ui_service_mgr_client.h"
#include "common/block_object.h"
namespace OHOS::MiscServices {
PasteBoardDialog &PasteBoardDialog::GetInstance()
{
    static PasteBoardDialog instance;
    return instance;
}

int32_t PasteBoardDialog::ShowDialog(const MessageInfo &message, const Cancel &cancel)
{
    auto rect = GetDisplayRect();
    std::string params =
        std::string("{\"appName\":\"") + message.appName + "\", \"deviceType\":\"" + message.deviceType + "\"}";
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "pasting_dialog message:%{public}s.", params.c_str());

    auto realId = std::make_shared<BlockObject<int32_t>>(-1, POPUP_INTERVAL);
    int32_t result = -1;
    Ace::UIServiceMgrClient::GetInstance()->ShowDialog(
        "pasting_dialog",
        params,
        OHOS::Rosen::WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW,
        rect.x, rect.y, rect.width, rect.height,
        [cancel, realId](int32_t id, const std::string &event, const std::string &params) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Id:%{public}d Event:%{public}s arrived.", id, event.c_str());
            if (event == std::string("EVENT_INIT") && realId) {
                realId->SetValue(id);
            }

            if (event == std::string("EVENT_CANCEL") && cancel) {
                cancel();
                PasteBoardDialog::GetInstance().CancelDialog(id);
            }
        },
        &result);
    return realId->GetValue();
}
int32_t PasteBoardDialog::CancelDialog(int32_t id)
{
    return Ace::UIServiceMgrClient::GetInstance()->CancelDialog(id);
}

PasteBoardDialog::Rect PasteBoardDialog::GetDisplayRect()
{
    Rect rect;
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    }

    if (display != nullptr) {
        rect.width = display->GetWidth();
        rect.height = display->GetHeight();
    }
    return rect;
}
} // namespace OHOS::MiscServicesNapi
