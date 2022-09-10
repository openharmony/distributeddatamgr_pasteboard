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

#include "paste_board_dialog.h"

#include "display_manager.h"
#include "napi_errcode.h"
#include "pasteboard_hilog_wreapper.h"
#include "ui_service_mgr_client.h"
namespace OHOS::MiscServicesNapi {
using namespace OHOS::MiscServices;
PasteBoardDialog &PasteBoardDialog::GetInstance()
{
    static PasteBoardDialog instance;
    return instance;
}

int32_t PasteBoardDialog::ShowDialog(std::shared_ptr<BlockObject<uint32_t>> block, const MessageInfo &message)
{
    int32_t id = -1;
    auto rect = GetDisplayRect();
    std::string params =
        std::string("{\"appName\":\"") + message.appName + "\", \"deviceType\":\"" + message.deviceType + "\"}";
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "pasting_dialog message:%{public}s.", params.c_str());
    Ace::UIServiceMgrClient::GetInstance()->ShowDialog(
        "pasting_dialog",
        params,
        OHOS::Rosen::WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW,
        rect.x, rect.y, rect.width, rect.height,
        [block](int32_t id, const std::string &event, const std::string &params) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "Event:%{public}s arrived.", event.c_str());
            if (event == std::string("EVENT_CANCEL")) {
                block->SetValue(E_CANCELED);
            }
        },
        &id);
    return id;
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
