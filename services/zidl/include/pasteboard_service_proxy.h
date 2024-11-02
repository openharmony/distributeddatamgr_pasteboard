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

#ifndef PASTE_BOARD_SERVICE_PROXY_H
#define PASTE_BOARD_SERVICE_PROXY_H

#include "i_pasteboard_observer.h"
#include "i_pasteboard_service.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace MiscServices {
class PasteboardServiceProxy : public IRemoteProxy<IPasteboardService> {
public:
    explicit PasteboardServiceProxy(const sptr<IRemoteObject> &object);
    ~PasteboardServiceProxy() = default;
    DISALLOW_COPY_AND_MOVE(PasteboardServiceProxy);
    virtual void Clear() override;
    virtual int32_t GetRecordValueByType(uint32_t dataId, uint32_t recordId, PasteDataEntry &value) override;
    virtual int32_t GetPasteData(PasteData &data, int32_t &syncTime) override;
    virtual bool HasPasteData() override;
    virtual std::set<Pattern> DetectPatterns(const std::set<Pattern> &patternsToCheck) override;
    virtual int32_t SetPasteData(PasteData &pasteData, const sptr<IPasteboardDelayGetter> delayGetter,
        const sptr<IPasteboardEntryGetter> entryGetter) override;
    virtual bool IsRemoteData() override;
    virtual int32_t GetDataSource(std::string &bundleName) override;
    virtual std::vector<std::string> GetMimeTypes() override;
    virtual bool HasDataType(const std::string &mimeType) override;
    virtual void SubscribeObserver(PasteboardObserverType type,
        const sptr<IPasteboardChangedObserver> &observer) override;
    virtual void UnsubscribeObserver(PasteboardObserverType type,
        const sptr<IPasteboardChangedObserver> &observer) override;
    virtual void UnsubscribeAllObserver(PasteboardObserverType type) override;
    virtual int32_t SetGlobalShareOption(const std::map<uint32_t, ShareOption> &globalShareOptions) override;
    virtual int32_t RemoveGlobalShareOption(const std::vector<uint32_t> &tokenIds) override;
    virtual std::map<uint32_t, ShareOption> GetGlobalShareOption(const std::vector<uint32_t> &tokenIds) override;
    virtual int32_t SetAppShareOptions(const ShareOption &shareOptions) override;
    virtual int32_t RemoveAppShareOptions() override;
    virtual void PasteStart(const std::string &pasteId) override;
    virtual void PasteComplete(const std::string &deviceId, const std::string &pasteId) override;
    virtual int32_t RegisterClientDeathObserver(sptr<IRemoteObject> observer) override;

private:
    static inline BrokerDelegator<PasteboardServiceProxy> delegator_;
    void ProcessObserver(uint32_t code, PasteboardObserverType type,
        const sptr<IPasteboardChangedObserver> &observer);
    static constexpr int32_t MAX_GET_GLOBAL_SHARE_OPTION_SIZE = 2000;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_SERVICE_PROXY_H