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

#ifndef PASTE_BOARD_SERVICE_INTERFACE_H
#define PASTE_BOARD_SERVICE_INTERFACE_H

#include "i_pasteboard_observer.h"
#include "iremote_broker.h"
#include "paste_data.h"

namespace OHOS {
namespace MiscServices {
class IPasteboardService : public IRemoteBroker {
public:
    enum {
        GET_PASTE_DATA = 0,
        HAS_PASTE_DATA = 1,
        SET_PASTE_DATA = 2,
        CLEAR_ALL = 3,
        ADD_CHANGED_OBSERVER = 4,
        DELETE_CHANGED_OBSERVER = 5,
        DELETE_ALL_CHANGED_OBSERVER = 6,
        ADD_EVENT_OBSERVER = 7,
        DELETE_EVENT_OBSERVER = 8,
        DELETE_ALL_EVENT_OBSERVER
    };
    virtual void Clear() = 0;
    virtual int32_t GetPasteData(PasteData& data) = 0;
    virtual bool HasPasteData() = 0;
    virtual int32_t SetPasteData(PasteData& pasteData) = 0;
    virtual void AddPasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer) = 0;
    virtual void RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer) = 0;
    virtual void RemoveAllChangedObserver() = 0;
    virtual void AddPasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer) = 0;
    virtual void RemovePasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer) = 0;
    virtual void RemoveAllEventObserver() = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.pasteboard.IPasteboardService");
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_SERVICE_INTERFACE_H