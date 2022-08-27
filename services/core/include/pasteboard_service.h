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

#include <singleton.h>
#include <sys/time.h>

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stack>
#include <thread>

#include "bundle_mgr_proxy.h"
#include "event_handler.h"
#include "i_pasteboard_observer.h"
#include "iremote_object.h"
#include "paste_data.h"
#include "pasteboard_dump_helper.h"
#include "pasteboard_service_stub.h"
#include "pasteboard_storage.h"
#include "system_ability.h"
#include "window_manager.h"

namespace OHOS {
namespace MiscServices {
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};
struct AppInfo {
    std::string appId;
    std::string bundleName;
    int32_t tokenType;
};
class PasteboardService final : public SystemAbility,
                                public PasteboardServiceStub,
                                public std::enable_shared_from_this<PasteboardService> {
    DECLARE_SYSTEM_ABILITY(PasteboardService)

public:
    PasteboardService();
    ~PasteboardService();
    virtual void Clear() override;
    virtual bool GetPasteData(PasteData& data) override;
    virtual bool HasPasteData() override;
    virtual void SetPasteData(PasteData& pasteData) override;
    virtual void AddPasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer) override;
    virtual void RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer) override;
    virtual void RemoveAllChangedObserver() override;
    virtual void OnStart() override;
    virtual void OnStop() override;
    size_t GetDataSize(PasteData& data) const;
    bool GetBundleNameByUid(int32_t uid, std::string &bundleName);
    bool SetPasteboardHistory(int32_t uId, std::string state, std::string timeStamp);
    int Dump(int fd, const std::vector<std::u16string> &args) override;
    std::string DumpHistory() const;
    std::string DunmpData();

    class PasteboardFocusChangedListener : public Rosen::IFocusChangedListener {
    public:
        void OnFocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override;
        void OnUnfocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override;
    };

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
    void SetPasteDataDot(PasteData& pasteData);
    void GetPasteDataDot();
    std::string GetTime();
    static bool CheckPastePermission(const std::string &appId, ShareOption shareOption);
    static bool GetAppInfoByTokenId(int32_t tokenId, AppInfo &appInfo);
    static bool IsFocusOrDefaultIme(const std::string &currentAppId);
    ServiceRunningState state_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    std::shared_ptr<IPasteboardStorage> pasteboardStorage_ = nullptr;
    std::mutex clipMutex_;
    std::mutex observerMutex_;
    std::map<int32_t, std::shared_ptr<std::set<sptr<IPasteboardChangedObserver>, classcomp>>> observerMap_;
    const std::string filePath_ = "";
    std::map<int32_t, std::shared_ptr<PasteData>> clips_;
    sptr<Rosen::IFocusChangedListener> focusChangedListener_;
    static int32_t focusAppUid_;
    int32_t uIdForLastCopy_ = 0;
    std::string timeForLastCopy_;
    static std::vector<std::shared_ptr<std::string>> dataHistory_;
    static std::shared_ptr<Command> copyHistory;
    static std::shared_ptr<Command> copyData;
};
} // MiscServices
} // OHOS
#endif // PASTE_BOARD_SERVICE_H
