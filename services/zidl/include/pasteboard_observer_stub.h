/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef PASTE_BOARD_OBERVER_STUB_H
#define PASTE_BOARD_OBERVER_STUB_H

#include <map>

#include "i_pasteboard_service.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"

namespace OHOS {
namespace MiscServices {
class PasteboardObserverStub : public IRemoteStub<IPasteboardChangedObserver> {
public:
    PasteboardObserverStub();
    ~PasteboardObserverStub();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using PasteboardObserverFunc = int32_t (PasteboardObserverStub::*)(MessageParcel &data, MessageParcel &reply);

    virtual int32_t OnPasteboardChangedStub(MessageParcel &data, MessageParcel &reply);
    virtual int32_t OnPasteboardEventStub(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, PasteboardObserverFunc> memberFuncMap_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_OBERVER_STUB_H
