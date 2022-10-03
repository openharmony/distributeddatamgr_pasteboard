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

#ifndef PASTEBOARD_INTERFACES_KITS_NAPI_SRC_PASTE_BOARD_DAILOG_H
#define PASTEBOARD_INTERFACES_KITS_NAPI_SRC_PASTE_BOARD_DAILOG_H
#include <functional>
#include <string>

#include "ability_manager_interface.h"
namespace OHOS::MiscServices {
class PasteBoardDialog {
public:
    struct MessageInfo {
        std::string appName{ "unknown" };
        std::string deviceType{ "unknown" };
    };
    static constexpr uint32_t POPUP_INTERVAL = 1;  // seconds
    static constexpr uint32_t MAX_LIFE_TIME = 300; // seconds
    static constexpr const char *DEFAULT_LABEL = "unknown";
    using Cancel = std::function<void()>;
    static PasteBoardDialog &GetInstance();
    int32_t ShowDialog(const MessageInfo &message, const Cancel &cancel);
    static sptr<AAFwk::IAbilityManager> GetAbilityManagerService();

private:
    static constexpr int32_t STATUS_BAR_HEIGHT = 72;
    static constexpr const char *PASTEBOARD_DIALOG_ACTION = "action.system.pasteboarddialog";
    static constexpr const char *PASTEBOARD_DIALOG_APP = "cn.openharmony.pasteboarddialog";
    static constexpr const char *PASTEBOARD_DIALOG_ABILITY = "DialogExtensionAbility";
};
} // namespace OHOS::MiscServices
#endif // PASTEBOARD_INTERFACES_KITS_NAPI_SRC_PASTE_BOARD_DAILOG_H
