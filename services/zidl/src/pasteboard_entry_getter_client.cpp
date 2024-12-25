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

#include "pasteboard_entry_getter_client.h"
#include "convert_utils.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::UDMF;
PasteboardEntryGetterClient::PasteboardEntryGetterClient(
    const std::map<uint32_t, std::shared_ptr<EntryGetter>> entryGetters)
    : entryGetters_(entryGetters)
{
}

int32_t PasteboardEntryGetterClient::GetRecordValueByType(uint32_t recordId, PasteDataEntry &value)
{
    auto it = entryGetters_.find(recordId);
    if (it == entryGetters_.end()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "recordId:%{public}d, have no entry getter", recordId);
        return static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR);
    }
    auto utdId = value.GetUtdId();
    if (it->second != nullptr) {
        value.SetValue(it->second->GetValueByType(utdId));
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}
} // namespace MiscServices
} // namespace OHOS