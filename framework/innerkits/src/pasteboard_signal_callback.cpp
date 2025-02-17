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

#include "pasteboard_client.h"
#include "pasteboard_error.h"
#include "pasteboard_signal_callback.h"

namespace OHOS {
namespace MiscServices {
void PasteboardSignalCallback::HandleProgressSignalValue(const std::string &signalValue)
{
    int32_t ret = PasteboardClient::GetInstance()->HandleSignalValue(signalValue);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "HandleProgressSignalValue finished: ret=%{public}d.", ret);
}
} // namespace MiscServices
} // namespace OHOS