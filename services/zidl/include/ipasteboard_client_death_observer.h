#ifndef I_PASTEBOARD_CLIENT_DEATH_OBSERVER_H
#define I_PASTEBOARD_CLIENT_DEATH_OBSERVER_H

#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
namespace OHOS {
namespace MiscServices {
class IPasteboardClientDeathObserver : public IRemoteBroker<IPasteboardClientDeathObserver> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DataShare.IDataShareClientDeathObserver");
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