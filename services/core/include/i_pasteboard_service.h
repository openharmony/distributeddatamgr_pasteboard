/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "i_pasteboard_delay_getter.h"
#include "i_pasteboard_observer.h"
#include "iremote_broker.h"
#include "paste_data.h"

namespace OHOS {
namespace MiscServices {
class IPasteboardService : public IRemoteBroker {
public:
    virtual void Clear() = 0;
    virtual int32_t GetPasteData(PasteData &data) = 0;
    virtual bool HasPasteData() = 0;
    virtual int32_t SetPasteData(PasteData &pasteData, const sptr<IPasteboardDelayGetter> delayGetter) = 0;
    virtual bool IsRemoteData() = 0;
    virtual int32_t GetDataSource(std::string &bundleName) = 0;
    virtual bool HasDataType(const std::string &mimeType) = 0;
    virtual void AddPasteboardChangedObserver(const sptr<IPasteboardChangedObserver> &observer) = 0;
    virtual void RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver> &observer) = 0;
    virtual void RemoveAllChangedObserver() = 0;
    virtual void AddPasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer) = 0;
    virtual void RemovePasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer) = 0;
    virtual void RemoveAllEventObserver() = 0;
    virtual int32_t SetGlobalShareOption(const std::map<uint32_t, ShareOption> &globalShareOptions) = 0;
    virtual int32_t RemoveGlobalShareOption(const std::vector<uint32_t> &tokenIds) = 0;
    virtual std::map<uint32_t, ShareOption> GetGlobalShareOption(const std::vector<uint32_t> &tokenIds) = 0;
    virtual int32_t SetAppShareOptions(const ShareOption &shareOptions) = 0;
    virtual int32_t RemoveAppShareOptions() = 0;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.pasteboard.IPasteboardService");
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_SERVICE_INTERFACE_H