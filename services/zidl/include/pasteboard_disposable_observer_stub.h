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

#ifndef PASTEBOARD_DISPOSABLE_OBSERVER_STUB_H
#define PASTEBOARD_DISPOSABLE_OBSERVER_STUB_H

#include <unordered_map>

#include "ipasteboard_disposable_observer.h"
#include "ipasteboard_service.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"

namespace OHOS {
namespace MiscServices {
class API_EXPORT PasteboardDisposableObserverStub : public IRemoteStub<IPasteboardDisposableObserver> {
public:
    PasteboardDisposableObserverStub();
    virtual ~PasteboardDisposableObserverStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;

private:
    using PasteboardDisposableObserverFunc =
        int32_t (PasteboardDisposableObserverStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t OnTextReceivedStub(MessageParcel &data, MessageParcel &reply);
    std::unordered_map<uint32_t, PasteboardDisposableObserverFunc> memberFuncMap_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_DISPOSABLE_OBSERVER_STUB_H
