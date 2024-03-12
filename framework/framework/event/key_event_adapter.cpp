/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "input_manager.h"
#include "key_option.h"
#include "event/key_event_adapter.h"
#include "pasteboard_hilog.h"

namespace OHOS::MiscServices {
KeyEventAdapter& KeyEventAdapter::GetInstance()
{
    static KeyEventAdapter keyEventAdapter;
    return keyEventAdapter;
}

void KeyEventAdapter::SubscribePasteEvent(const std::vector<int32_t>& keyCodes,
    const std::function<void(std::shared_ptr<MMI::KeyEvent>)>& callback)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "SubscribePasteEvent enter.");
    auto* inputManager = MMI::InputManager::GetInstance();
    for (auto keyCode : keyCodes) {
        std::set<int32_t> preKeys;
        preKeys.insert(keyCode);
        auto keyOption = std::make_shared<MMI::KeyOption>();
        keyOption->SetPreKeys(preKeys);
        keyOption->SetFinalKey(MMI::KeyEvent::KEYCODE_V);
        keyOption->SetFinalKeyDown(false);
        keyOption->SetFinalKeyDownDuration(0);
        if (inputManager->SubscribeKeyEvent(keyOption, callback) < 0) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "SubscribePasteEvent failed.");
        }
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "SubscribePasteEvent success.");
    }
}

}
