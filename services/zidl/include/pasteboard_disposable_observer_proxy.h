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
#ifndef PASTEBOARD_DISPOSBALE_OBSERVER_PROXY_H
#define PASTEBOARD_DISPOSBALE_OBSERVER_PROXY_H

#include "ipasteboard_disposable_observer.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {
class PasteboardDisposableObserverProxy : public IRemoteProxy<IPasteboardDisposableObserver> {
public:
    explicit PasteboardDisposableObserverProxy(const sptr<IRemoteObject> &object);
    ~PasteboardDisposableObserverProxy() = default;
    DISALLOW_COPY_AND_MOVE(PasteboardDisposableObserverProxy);
    void OnTextReceived(const std::string &text, int32_t errCode) override;

private:
    static inline BrokerDelegator<PasteboardDisposableObserverProxy> delegator_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_DISPOSBALE_OBSERVER_PROXY_H
