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

#include <sys/time.h>
#include <system_ability_definition.h>

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stack>
#include <thread>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "clip/clip_plugin.h"
#include "event_handler.h"
#include "i_pasteboard_observer.h"
#include "iremote_object.h"
#include "paste_data.h"
#include "pasteboard_dump_helper.h"
#include "pasteboard_service_stub.h"
#include "system_ability.h"

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

struct HistoryInfo {
    std::string time;
    std::string bundleName;
    std::string state;
    std::string pop;
    std::string remote;
};

class PasteboardService final : public SystemAbility, public PasteboardServiceStub {
    DECLARE_SYSTEM_ABILITY(PasteboardService)

public:
    API_EXPORT PasteboardService();
    API_EXPORT ~PasteboardService();
    virtual void Clear() override;
    virtual int32_t GetPasteData(PasteData& data) override;
    virtual bool HasPasteData() override;
    virtual int32_t SetPasteData(PasteData& pasteData) override;
    virtual void AddPasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer) override;
    virtual void RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer) override;
    virtual void RemoveAllChangedObserver() override;
    virtual void AddPasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer) override;
    virtual void RemovePasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer) override;
    virtual void RemoveAllEventObserver() override;
    virtual void OnStart() override;
    virtual void OnStop() override;
    size_t GetDataSize(PasteData& data) const;
    bool SetPasteboardHistory(HistoryInfo &info);
    int Dump(int fd, const std::vector<std::u16string> &args) override;

private:
    using Event = ClipPlugin::GlobalEvent;
    using ServiceListenerFunc = void (PasteboardService::*)();
    static constexpr const int32_t LISTENING_SERVICE[] = { DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID,
        DISTRIBUTED_DEVICE_PROFILE_SA_ID, WINDOW_MANAGER_SERVICE_ID };
    static constexpr const char *PLUGIN_NAME = "distributed_clip";
    static constexpr const pid_t EDM_UID = 3057;
    static constexpr const pid_t ROOT_UID = 0;
    static constexpr uint32_t EXPIRATION_INTERVAL = 2;
    struct classcomp {
        bool operator() (const sptr<IPasteboardChangedObserver>& l, const sptr<IPasteboardChangedObserver>& r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };
    using ObserverMap = std::map<int32_t, std::shared_ptr<std::set<sptr<IPasteboardChangedObserver>, classcomp>>>;
    void AddSysAbilityListener();
    int32_t Init();
    int32_t GetUserIdByToken(uint32_t tokenId);
    std::string DumpHistory() const;
    std::string DumpData();
    void NotifyObservers(std::string bundleName, PasteboardEventStatus status);
    void InitServiceHandler();
    bool IsCopyable(uint32_t tokenId) const;

    void SetPasteDataDot(PasteData &pasteData);
    void GetPasteDataDot(PasteData &pasteData, const std::string &pop, uint32_t tokenId);
    bool GetPasteData(PasteData& data, uint32_t tokenId, bool isFocusedApp);
    std::string GetAppLabel(uint32_t tokenId);
    sptr<OHOS::AppExecFwk::IBundleMgr> GetAppBundleManager();
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
    static bool HasPastePermission(uint32_t tokenId, bool isFocusedApp, const std::shared_ptr<PasteData> &pasteData);
    static AppInfo GetAppInfo(uint32_t tokenId);
    static std::string GetAppBundleName(uint32_t tokenId);
    static bool IsDefaultIME(const AppInfo &appInfo);
    static bool IsFocusedApp(int32_t tokenId);
    static void SetLocalPasteFlag(bool isCrossPaste, uint32_t tokenId, PasteData &pasteData);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void DevManagerInit();
    void DevProfileInit();
    static void ShareOptionToString(ShareOption shareOption, std::string &out);
    ServiceRunningState state_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    std::mutex clipMutex_;
    std::mutex observerMutex_;
    ObserverMap observerChangedMap_;
    ObserverMap observerEventMap_;
    ClipPlugin::GlobalEvent currentEvent_;
    const std::string filePath_ = "";
    std::map<int32_t, std::shared_ptr<PasteData>> clips_;

    std::recursive_mutex mutex;
    std::shared_ptr<ClipPlugin> clipPlugin_ = nullptr;
    std::atomic<uint32_t> sequenceId_ = 0;
    static std::mutex historyMutex_;
    static std::vector<std::string> dataHistory_;
    static std::shared_ptr<Command> copyHistory;
    static std::shared_ptr<Command> copyData;
    std::atomic<bool> pasting_ = false;
    std::atomic<bool> setting_ = false;
    std::mutex deviceMutex_;
    std::string fromDevice_;
    std::map<int32_t, ServiceListenerFunc> ServiceListenerFunc_;

    void AddObserver(const sptr<IPasteboardChangedObserver> &observer, ObserverMap &observerMap);
    void RemoveSingleObserver(const sptr<IPasteboardChangedObserver> &observer, ObserverMap &observerMap);
    void RemoveAllObserver(ObserverMap &observerMap);
    inline bool IsCallerUidValid();
};
} // MiscServices
} // OHOS
#endif // PASTE_BOARD_SERVICE_H
