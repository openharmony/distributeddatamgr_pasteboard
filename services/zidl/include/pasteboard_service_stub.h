/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#ifndef PASTE_BOARD_SERVICE_STUB_H
#define PASTE_BOARD_SERVICE_STUB_H

#include <cstdint>
#include <map>

#include "i_pasteboard_service.h"
#include "ipc_skeleton.h"
#include "iremote_stub.h"

namespace OHOS {
namespace MiscServices {
class PasteboardServiceStub : public IRemoteStub<IPasteboardService> {
public:
    PasteboardServiceStub();
    ~PasteboardServiceStub();
    API_EXPORT int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;

private:
    using PasteboardServiceFunc = int32_t (PasteboardServiceStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t OnClear(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetPasteData(MessageParcel &data, MessageParcel &reply);
    int32_t OnHasPasteData(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetPasteData(MessageParcel &data, MessageParcel &reply);
    int32_t OnAddPasteboardChangedObserver(MessageParcel &data, MessageParcel &reply);
    int32_t OnRemovePasteboardChangedObserver(MessageParcel &data, MessageParcel &reply);
    int32_t OnRemoveAllChangedObserver(MessageParcel &data, MessageParcel &reply);
    int32_t OnAddPasteboardEventObserver(MessageParcel &data, MessageParcel &reply);
    int32_t OnRemovePasteboardEventObserver(MessageParcel &data, MessageParcel &reply);
    int32_t OnRemoveAllEventObserver(MessageParcel &data, MessageParcel &reply);
    inline bool IsObserverValid(MessageParcel &data, sptr<IPasteboardChangedObserver> &callback);

    std::map<uint32_t, PasteboardServiceFunc> memberFuncMap_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // PASTE_BOARD_SERVICE_STUB_H