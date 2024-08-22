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

#ifndef PASTE_BOARD_SERVICE_H
#define PASTE_BOARD_SERVICE_H

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <stack>
#include <sys/time.h>
#include <system_ability_definition.h>
#include <thread>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "clip/clip_plugin.h"
#include "common/block_object.h"
#include "common/concurrent_map.h"
#include "distributed_module_config.h"
#include "eventcenter/event_center.h"
#include "pasteboard_switch.h"
#include "event_handler.h"
#include "iremote_object.h"
#include "i_pasteboard_delay_getter.h"
#include "i_pasteboard_observer.h"
#include "pasteboard_common_event_subscriber.h"
#include "paste_data.h"
#include "pasteboard_dump_helper.h"
#include "pasteboard_service_stub.h"
#include "system_ability.h"
#include "privacy_kit.h"
#include "input_manager.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace MiscServices {
const std::int32_t ERROR_USERID = -1;
const std::int32_t RESULT_OK = 0;
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };
struct AppInfo {
    std::string bundleName = "com.pasteboard.default";
    int32_t tokenType = -1;
    int32_t userId = ERROR_USERID;
    uint32_t tokenId;
};

struct HistoryInfo {
    std::string time;
    std::string bundleName;
    std::string state;
    std::string remote;
};

struct PasteDateTime {
    int32_t syncTime = 0;
    std::shared_ptr<PasteData> data;
};

class InputEventCallback : public MMI::IInputEventConsumer {
public:
    void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override;
    void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
    void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override;
    bool IsCtrlVProcess(uint32_t callingPid, bool isFocused);
    void Clear();
private:
    static constexpr uint32_t EVENT_TIME_OUT = 2000;
    mutable int32_t windowPid_;
    mutable uint64_t actionTime_;
    mutable std::shared_mutex inputEventMutex_;
};

class PasteboardService final : public SystemAbility, public PasteboardServiceStub {
    DECLARE_SYSTEM_ABILITY(PasteboardService)

public:
    API_EXPORT PasteboardService();
    API_EXPORT ~PasteboardService();
    virtual void Clear() override;
    virtual int32_t GetPasteData(PasteData &data, int32_t &syncTime) override;
    virtual bool HasPasteData() override;
    virtual int32_t SetPasteData(PasteData &pasteData, const sptr<IPasteboardDelayGetter> delayGetter) override;
    virtual bool IsRemoteData() override;
    virtual bool HasDataType(const std::string &mimeType) override;
    virtual int32_t GetDataSource(std::string &bundleNme) override;
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
    virtual void OnStart() override;
    virtual void OnStop() override;
    virtual void PasteStart(const int32_t pasteId) override;
    virtual void PasteComplete(const std::string &deviceId, const int32_t pasteId) override;
    virtual int32_t RegisterClientDeathObserver(sptr<IRemoteObject> observer) override;
    static int32_t currentUserId;
    static ScreenEvent currentScreenStatus;
    size_t GetDataSize(PasteData &data) const;
    bool SetPasteboardHistory(HistoryInfo &info);
    int Dump(int fd, const std::vector<std::u16string> &args) override;
    void NotifyDelayGetterDied(int32_t userId);
    bool IsFocusedApp(uint32_t tokenId);

private:
    using Event = ClipPlugin::GlobalEvent;
    using ServiceListenerFunc = void (PasteboardService::*)();
    static constexpr const int32_t LISTENING_SERVICE[] = { DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID,
        WINDOW_MANAGER_SERVICE_ID, MEMORY_MANAGER_SA_ID };
    static constexpr const char *PLUGIN_NAME = "distributed_clip";
    static constexpr uint32_t PLAIN_INDEX = 0;
    static constexpr uint32_t HTML_INDEX = 1;
    static constexpr uint32_t URI_INDEX = 2;
    static constexpr uint32_t WANT_INDEX = 3;
    static constexpr uint32_t PIXELMAP_INDEX = 4;
    static constexpr uint32_t MAX_INDEX_LENGTH = 8;
    static constexpr const pid_t EDM_UID = 3057;
    static constexpr const pid_t ROOT_UID = 0;
    static constexpr uint32_t EXPIRATION_INTERVAL = 2;
    static constexpr int MIN_TRANMISSION_TIME = 30 * 1000; //ms
    static constexpr uint64_t ONE_HOUR_MILLISECONDS = 60 * 60 * 1000;
    static constexpr uint32_t GET_REMOTE_DATA_WAIT_TIME = 4000;
    class DelayGetterDeathRecipient final : public IRemoteObject::DeathRecipient {
    public:
        explicit DelayGetterDeathRecipient(int32_t userId, PasteboardService &service);
        virtual ~DelayGetterDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    private:
        int32_t userId_ = ERROR_USERID;
        PasteboardService &service_;
    };

