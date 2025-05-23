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

#ifndef I_PASTEBOARD_ENTRY_GETTER_H
#define I_PASTEBOARD_ENTRY_GETTER_H

#include "paste_data_entry.h"

namespace OHOS {
namespace MiscServices {
class IPasteboardEntryGetter : public IRemoteBroker {
public:
    virtual ~IPasteboardEntryGetter() = default;
    virtual int32_t GetRecordValueByType(uint32_t recordId, PasteDataEntry &value) = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.misc.services.pasteboard.IPasteboardEntryGetter");
};
} // namespace MiscServices
} // namespace OHOS
#endif // I_PASTEBOARD_ENTRY_GETTER_H