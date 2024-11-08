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

#include "pasteboard_delay_getter_client.h"
#include "pasteboard_utils.h"

namespace OHOS {
namespace MiscServices {
PasteboardDelayGetterClient::PasteboardDelayGetterClient(std::shared_ptr<PasteboardDelayGetter> delayGetter)
    :delayGetter_(delayGetter)
{
}

void PasteboardDelayGetterClient::GetPasteData(const std::string &type, PasteData &data)
{
    UDMF::UnifiedData unifiedData;
    delayGetter_->GetUnifiedData(type, unifiedData);
    auto delayData = PasteboardUtils::GetInstance().Convert(unifiedData);
    if (delayData != nullptr) {
        data = *delayData;
    }
}

void PasteboardDelayGetterClient::GetUnifiedData(const std::string &type, UDMF::UnifiedData &data)
{
}
} // namespace MiscServices
} // namespace OHOS