    class RemoteDataTaskManager {
    public:
        struct TaskContext {
            std::atomic<bool> pasting_ = false;
            ConcurrentMap<uint32_t, std::shared_ptr<BlockObject<bool>>>  getDataBlocks_;
            std::shared_ptr<PasteDateTime>  data_;
        };
        using DataTask = std::pair<std::shared_ptr<PasteboardService::RemoteDataTaskManager::TaskContext>, bool>;
        DataTask GetRemoteDataTask(const Event &event);
        void Notify(const Event &event, std::shared_ptr<PasteDateTime> data);
        void ClearRemoteDataTask(const Event &event);
        std::shared_ptr<PasteDateTime> WaitRemoteData(const Event &event);
    private:
        std::atomic<uint32_t> mapKey_ = 0;
        std::mutex mutex_;
        std::map<std::string, std::shared_ptr<TaskContext>> dataTasks_;
    };

    struct classcomp {
        bool operator()(const sptr<IPasteboardChangedObserver> &l, const sptr<IPasteboardChangedObserver> &r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };
    using ObserverMap = std::map<int32_t, std::shared_ptr<std::set<sptr<IPasteboardChangedObserver>, classcomp>>>;
    void AddSysAbilityListener();
    int32_t Init();
    void InitScreenStatus();
    static int32_t GetCurrentAccountId();
    static ScreenEvent GetCurrentScreenStatus();
    std::string DumpHistory() const;
    std::string DumpData();
    void NotifyObservers(std::string bundleName, PasteboardEventStatus status);
    void InitServiceHandler();
    bool IsCopyable(uint32_t tokenId) const;

    int32_t SavePasteData(std::shared_ptr<PasteData> &pasteData,
        sptr<IPasteboardDelayGetter> delayGetter = nullptr) override;
    void RemovePasteData(const AppInfo &appInfo);
    void SetPasteDataDot(PasteData &pasteData);
    std::pair<bool, ClipPlugin::GlobalEvent> GetValidDistributeEvent(int32_t user);
    int32_t GetSdkVersion(uint32_t tokenId);
    bool IsPermissionGranted(const std::string& perm, uint32_t tokenId);
    int32_t GetData(uint32_t tokenId, PasteData &data, int32_t &syncTime);

    void GetPasteDataDot(PasteData &pasteData, const std::string &bundleName);
    int32_t GetLocalData(const AppInfo &appInfo, PasteData &data);
    int32_t GetRemoteData(int32_t userId, const Event &event, PasteData &data, int32_t &syncTime);
    int32_t GetRemotePasteData(int32_t userId, const Event &event, PasteData &data, int32_t &syncTime);
    void GetDelayPasteData(const AppInfo &appInfo, PasteData &data);
    void CheckUriPermission(PasteData &data, std::vector<Uri> &grantUris, const std::string &targetBundleName);
    int32_t GrantUriPermission(PasteData &data, const std::string &targetBundleName);
    void RevokeUriPermission(std::shared_ptr<PasteData> pasteData);
    void GenerateDistributedUri(PasteData &data);
    bool IsBundleOwnUriPermission(const std::string &bundleName, Uri &uri);
    void CheckAppUriPermission(PasteData &data);
    std::string GetAppLabel(uint32_t tokenId);
    sptr<OHOS::AppExecFwk::IBundleMgr> GetAppBundleManager();
    void EstablishP2PLink(const std::string& networkId, int32_t pasteId);
    void CloseP2PLink(const std::string& networkId);
    uint8_t GenerateDataType(PasteData &data);
    bool HasDistributedDataType(const std::string &mimeType);

    std::pair<std::shared_ptr<PasteData>, int32_t> GetDistributedData(const Event &event, int32_t user);
    bool SetDistributedData(int32_t user, PasteData &data);
    bool CleanDistributedData(int32_t user);
    void OnConfigChange(bool isOn);
    std::shared_ptr<ClipPlugin> GetClipPlugin();

    static std::string GetTime();
    bool IsDataAged();
    bool VerifyPermission(uint32_t tokenId);
    bool IsDataVaild(PasteData &pasteData, uint32_t tokenId);
    static AppInfo GetAppInfo(uint32_t tokenId);
    static std::string GetAppBundleName(const AppInfo &appInfo);
    bool IsDefaultIME(const AppInfo &appInfo);
    static void SetLocalPasteFlag(bool isCrossPaste, uint32_t tokenId, PasteData &pasteData);
    void ShowHintToast(uint32_t tokenId, uint32_t pid);
    void SetWebViewPasteData(PasteData &pasteData, const std::string &bundleName);
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void DMAdapterInit();
    void NotifySaStatus();

