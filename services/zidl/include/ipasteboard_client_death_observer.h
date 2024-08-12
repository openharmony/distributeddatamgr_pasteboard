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

#ifndef I_PASTEBOARD_CLIENT_DEATH_OBSERVER_H
#define I_PASTEBOARD_CLIENT_DEATH_OBSERVER_H

#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
namespace OHOS {
namespace MiscServices {
class IPasteboardClientDeathObserver : public IRemoteBroker<IPasteboardClientDeathObserver> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.MiscServices.IPasteboardClientDeathObserver");
};

class PasteboardClientDeathObserverStub : public IRemoteStub<IPasteboardClientDeathObserver> {
public:
    PasteboardClientDeathObserverStub();
    virtual ~PasteboardClientDeathObserverStub();
};

class PasteboardClientDeathObserverProxy : public IRemoteProxy<IPasteboardClientDeathObserver> {
public:
    explicit PasteboardClientDeathObserverProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<IPasteboardClientDeathObserver>(impl){};
    ~PasteboardClientDeathObserverProxy() = default;

private:
    static inline BrokerDelegator<IPasteboardClientDeathObserver> delegator_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // I_PASTEBOARD_CLIENT_DEATH_OBSERVER_H