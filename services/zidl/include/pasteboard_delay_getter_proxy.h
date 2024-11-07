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

#ifndef OHOS_PASTEBOARD_DELAY_GETTER_PROXY_H
#define OHOS_PASTEBOARD_DELAY_GETTER_PROXY_H

#include "iremote_object.h"
#include "iremote_proxy.h"
#include "i_pasteboard_delay_getter.h"

namespace OHOS {
namespace MiscServices {
class PasteboardDelayGetterProxy : public IRemoteProxy<IPasteboardDelayGetter> {
public:
    explicit PasteboardDelayGetterProxy(const sptr<IRemoteObject> &object);
    ~PasteboardDelayGetterProxy() = default;
    void GetPasteData(const std::string &type, PasteData &data) override;
    void GetUnifiedData(const std::string &type, UDMF::UnifiedData &data) override;
private:
    static inline BrokerDelegator<PasteboardDelayGetterProxy> delegator_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // OHOS_PASTEBOARD_DELAY_GETTER_PROXY_H