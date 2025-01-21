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

#ifndef PASTEBOARD_SIGNAL_STUB_H
#define PASTEBOARD_SIGNAL_STUB_H

#include "i_pasteboard_signal.h"
#include "iremote_stub.h"

namespace OHOS {
namespace MiscServices {
class PasteboardSignalStub : public IRemoteStub<IPasteboardSignal> {
public:
    PasteboardSignalStub();
    ~PasteboardSignalStub();

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    using PasteboardSignalFunc = int32_t (PasteboardSignalStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, PasteboardSignalFunc> memberFuncMap_;
    int32_t OnGetProgressSignal(MessageParcel &data, MessageParcel &reply);
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_SIGNAL_STUB_H
