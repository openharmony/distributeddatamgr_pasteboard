/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_PASTE_SIGNAL_CALLBACK_H
#define OHOS_PASTE_SIGNAL_CALLBACK_H

#include "pasteboard_signal_stub.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
class PasteboardSignalCallback : public PasteboardSignalStub {
public:
    PasteboardSignalCallback()
    {}
    ~PasteboardSignalCallback()
    {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "deconstructor.");
    };

    void HandleProgressSignalValue(const std::string &signalValue) override;
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_PASTE_SIGNAL_CALLBACK_H