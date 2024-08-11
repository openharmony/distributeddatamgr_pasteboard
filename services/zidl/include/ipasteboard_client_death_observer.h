#ifndef I_PASTEBOARD_CLIENT_DEATH_OBSERVER_H
#define I_PASTEBOARD_CLIENT_DEATH_OBSERVER_H

#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
namespace OHOS {
namespace MiscServices {
class IPasteboardClientDeathObserver : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.DataShare.IDataShareClientDeathObserver");
};

class PasteboardClientDeathObserverStub : public IRemoteStub {
public:
    PasteboardClientDeathObserverStub();
    virtual ~PasteboardClientDeathObserverStub();
};

class PasteboardClientDeathObserverProxy : public IRemoteProxy {
public:
    explicit PasteboardClientDeathObserverProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy(impl){};
    ~PasteboardClientDeathObserverProxy() = default;

private:
    static inline BrokerDelegator delegator_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // I_PASTEBOARD_CLIENT_DEATH_OBSERVER_H