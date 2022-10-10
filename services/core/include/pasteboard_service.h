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

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stack>
#include <sys/time.h>
#include <thread>

#include "clip/clip_plugin.h"
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
const std::int32_t ERROR_USERID = -1;
enum class ServiceRunningState {
    STATE_NOT_START,
    STATE_RUNNING
};
struct AppInfo {
    std::string bundleName = "com.pasteboard.default";
    int32_t tokenType = -1;
    int32_t userId = ERROR_USERID;
};
class PasteboardService final : public SystemAbility, public PasteboardServiceStub {
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
    bool SetPasteboardHistory(const std::string &bundleName, std::string state, std::string timeStamp);
    int Dump(int fd, const std::vector<std::u16string> &args) override;

    class PasteboardFocusChangedListener : public Rosen::IFocusChangedListener {
    public:
        void OnFocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override;
        void OnUnfocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo) override;
    };

private:
    using Event = ClipPlugin::GlobalEvent;
    static constexpr const char *PLUGIN_NAME = "distributed_clip";
    static constexpr uint32_t EXPIRATION_INTERVAL = 2;
    struct classcomp {
        bool operator() (const sptr<IPasteboardChangedObserver>& l, const sptr<IPasteboardChangedObserver>& r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };
    void AddSysAbilityListener();
    void AddWmsSysAbilityListener(uint32_t times);
    int32_t Init();
    int32_t GetUserIdByToken(uint32_t tokenId);
    std::string DumpHistory() const;
    std::string DumpData();
    void NotifyObservers();
    void InitServiceHandler();
    void InitStorage();
    bool IsCopyable(uint32_t tokenId) const;
    void SetPasteDataDot(PasteData& pasteData, uint32_t tokenId);
    void GetPasteDataDot(uint32_t tokenId);
    bool GetPasteData(PasteData& data, uint32_t tokenId, int32_t pid);
    std::string GetAppLabel(uint32_t tokenId);
    std::string GetDeviceName();
    void SetDeviceName(const std::string &device = "");

    std::shared_ptr<PasteData> GetDistributedData(int32_t user);
    bool SetDistributedData(int32_t user, PasteData& data);
    bool HasDistributedData(int32_t user);
    bool GetDistributedEvent(std::shared_ptr<ClipPlugin> plugin, int32_t user, Event &event);
    bool CleanDistributedData(int32_t user);
    void OnConfigChange(bool isOn);
    std::shared_ptr<ClipPlugin> GetClipPlugin();

    std::string GetTime();
    static bool HasPastePermission(uint32_t tokenId, int32_t pid, std::shared_ptr<PasteData> pasteData);
    static AppInfo GetAppInfo(uint32_t tokenId);
    static bool IsFocusOrDefaultIme(const AppInfo &appInfo, int32_t pid);
    static void SetLocalPasteFlag(bool isCrossPaste, uint32_t tokenId, PasteData &pasteData);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void RegisterFocusListener();

    ServiceRunningState state_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    std::shared_ptr<IPasteboardStorage> pasteboardStorage_ = nullptr;
    std::mutex clipMutex_;
    std::mutex observerMutex_;
    std::map<int32_t, std::shared_ptr<std::set<sptr<IPasteboardChangedObserver>, classcomp>>> observerMap_;
    ClipPlugin::GlobalEvent currentEvent_;
    const std::string filePath_ = "";
    std::map<int32_t, std::shared_ptr<PasteData>> clips_;
    sptr<Rosen::IFocusChangedListener> focusChangedListener_;

    std::recursive_mutex mutex;
    std::shared_ptr<ClipPlugin> clipPlugin_ = nullptr;
    std::atomic<uint32_t> sequenceId_ = 0;
    static int32_t focusApp_;
    uint32_t lastCopyApp_ = 0;
    std::string timeForLastCopy_;
    static std::mutex historyMutex_;
    static std::vector<std::string> dataHistory_;
    static std::shared_ptr<Command> copyHistory;
    static std::shared_ptr<Command> copyData;
    std::atomic<bool> pasting_ = false;

    std::mutex deviceMutex_;
    std::string fromDevice_;
};
} // MiscServices
} // OHOS
#endif // PASTE_BOARD_SERVICE_H
