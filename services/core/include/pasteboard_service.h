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

#ifndef PASTE_BOARD_SERVICE_H
#define PASTE_BOARD_SERVICE_H
#include <iremote_object.h>
#include <system_ability.h>

#include <memory>
#include <mutex>
#include <set>
#include <map>
#include <atomic>
#include <thread>
#include <stack>
#include <condition_variable>
#include <singleton.h>
#include "event_handler.h"
#include "i_pasteboard_observer.h"
#include "paste_data.h"
#include "pasteboard_storage.h"
#include "pasteboard_service_stub.h"

namespace OHOS {
namespace MiscServices {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};

class PasteboardService final : public SystemAbility,
                                public PasteboardServiceStub,
                                public std::enable_shared_from_this<PasteboardService> {
    DECLARE_SYSTEM_ABILITY(PasteboardService)
    DECLARE_DELAYED_SINGLETON(PasteboardService)

public:
    virtual void Clear() override;
    virtual PasteData GetPasteData() override;
    virtual bool HasPasteData() override;
    virtual void SetPasteData(PasteData& pasteData) override;
    virtual void AddPasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer) override;
    virtual void RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer) override;
    virtual void OnStart() override;
    virtual void OnStop() override;
private:
    struct classcomp {
        bool operator() (const sptr<IPasteboardChangedObserver>& l, const sptr<IPasteboardChangedObserver>& r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };
    int32_t Init();
    int32_t GetUserId();
    void NotifyObservers();
    void InitServiceHandler();
    void InitStorage();
    ServiceRunningState state_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    std::shared_ptr<IPasteboardStorage> pasteboardStorage_ = nullptr;
    std::mutex clipMutex_;
    std::mutex observerMutex_;
    std::map<int32_t, std::shared_ptr<std::set<const sptr<IPasteboardChangedObserver>, classcomp>>> observerMap_;
    const std::string filePath_ = "";
    std::map<int32_t, std::shared_ptr<PasteData>> clips_;
};
} // MiscServices
} // OHOS
#endif // PASTE_BOARD_SERVICE_H
