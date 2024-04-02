/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <mutex>
#include <string>

#include "ability_manager_interface.h"
#include "refbase.h"

namespace OHOS::MiscServices {
class DialogConnection;
class PasteBoardDialog {
public:
    struct ToastMessageInfo {
        std::string appName{ DEFAULT_LABEL };
    };
    static constexpr uint32_t POPUP_INTERVAL = 1000;  // milliseconds
    static constexpr uint32_t MAX_LIFE_TIME = 300 * 1000; // milliseconds
    static constexpr uint32_t SHOW_TOAST_TIME = 3000; // milliseconds
    static constexpr const char *DEFAULT_LABEL = "unknown";
    using Cancel = std::function<void()>;
    static PasteBoardDialog &GetInstance();
    int32_t ShowToast(const ToastMessageInfo &message);
    void CancelToast();

private:
    static sptr<OHOS::AAFwk::IAbilityManager> GetAbilityManagerService();

    static constexpr const char *PASTEBOARD_DIALOG_APP = "com.ohos.pasteboarddialog";
    static constexpr const char *PASTEBOARD_TOAST_ABILITY = "ToastExtensionAbility";

    std::mutex connectionLock_;
    sptr<DialogConnection> connection_;
};
} // namespace OHOS::MiscServices
#endif // PASTEBOARD_INTERFACES_KITS_NAPI_SRC_PASTE_BOARD_DAILOG_H
