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

#include "pasteboard_client.h"
#include "pasteboard_entry_getter.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::UDMF;
__attribute__((used)) PasteboardEntryGetter::Factory PasteboardEntryGetter::factory_;
PasteboardEntryGetter::Factory::Factory()
{
    GetterSystem::GetInstance().RegisterCreator("pasteboard", [this]() {
        if (getter_ == nullptr) {
            getter_ = std::make_shared<PasteboardEntryGetter>();
        }
        return getter_;
    });
}

PasteboardEntryGetter::Factory::~Factory()
{
    getter_ = nullptr;
}

UDMF::ValueType PasteboardEntryGetter::GetValueByType(uint32_t dataId, uint32_t recordId, const std::string &utdId)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "dataId:%{public}d. recordId:%{public}d, utdId:%{public}s", dataId,
        recordId, utdId.c_str());
    auto pasteType = CommonUtils::Convert2MimeType(utdId);
    PasteDataEntry entryValue;
    entryValue.SetUtdId(utdId);
    entryValue.SetMimeType(pasteType);
    auto result = PasteboardClient::GetInstance()->GetRecordValueByType(dataId, recordId, entryValue);
    if (result != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "get entry value fail, result:%{public}d", result);
    }
    return entryValue.GetValue();
}
} // namespace MiscServices
} // namespace OHOS