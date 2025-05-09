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

#ifndef PASTEBOARD_ENTRY_GETTER_PROXY_H
#define PASTEBOARD_ENTRY_GETTER_PROXY_H

#include "iremote_proxy.h"
#include "ipasteboard_entry_getter.h"

namespace OHOS {
namespace MiscServices {
class PasteboardEntryGetterProxy : public IRemoteProxy<IPasteboardEntryGetter> {
public:
    explicit PasteboardEntryGetterProxy(const sptr<IRemoteObject> &object);
    ~PasteboardEntryGetterProxy() = default;
    int32_t GetRecordValueByType(uint32_t recordId, PasteDataEntry& value) override;
private:
    int32_t MakeRequest(uint32_t recordId, PasteDataEntry& value, MessageParcel& request);
    static inline BrokerDelegator<PasteboardEntryGetterProxy> delegator_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_ENTRY_GETTER_PROXY_H