    ServiceRunningState state_;
    std::shared_ptr<AppExecFwk::EventHandler> serviceHandler_;
    std::mutex observerMutex_;
    ObserverMap observerLocalChangedMap_;
    ObserverMap observerRemoteChangedMap_;
    ObserverMap observerEventMap_;
    ClipPlugin::GlobalEvent currentEvent_;
    ClipPlugin::GlobalEvent remoteEvent_;
    const std::string filePath_ = "";
    ConcurrentMap<int32_t, std::shared_ptr<PasteData>> clips_;
    ConcurrentMap<int32_t, std::pair<sptr<IPasteboardDelayGetter>, sptr<DelayGetterDeathRecipient>>> delayGetters_;
    ConcurrentMap<int32_t, uint64_t> copyTime_;
    std::set<std::string> readBundles_;
    std::shared_ptr<PasteBoardCommonEventSubscriber> commonEventSubscriber_ = nullptr;

    std::recursive_mutex mutex;
    std::shared_ptr<ClipPlugin> clipPlugin_ = nullptr;
    std::atomic<uint16_t> sequenceId_ = 0;
    static std::mutex historyMutex_;
    static std::mutex bundleMutex_;
    static std::vector<std::string> dataHistory_;
    static std::shared_ptr<Command> copyHistory;
    static std::shared_ptr<Command> copyData;
    std::atomic<bool> setting_ = false;
    std::map<int32_t, ServiceListenerFunc> ServiceListenerFunc_;
    std::map<std::string, int> typeMap_ = {
        { MIMETYPE_TEXT_PLAIN, PLAIN_INDEX },
        { MIMETYPE_TEXT_HTML, HTML_INDEX },
        { MIMETYPE_TEXT_URI, URI_INDEX },
        { MIMETYPE_TEXT_WANT, WANT_INDEX },
        { MIMETYPE_PIXELMAP, PIXELMAP_INDEX }
    };

    std::shared_ptr<FFRTTimer> ffrtTimer_;
    std::atomic<int32_t> pasteId_ = 0;
    ConcurrentMap<std::string, ConcurrentMap<int32_t, int32_t>> p2pMap_;
    ConcurrentMap<uint32_t, ShareOption> globalShareOptions_;
    PastedSwitch switch_;

    void AddObserver(int32_t userId, const sptr<IPasteboardChangedObserver> &observer, ObserverMap &observerMap);
    void RemoveSingleObserver(int32_t userId, const sptr<IPasteboardChangedObserver> &observer,
        ObserverMap &observerMap);
    void RemoveAllObserver(int32_t userId, ObserverMap &observerMap);
    inline bool IsCallerUidValid();
    bool HasLocalDataType(const std::string &mimeType);
    void AddPermissionRecord(uint32_t tokenId, bool isReadGrant, bool isSecureGrant);
    bool SubscribeKeyboardEvent();
    bool IsAllowSendData();
    void UpdateShareOption(PasteData &pasteData);
    void PasteboardEventSubscriber();
    void CommonEventSubscriber();
    std::function<void(const OHOS::MiscServices::Event &)> RemotePasteboardChange();
    std::shared_ptr<InputEventCallback> inputEventCallback_;
    DistributedModuleConfig moduleConfig_;
    std::vector<std::string> bundles_;
    int32_t uid_ = -1;
    RemoteDataTaskManager  taskMgr_;
    pid_t setPasteDataUId_ = 0;
    static constexpr const pid_t TESE_SERVER_UID = 3500;
    std::mutex eventMutex_;
    class PasteboardClientDeathObserverImpl {
    public:
        PasteboardClientDeathObserverImpl(PasteboardService &service, sptr<IRemoteObject> observer);
        explicit PasteboardClientDeathObserverImpl(PasteboardService &service);
        explicit PasteboardClientDeathObserverImpl(PasteboardClientDeathObserverImpl &&impl);
        PasteboardClientDeathObserverImpl &operator=(PasteboardClientDeathObserverImpl &&impl);
        virtual ~PasteboardClientDeathObserverImpl();

        pid_t GetPid() const;
    private:
        class PasteboardDeathRecipient : public IRemoteObject::DeathRecipient {
        public:
            explicit PasteboardDeathRecipient(PasteboardClientDeathObserverImpl &pasteboardClientDeathObserverImpl);
            virtual ~PasteboardDeathRecipient();
            void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

        private:
            PasteboardClientDeathObserverImpl &pasteboardClientDeathObserverImpl_;
        };
        void Reset();
        pid_t uid_;
        pid_t pid_;
        uint32_t token_;
        PasteboardService &dataService_;
        sptr<IRemoteObject> observerProxy_;
        sptr<PasteboardDeathRecipient> deathRecipient_;
    };
    int32_t AppExit(pid_t uid, pid_t pid, uint32_t token);
    ConcurrentMap<pid_t, PasteboardClientDeathObserverImpl> clients_;
    static constexpr pid_t INVALID_UID = -1;
    static constexpr pid_t INVALID_PID = -1;
    static constexpr uint32_t INVALID_TOKEN = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_SERVICE_H