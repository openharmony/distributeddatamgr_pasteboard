/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#include "ipc_skeleton.h"
#include "iremote_stub.h"
#include "i_pasteboard_delay_getter.h"
#include "i_pasteboard_service.h"

namespace OHOS {
namespace MiscServices {
class PasteboardServiceStub : public IRemoteStub<IPasteboardService> {
public:
    PasteboardServiceStub();
    ~PasteboardServiceStub();
    API_EXPORT int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using PasteboardServiceFunc = int32_t (PasteboardServiceStub::*)(MessageParcel &data, MessageParcel &reply);
    virtual int32_t SavePasteData(std::shared_ptr<PasteData> &pasteData,
        sptr<IPasteboardDelayGetter> delayGetter = nullptr) = 0;
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
    int32_t OnIsRemoteData(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetDataSource(MessageParcel &data, MessageParcel &reply);
    int32_t OnHasDataType(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetGlobalShareOption(MessageParcel &data, MessageParcel &reply);
    int32_t OnRemoveGlobalShareOption(MessageParcel &data, MessageParcel &reply);
    int32_t OnGetGlobalShareOption(MessageParcel &data, MessageParcel &reply);
    int32_t OnSetAppShareOptions(MessageParcel &data, MessageParcel &reply);
    int32_t OnRemoveAppShareOptions(MessageParcel &data, MessageParcel &reply);
    inline bool IsObserverValid(MessageParcel &data, sptr<IPasteboardChangedObserver> &callback);

    std::map<uint32_t, PasteboardServiceFunc> memberFuncMap_;
    static constexpr uint32_t MAX_BUNDLE_NAME_LENGTH = 127;
    static constexpr int32_t MAX_SET_GLOBAL_SHARE_OPTION_SIZE = 100;
};
} // namespace MiscServices
} // namespace OHOS

#endif // PASTE_BOARD_SERVICE_STUB_H