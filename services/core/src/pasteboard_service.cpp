/*
 * Copyright (C) 2021-2024 Huawei Device Co., Ltd.
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
#include "pasteboard_service.h"

#include <dlfcn.h>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "account_manager.h"
#include "calculate_time_consuming.h"
#include "common_event_manager.h"
#include "dev_profile.h"
#include "distributed_file_daemon_manager.h"
#ifdef WITH_DLP
#include "dlp_permission_kit.h"
#include "dlp_permission.h"
#endif // WITH_DLP
#include "eventcenter/pasteboard_event.h"
#include "hiview_adapter.h"
#include "input_method_controller.h"
#include "iservice_registry.h"
#include "mem_mgr_client.h"
#include "os_account_manager.h"
#include "parameters.h"
#include "pasteboard_dialog.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_event_dfx.h"
#include "pasteboard_event_ue.h"
#include "pasteboard_trace.h"
#include "pasteboard_web_controller.h"
#include "remote_file_share.h"
#include "res_sched_client.h"
#include "reporter.h"
#include "distributed_module_config.h"
#ifdef PB_SCREENLOCK_MGR_ENABLE
#include "screenlock_manager.h"
#endif // PB_SCREENLOCK_MGR_ENABLE
#include "tokenid_kit.h"
#include "uri_permission_manager_client.h"
#ifdef SCENE_BOARD_ENABLE
#include "window_manager_lite.h"
#else
#include "window_manager.h"
#endif // SCENE_BOARD_ENABLE

namespace OHOS {
namespace MiscServices {
using namespace Rosen;
using namespace std::chrono;
using namespace Storage::DistributedFile;
using namespace RadarReporter;
using namespace UeReporter;
namespace {
constexpr int32_t COMMON_USERID = 0;
constexpr int32_t INIT_INTERVAL = 10000L;
constexpr uint32_t MAX_IPC_THREAD_NUM = 32;
constexpr const char *PASTEBOARD_SERVICE_SA_NAME = "pasteboard_service";
constexpr const char *PASTEBOARD_SERVICE_NAME = "PasteboardService";
constexpr const char *NLU_SO_PATH = "libai_nlu_innerapi.z.so";
constexpr const char *GET_PASTE_DATA_PROCESSOR = "GetPasteDataProcessor";
constexpr const char *FAIL_TO_GET_TIME_STAMP = "FAIL_TO_GET_TIME_STAMP";
constexpr const char *SECURE_PASTE_PERMISSION = "ohos.permission.SECURE_PASTE";
constexpr const char *READ_PASTEBOARD_PERMISSION = "ohos.permission.READ_PASTEBOARD";
constexpr const char *TRANSMIT_CONTROL_PROP_KEY = "persist.distributed_scene.datafiles_trans_ctrl";
constexpr const char *MANAGE_PASTEBOARD_APP_SHARE_OPTION_PERMISSION =
    "ohos.permission.MANAGE_PASTEBOARD_APP_SHARE_OPTION";

constexpr int32_t INVALID_VERSION = -1;
constexpr int32_t ADD_PERMISSION_CHECK_SDK_VERSION = 12;
constexpr int32_t CTRLV_EVENT_SIZE = 2;
constexpr int32_t CONTROL_TYPE_ALLOW_SEND_RECEIVE = 1;
constexpr uint32_t EVENT_TIME_OUT = 2000;
constexpr uint32_t MAX_RECOGNITION_LENGTH = 1000;
constexpr int32_t DEVICE_COLLABORATION_UID = 5521;
constexpr uint64_t SYSTEM_APP_MASK = (static_cast<uint64_t>(1) << 32);

const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(new PasteboardService());
} // namespace
using namespace Security::AccessToken;
using namespace OHOS::AppFileService::ModuleRemoteFileShare;
std::mutex PasteboardService::historyMutex_;
std::vector<std::string> PasteboardService::dataHistory_;
std::shared_ptr<Command> PasteboardService::copyHistory;
std::shared_ptr<Command> PasteboardService::copyData;
int32_t PasteboardService::currentUserId_ = ERROR_USERID;
ScreenEvent PasteboardService::currentScreenStatus = ScreenEvent::Default;

PasteboardService::PasteboardService()
    : SystemAbility(PASTEBOARD_SERVICE_ID, true), state_(ServiceRunningState::STATE_NOT_START)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PasteboardService Start.");
    ServiceListenerFuncs_[static_cast<int32_t>(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID)] =
        &PasteboardService::DMAdapterInit;
    ServiceListenerFuncs_[static_cast<int32_t>(MEMORY_MANAGER_SA_ID)] = &PasteboardService::NotifySaStatus;
}

PasteboardService::~PasteboardService()
{
    clients_.Clear();
    UnsubscribeAllEntityObserver();
}

int32_t PasteboardService::Init()
{
    if (!Publish(this)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "OnStart register to system ability manager failed.");
        auto userId = GetCurrentAccountId();
        Reporter::GetInstance().PasteboardFault().Report({ userId, "ERR_INVALID_OPTION" });
        return static_cast<int32_t>(PasteboardError::INVALID_OPTION_ERROR);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Init Success.");
    state_ = ServiceRunningState::STATE_RUNNING;
    InitScreenStatus();
    return ERR_OK;
}

void PasteboardService::InitScreenStatus()
{
#ifdef PB_SCREENLOCK_MGR_ENABLE
    auto isScreenLocked = OHOS::ScreenLock::ScreenLockManager::GetInstance()->IsScreenLocked();
    PasteboardService::currentScreenStatus = isScreenLocked ? ScreenEvent::ScreenLocked : ScreenEvent::ScreenUnlocked;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "screen status is %{public}d", PasteboardService::currentScreenStatus);
#else
    PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "PB_SCREENLOCK_MGR_ENABLE not defined");
    return;
#endif
}

void PasteboardService::OnStart()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardService OnStart.");
    std::lock_guard<std::mutex> lock(saMutex_);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(state_ != ServiceRunningState::STATE_RUNNING,
        PASTEBOARD_MODULE_SERVICE, "PasteboardService is already running.");
    IPCSkeleton::SetMaxWorkThreadNum(MAX_IPC_THREAD_NUM);
    InitServiceHandler();
    auto appInfo = GetAppInfo(IPCSkeleton::GetCallingTokenID());
    Loader loader;
    uid_ = loader.LoadUid();
    moduleConfig_.Init();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "datasl on start ret:%{public}d", DATASL_OnStart());
    moduleConfig_.Watch(std::bind(&PasteboardService::OnConfigChange, this, std::placeholders::_1));
    AddSysAbilityListener();
    if (Init() != ERR_OK) {
        auto callback = [this]() {
            Init();
        };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Init failed. Try again 10s later.");
        return;
    }
    auto callback = [this]() {
        switch_.Init(GetCurrentAccountId());
    };
    serviceHandler_->PostTask(callback);
    copyHistory = std::make_shared<Command>(std::vector<std::string>{ "--copy-history" },
        "Dump access history last ten times.",
        [this](const std::vector<std::string> &input, std::string &output) -> bool {
            output = DumpHistory();
            return true;
        });
    copyData = std::make_shared<Command>(std::vector<std::string>{ "--data" }, "Show copy data details.",
        [this](const std::vector<std::string> &input, std::string &output) -> bool {
            output = DumpData();
            return true;
        });
    PasteboardDumpHelper::GetInstance().RegisterCommand(copyHistory);
    PasteboardDumpHelper::GetInstance().RegisterCommand(copyData);
    CommonEventSubscriber();
    AccountStateSubscriber();
    PasteboardEventSubscriber();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Start PasteboardService success.");
    EventCenter::GetInstance().Subscribe(OHOS::MiscServices::Event::EVT_REMOTE_CHANGE, RemotePasteboardChange());
    HiViewAdapter::StartTimerThread();
    ffrtTimer_ = std::make_shared<FFRTTimer>("pasteboard_service");
    return;
}

void PasteboardService::OnStop()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OnStop Started.");
    std::lock_guard<std::mutex> lock(saMutex_);
    if (state_ != ServiceRunningState::STATE_RUNNING) {
        return;
    }
    serviceHandler_ = nullptr;
    state_ = ServiceRunningState::STATE_NOT_START;
    DMAdapter::GetInstance().DeInitialize();
    if (commonEventSubscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(commonEventSubscriber_);
    }
    moduleConfig_.DeInit();
    switch_.DeInit();
    EventCenter::GetInstance().Unsubscribe(PasteboardEvent::DISCONNECT);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OnStop End.");
    EventCenter::GetInstance().Unsubscribe(OHOS::MiscServices::Event::EVT_REMOTE_CHANGE);
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(getpid(), 1, 0, PASTEBOARD_SERVICE_ID);
}

void PasteboardService::AddSysAbilityListener()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "begin.");
    for (uint32_t i = 0; i < sizeof(LISTENING_SERVICE) / sizeof(LISTENING_SERVICE[0]); i++) {
        auto ret = AddSystemAbilityListener(LISTENING_SERVICE[i]);
        if (ret) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Add listener success, serviceId = %{public}d.",
                LISTENING_SERVICE[i]);
        } else {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Add listener failed, serviceId = %{public}d.",
                LISTENING_SERVICE[i]);
        }
    }
}

void PasteboardService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "systemAbilityId = %{public}d added!", systemAbilityId);
    auto itFunc = ServiceListenerFuncs_.find(systemAbilityId);
    if (itFunc != ServiceListenerFuncs_.end()) {
        auto ServiceListenerFunc = itFunc->second;
        if (ServiceListenerFunc != nullptr) {
            (this->*ServiceListenerFunc)();
        }
    }
}

PasteboardService::DelayGetterDeathRecipient::DelayGetterDeathRecipient(int32_t userId, PasteboardService &service)
    : userId_(userId), service_(service)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Construct Delay Getter Death Recipient");
}

void PasteboardService::DelayGetterDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start");
    (void)remote;
    service_.NotifyDelayGetterDied(userId_);
}

void PasteboardService::NotifyDelayGetterDied(int32_t userId)
{
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "error userId: %{public}d", userId);
        return;
    }
    delayGetters_.Erase(userId);
}

PasteboardService::EntryGetterDeathRecipient::EntryGetterDeathRecipient(int32_t userId, PasteboardService &service)
    : userId_(userId), service_(service)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Construct Entry Getter Death Recipient");
}

void PasteboardService::EntryGetterDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start");
    (void)remote;
    service_.NotifyEntryGetterDied(userId_);
}

void PasteboardService::NotifyEntryGetterDied(int32_t userId)
{
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "error userId: %{public}d", userId);
        return;
    }
    entryGetters_.Erase(userId);
}

void PasteboardService::DMAdapterInit()
{
    auto appInfo = GetAppInfo(IPCSkeleton::GetCallingTokenID());
    DMAdapter::GetInstance().Initialize(appInfo.bundleName);
}

void PasteboardService::NotifySaStatus()
{
    Memory::MemMgrClient::GetInstance().NotifyProcessStatus(getpid(), 1, 1, PASTEBOARD_SERVICE_ID);
}

void PasteboardService::ReportUeCopyEvent(PasteData &pasteData, int32_t result)
{
    auto appInfo = GetAppInfo(IPCSkeleton::GetCallingTokenID());
    auto res = (result == static_cast<int32_t>(PasteboardError::E_OK)) ? UeReporter::E_OK_OPERATION : result;
    UE_REPORT(UeReporter::UE_COPY, GenerateDataType(pasteData), appInfo.bundleName, res,
        DMAdapter::GetInstance().GetLocalDeviceType());
}

void PasteboardService::InitServiceHandler()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "InitServiceHandler started.");
    if (serviceHandler_ != nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Already init.");
        return;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner =
        AppExecFwk::EventRunner::Create(PASTEBOARD_SERVICE_NAME, AppExecFwk::ThreadMode::FFRT);
    serviceHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);

    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "InitServiceHandler Succeeded.");
}

void PasteboardService::Clear()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "enter, clips_.Size=%{public}zu", clips_.Size());
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return;
    }
    RADAR_REPORT(DFX_CLEAR_PASTEBOARD, DFX_MANUAL_CLEAR, DFX_SUCCESS);
    auto it = clips_.Find(userId);
    if (it.first) {
        RevokeUriPermission(it.second);
        clips_.Erase(userId);
        delayDataId_ = 0;
        delayTokenId_ = 0;
        auto appInfo = GetAppInfo(IPCSkeleton::GetCallingTokenID());
        std::string bundleName = GetAppBundleName(appInfo);
        NotifyObservers(bundleName, PasteboardEventStatus::PASTEBOARD_CLEAR);
    }
    CleanDistributedData(userId);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "leave, clips_.Size=%{public}zu", clips_.Size());
}

int32_t PasteboardService::GetChangeCount(uint32_t &changeCount)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto appInfo = GetAppInfo(tokenId);
    changeCount = 0;
    clipChangeCount_.ComputeIfPresent(appInfo.userId, [&changeCount](auto, auto &value) {
        changeCount = value;
        PASTEBOARD_HILOGI(
            PASTEBOARD_MODULE_SERVICE, "Find changeCount succeed, changeCount is %{public}u", changeCount);
        return true;
    });
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardService::IncreaseChangeCount(int32_t userId)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "IncreaseChangeCount start!");
    clipChangeCount_.Compute(userId, [](auto userId, uint32_t &changeCount) {
        changeCount = (changeCount == UINT32_MAX) ? 0 : changeCount + 1;
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "userId=%{public}d, changeCount=%{public}u", userId, changeCount);
        return true;
    });
}

void PasteboardService::NotifyEntityObservers(std::string &entity, EntityType entityType, uint32_t dataLength)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "entityType=%{public}u, dataLength=%{public}u",
        static_cast<uint32_t>(entityType), dataLength);
    entityObserverMap_.ForEach([this, &entity, entityType, dataLength](const auto &key, auto &value) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "pid=%{public}u, listSize=%{public}zu", key, value.size());
        for (auto entityObserver : value) {
            if (entityType == entityObserver.entityType && dataLength <= entityObserver.expectedDataLength &&
                VerifyPermission(entityObserver.tokenId)) {
                entityObserver.observer->OnRecognitionEvent(entityType, entity);
            }
        }
        return false;
    });
}

int32_t PasteboardService::GetAllEntryPlainText(uint32_t dataId, uint32_t recordId,
    std::vector<std::shared_ptr<PasteDataEntry>> &entries, std::string &primaryText)
{
    for (auto &entry : entries) {
        if (primaryText.size() > MAX_RECOGNITION_LENGTH) {
            return static_cast<int32_t>(PasteboardError::EXCEEDING_LIMIT_EXCEPTION);
        }
        int32_t result = static_cast<int32_t>(PasteboardError::E_OK);
        if (entry->GetMimeType() == MIMETYPE_TEXT_PLAIN && !entry->HasContentByMimeType(MIMETYPE_TEXT_PLAIN)) {
            result = GetRecordValueByType(dataId, recordId, *entry);
        }
        if (result != static_cast<int32_t>(PasteboardError::E_OK)) {
            continue;
        }
        std::shared_ptr<std::string> plainTextPtr = entry->ConvertToPlainText();
        if (plainTextPtr != nullptr) {
            primaryText += *plainTextPtr;
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetAllEntryPlainText finished");
    return static_cast<int32_t>(PasteboardError::E_OK);
}

std::string PasteboardService::GetAllPrimaryText(const PasteData &pasteData)
{
    std::string primaryText = "";
    std::vector<std::shared_ptr<PasteDataRecord>> records = pasteData.AllRecords();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "size of records=%{public}zu", records.size());
    for (const auto &record : records) {
        if (primaryText.size() > MAX_RECOGNITION_LENGTH) {
            primaryText = "";
            break;
        }
        std::shared_ptr<std::string> plainTextPtr = record->GetPlainText();
        if (plainTextPtr != nullptr) {
            primaryText += *plainTextPtr;
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "primaryText in record");
            continue;
        }
        auto dataId = pasteData.GetDataId();
        auto recordId = record->GetRecordId();
        std::vector<std::shared_ptr<PasteDataEntry>> entries = record->GetEntries();
        int32_t result = GetAllEntryPlainText(dataId, recordId, entries, primaryText);
        if (result != static_cast<int32_t>(PasteboardError::E_OK)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "primaryText exceeded size, result=%{public}d", result);
            primaryText = "";
            break;
        }
    }
    return primaryText;
}

int32_t PasteboardService::ExtractEntity(const std::string &entity, std::string &location)
{
    nlohmann::json entityJson = nlohmann::json::parse(entity);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(!entityJson.is_discarded(),
        static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR), PASTEBOARD_MODULE_SERVICE,
        "parse entity to json failed");
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(entityJson.contains("code") && entityJson["code"].is_number(),
        static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR), PASTEBOARD_MODULE_SERVICE,
        "entity find code failed");
    int code = entityJson["code"].get<int>();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        code == 0, static_cast<int32_t>(code), PASTEBOARD_MODULE_SERVICE, "failed to get entity");
    if (entityJson.contains("entity") && entityJson["entity"].contains("location") &&
        entityJson["entity"]["location"].is_array()) {
        nlohmann::json locationJson = entityJson["entity"]["location"].get<nlohmann::json>();
        location = locationJson.dump();
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "location dump finished, location=%{private}s", location.c_str());
        return static_cast<int32_t>(PasteboardError::E_OK);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteData did not contain entity");
    return static_cast<int32_t>(PasteboardError::NO_DATA_ERROR);
}

void PasteboardService::RecognizePasteData(PasteData &pasteData)
{
    std::string primaryText = GetAllPrimaryText(pasteData);
    if (primaryText.empty()) {
        return;
    }
    FFRTTask task = [this, primaryText]() {
        pthread_setname_np(pthread_self(), "PasteDataRecognize");
        auto handle = dlopen(NLU_SO_PATH, RTLD_NOW);
        PASTEBOARD_CHECK_AND_RETURN_LOGE(handle != nullptr, PASTEBOARD_MODULE_SERVICE, "Can not get AIEngine handle");
        GetProcessorFunc GetProcessor = reinterpret_cast<GetProcessorFunc>(dlsym(handle, GET_PASTE_DATA_PROCESSOR));
        if (GetProcessor == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Can not get ProcessorFunc");
            dlclose(handle);
            return;
        }
        IPasteDataProcessor &processor = GetProcessor();
        std::string entity = "";
        int32_t result = processor.Process(primaryText, entity);
        if (result != ERR_OK) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "AI Process failed, result=%{public}d", result);
            dlclose(handle);
            return;
        }
        std::string location = "";
        int32_t ret = ExtractEntity(entity, location);
        if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "ExtractEntity failed, ret=%{public}d", ret);
            dlclose(handle);
            return;
        }
        NotifyEntityObservers(location, EntityType::ADDRESS, static_cast<uint32_t>(primaryText.size()));
        dlclose(handle);
    };
    FFRTUtils::SubmitTask(task);
}

int32_t PasteboardService::SubscribeEntityObserver(
    EntityType entityType, uint32_t expectedDataLength, const sptr<IEntityRecognitionObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE,
        "start, type=%{public}u, len=%{public}u", static_cast<uint32_t>(entityType), expectedDataLength);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(expectedDataLength <= MAX_RECOGNITION_LENGTH,
        static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR), PASTEBOARD_MODULE_SERVICE,
        "expected data length exceeds limitation");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(VerifyPermission(tokenId),
        static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR), PASTEBOARD_MODULE_SERVICE,
        "check permission failed");
    auto callingPid = IPCSkeleton::GetCallingPid();
    bool result = entityObserverMap_.ComputeIfPresent(
        callingPid, [entityType, expectedDataLength, tokenId, &observer](auto, auto &observerList) {
            auto it = std::find_if(observerList.begin(), observerList.end(),
                [entityType, expectedDataLength](const EntityObserverInfo &observer) {
                    return observer.entityType == entityType && observer.expectedDataLength == expectedDataLength;
                });
            if (it != observerList.end()) {
                it->tokenId = tokenId;
                it->observer = observer;
                return true;
            }
            observerList.emplace_back(entityType, expectedDataLength, tokenId, observer);
            return true;
        });
    if (!result) {
        std::vector<EntityObserverInfo> observerList;
        observerList.emplace_back(entityType, expectedDataLength, tokenId, observer);
        entityObserverMap_.Emplace(callingPid, observerList);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "subscribe entityObserver finished");
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::UnsubscribeEntityObserver(
    EntityType entityType, uint32_t expectedDataLength, const sptr<IEntityRecognitionObserver> &observer)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(expectedDataLength <= MAX_RECOGNITION_LENGTH,
        static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR), PASTEBOARD_MODULE_SERVICE,
        "expected data length exceeds limitation");
    auto callingPid = IPCSkeleton::GetCallingPid();
    auto result = entityObserverMap_.ComputeIfPresent(
        callingPid, [entityType, expectedDataLength](auto, auto &observerList) {
            auto it = std::find_if(observerList.begin(), observerList.end(),
                [entityType, expectedDataLength](const EntityObserverInfo &observer) {
                    return observer.entityType == entityType && observer.expectedDataLength == expectedDataLength;
                });
            if (it == observerList.end()) {
                PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE,
                    "Failed to unsubscribe, observer not found, type is %{public}u, length is %{public}u.",
                    static_cast<uint32_t>(entityType), expectedDataLength);
                return true;
            }
            observerList.erase(it);
            if (observerList.empty()) {
                return false;
            }
            return true;
        });
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardService::UnsubscribeAllEntityObserver()
{
    entityObserverMap_.Clear();
}

int32_t PasteboardService::GetRecordValueByType(uint32_t dataId, uint32_t recordId, PasteDataEntry &value)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto callPid = IPCSkeleton::GetCallingPid();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE((dataId == delayDataId_ && tokenId == delayTokenId_) ||
        VerifyPermission(tokenId), static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR),
        PASTEBOARD_MODULE_SERVICE, "check permission failed, calling pid is %{public}d", callPid);

    auto appInfo = GetAppInfo(tokenId);
    auto [hasData, data] = clips_.Find(appInfo.userId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(hasData && data, static_cast<int32_t>(PasteboardError::NO_DATA_ERROR),
        PASTEBOARD_MODULE_SERVICE, "data not find, userId=%{public}u", appInfo.userId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(dataId == data->GetDataId(),
        static_cast<int32_t>(PasteboardError::INVALID_DATA_ID), PASTEBOARD_MODULE_SERVICE,
        "dataId=%{public}u mismatch, local=%{public}u", dataId, data->GetDataId());

    auto record = data->GetRecordById(recordId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(record != nullptr, static_cast<int32_t>(PasteboardError::INVALID_RECORD_ID),
        PASTEBOARD_MODULE_SERVICE, "recordId=%{public}u invalid, max=%{public}zu", recordId, data->GetRecordCount());

    std::string utdId = value.GetUtdId();
    auto entry = record->GetEntry(utdId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(entry != nullptr, static_cast<int32_t>(PasteboardError::INVALID_MIMETYPE),
        PASTEBOARD_MODULE_SERVICE, "entry is null, recordId=%{public}u, type=%{public}s", recordId, utdId.c_str());

    if (data->IsRemote() && !entry->HasContent(utdId)) {
        int32_t ret = GetRemoteEntryValue(appInfo, *data, *record, value);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
            PASTEBOARD_MODULE_SERVICE, "get remote entry failed, type=%{public}s, ret=%{public}d", utdId.c_str(), ret);
        return static_cast<int32_t>(PasteboardError::E_OK);
    }

    int32_t ret = GetLocalEntryValue(appInfo.userId, *data, *record, value);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
        PASTEBOARD_MODULE_SERVICE, "get local entry failed, type=%{public}s, ret=%{public}d", utdId.c_str(), ret);

    std::string mimeType = value.GetMimeType();
    if (mimeType == MIMETYPE_TEXT_HTML) {
        return ProcessDelayHtmlEntry(*data, appInfo.bundleName, value);
    }
    if (mimeType == MIMETYPE_TEXT_URI) {
        std::vector<Uri> grantUris = CheckUriPermission(*data, appInfo.bundleName);
        return GrantUriPermission(grantUris, appInfo.bundleName);
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::ProcessDelayHtmlEntry(PasteData &data, const std::string &targetBundle,
    PasteDataEntry &entry)
{
    if (!PasteboardWebController::GetInstance().SplitWebviewPasteData(data)) {
        return static_cast<int32_t>(PasteboardError::E_OK);
    }

    PasteboardWebController::GetInstance().SetWebviewPasteData(data, data.GetOriginAuthority());
    PasteboardWebController::GetInstance().CheckAppUriPermission(data);

    PasteData tmp;
    std::shared_ptr<std::string> html = entry.ConvertToHtml();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(html != nullptr, static_cast<int32_t>(PasteboardError::GET_ENTRY_VALUE_FAILED),
        PASTEBOARD_MODULE_SERVICE, "convert to html failed");

    tmp.AddHtmlRecord(*html);
    tmp.SetOriginAuthority(data.GetOriginAuthority());
    tmp.SetTokenId(data.GetTokenId());
    tmp.SetRemote(data.IsRemote());
    PasteboardWebController::GetInstance().SplitWebviewPasteData(tmp);
    PasteboardWebController::GetInstance().SetWebviewPasteData(tmp, data.GetOriginAuthority());
    PasteboardWebController::GetInstance().CheckAppUriPermission(tmp);

    std::vector<Uri> grantUris = CheckUriPermission(tmp, targetBundle);
    int32_t ret = GrantUriPermission(grantUris, targetBundle);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
        PASTEBOARD_MODULE_SERVICE, "grant to %{public}s failed, ret=%{public}d", targetBundle.c_str(), ret);

    return PostProcessDelayHtmlEntry(tmp, targetBundle, entry);
}

int32_t PasteboardService::PostProcessDelayHtmlEntry(PasteData &data, const std::string &targetBundle,
    PasteDataEntry &entry)
{
    PasteboardWebController::GetInstance().RetainUri(data);
    PasteboardWebController::GetInstance().RebuildWebviewPasteData(data, targetBundle);

    std::shared_ptr<std::string> html = data.GetPrimaryHtml();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(html != nullptr, static_cast<int32_t>(PasteboardError::REBUILD_HTML_FAILED),
        PASTEBOARD_MODULE_SERVICE, "rebuild html failed");

    auto entryValue = entry.GetValue();
    if (std::holds_alternative<std::string>(entryValue)) {
        entry.SetValue(*html);
    } else if (std::holds_alternative<std::shared_ptr<Object>>(entryValue)) {
        auto object = std::get<std::shared_ptr<Object>>(entryValue);
        auto newObject = std::make_shared<Object>();
        newObject->value_ = object->value_;
        newObject->value_[UDMF::HTML_CONTENT] = *html;
        entry.SetValue(newObject);
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

bool PasteboardService::VerifyPermission(uint32_t tokenId)
{
    auto version = GetSdkVersion(tokenId);
    auto callPid = IPCSkeleton::GetCallingPid();
    if (version == INVALID_VERSION) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "get hap version failed, callPid is %{public}d, tokenId is %{public}d", callPid, tokenId);
        return false;
    }
    auto isReadGrant = IsPermissionGranted(READ_PASTEBOARD_PERMISSION, tokenId);
    if (isReadGrant) {
        return true;
    }
    auto isSecureGrant = IsPermissionGranted(SECURE_PASTE_PERMISSION, tokenId);
    if (isSecureGrant) {
        return true;
    }
    AddPermissionRecord(tokenId, isReadGrant, isSecureGrant);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE,
        "isReadGrant is %{public}d, isSecureGrant is %{public}d,", isReadGrant, isSecureGrant);
    bool isCtrlVAction = false;
    if (inputEventCallback_ != nullptr) {
        isCtrlVAction = inputEventCallback_->IsCtrlVProcess(callPid, IsFocusedApp(tokenId));
        inputEventCallback_->Clear();
    }
    auto isGrant = isReadGrant || isSecureGrant || isCtrlVAction;
    if (!isGrant && version >= ADD_PERMISSION_CHECK_SDK_VERSION) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "no permission, callPid is %{public}d, version is %{public}d",
            callPid, version);
        return false;
    }
    return true;
}

int32_t PasteboardService::IsDataVaild(PasteData &pasteData, uint32_t tokenId)
{
    if (pasteData.IsDraggedData() || !pasteData.IsValid()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "data is invalid");
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    if (IsDataAged()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "data is aged");
        return static_cast<int32_t>(PasteboardError::DATA_EXPIRED_ERROR);
    }
    auto screenStatus = GetCurrentScreenStatus();
    if (pasteData.GetScreenStatus() > screenStatus) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "current screen is %{public}d, set data screen is %{public}d.",
            screenStatus, pasteData.GetScreenStatus());
        return static_cast<int32_t>(PasteboardError::CROSS_BORDER_ERROR);
    }
    switch (pasteData.GetShareOption()) {
        case ShareOption::InApp: {
            if (pasteData.GetTokenId() != tokenId) {
                PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "InApp check failed.");
                return static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR);
            }
            break;
        }
        case ShareOption::LocalDevice: {
            break;
        }
        case ShareOption::CrossDevice: {
            break;
        }
        default: {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "tokenId = 0x%{public}x, shareOption = %{public}d is error.",
                tokenId, pasteData.GetShareOption());
            return static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR);
        }
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::GetSdkVersion(uint32_t tokenId)
{
    if (AccessTokenKit::GetTokenTypeFlag(tokenId) != ATokenTypeEnum::TOKEN_HAP) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "caller is not application");
        return 0;
    }
    HapTokenInfo hapTokenInfo;
    auto ret = AccessTokenKit::GetHapTokenInfo(tokenId, hapTokenInfo);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetHapTokenInfo fail, tokenid is %{public}u, ret is %{public}d.",
            tokenId, ret);
        return INVALID_VERSION;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ver:%{public}d.", hapTokenInfo.apiVersion);
    return hapTokenInfo.apiVersion;
}

bool PasteboardService::IsPermissionGranted(const std::string &perm, uint32_t tokenId)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "check grant permission, perm=%{public}s", perm.c_str());
    int32_t result = AccessTokenKit::VerifyAccessToken(tokenId, perm);
    if (result == PermissionState::PERMISSION_DENIED) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "permission denied");
        return false;
    }
    return true;
}

bool PasteboardService::IsDataAged()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "IsDataAged start");
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return true;
    }
    auto it = copyTime_.Find(userId);
    if (!it.first) {
        return true;
    }
    uint64_t copyTime = it.second;
    auto curTime = static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "copyTime = %{public}" PRIu64 ", curTime = %{public}" PRIu64,
        copyTime, curTime);
    if (curTime > copyTime && curTime - copyTime > ONE_HOUR_MILLISECONDS) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "data is out of the time");
        auto data = clips_.Find(userId);
        if (data.first) {
            RevokeUriPermission(data.second);
            clips_.Erase(userId);
            delayDataId_ = 0;
            delayTokenId_ = 0;
        }
        copyTime_.Erase(userId);
        RADAR_REPORT(DFX_CLEAR_PASTEBOARD, DFX_AUTO_CLEAR, DFX_SUCCESS);
        return true;
    }
    return false;
}

AppInfo PasteboardService::GetAppInfo(uint32_t tokenId)
{
    AppInfo info;
    info.tokenId = tokenId;
    info.tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    info.userId = GetCurrentAccountId();
    switch (info.tokenType) {
        case ATokenTypeEnum::TOKEN_HAP: {
            HapTokenInfo hapInfo;
            if (AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != 0) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get hap token info fail.");
                return info;
            }
            info.bundleName = hapInfo.bundleName;
            break;
        }
        case ATokenTypeEnum::TOKEN_NATIVE:
        case ATokenTypeEnum::TOKEN_SHELL: {
            NativeTokenInfo tokenInfo;
            if (AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != 0) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get native token info fail.");
                return info;
            }
            info.bundleName = tokenInfo.processName;
            break;
        }
        default: {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "tokenType = %{public}d not match.", info.tokenType);
        }
    }
    return info;
}

std::string PasteboardService::GetAppBundleName(const AppInfo &appInfo)
{
    std::string bundleName;
    if (appInfo.userId != ERROR_USERID) {
        bundleName = appInfo.bundleName;
    } else {
        bundleName = "error";
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetAppInfo error");
    }
    return bundleName;
}

void PasteboardService::SetLocalPasteFlag(bool isCrossPaste, uint32_t tokenId, PasteData &pasteData)
{
    pasteData.SetLocalPasteFlag(!isCrossPaste && tokenId == pasteData.GetTokenId());
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "isLocalPaste = %{public}d.", pasteData.IsLocalPaste());
}

void PasteboardService::ShowProgress(const std::string &progressKey, const sptr<IRemoteObject> &observer)
{
    if (!HasPasteData()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "not pastedata, no need to show progress.");
        return;
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!IsFocusedApp(tokenId)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "not focused app, no need to show progress.");
        return;
    }
    PasteBoardDialog::ProgressMessageInfo message;
    std::string deviceName = "";
    bool isRemote = false;
    auto result = (GetRemoteDeviceName(deviceName, isRemote) == static_cast<int32_t>(PasteboardError::E_OK));
    if (result && isRemote) {
        message.promptText = "PromptText_PasteBoard_Remote";
        message.remoteDeviceName = deviceName;
    } else {
        message.promptText = "PromptText_PasteBoard_Local";
        message.remoteDeviceName = "";
    }
    message.isRemote = isRemote;
    message.progressKey = progressKey;

    FocusedAppInfo appInfo = GetFocusedAppInfo();
    message.windowId = appInfo.windowId;
    message.callerToken = appInfo.abilityToken;
    message.clientCallback = observer;
    PasteBoardDialog::GetInstance().ShowProgress(message);
}

int32_t PasteboardService::GetPasteData(PasteData &data, int32_t &syncTime)
{
    PasteboardTrace tracer("PasteboardService GetPasteData");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto callPid = IPCSkeleton::GetCallingPid();
    auto appInfo = GetAppInfo(tokenId);
    bool developerMode = OHOS::system::GetBoolParameter("const.security.developermode.state", false);
    bool isTestServerSetPasteData = developerMode && setPasteDataUId_ == TEST_SERVER_UID;
    if (!VerifyPermission(tokenId) && !isTestServerSetPasteData) {
        RADAR_REPORT(DFX_GET_PASTEBOARD, DFX_CHECK_GET_AUTHORITY, DFX_SUCCESS, GET_DATA_APP, appInfo.bundleName,
            RadarReporter::CONCURRENT_ID, data.GetPasteId());
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "check permission failed, callingPid is %{public}d", callPid);
        return static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR);
    }
    auto ret = GetData(tokenId, data, syncTime);
    UE_REPORT(UeReporter::UE_PASTE, GenerateDataType(data), appInfo.bundleName,
        (ret == static_cast<int32_t>(PasteboardError::E_OK)) ? UeReporter::E_OK_OPERATION : ret,
        DMAdapter::GetInstance().GetLocalDeviceType(), UeReporter::CROSS_FLAG, data.IsRemote());
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "data is invalid, ret is %{public}d, callPid is %{public}d, tokenId is %{public}d", ret, callPid, tokenId);
    } else {
        delayDataId_ = data.GetDataId();
        delayTokenId_ = tokenId;
    }
    return ret;
}

void PasteboardService::AddPermissionRecord(uint32_t tokenId, bool isReadGrant, bool isSecureGrant)
{
    if (AccessTokenKit::GetTokenTypeFlag(tokenId) != TOKEN_HAP) {
        return;
    }
    bool isGrant = isReadGrant || isSecureGrant;
    if (!isGrant) {
        return;
    }
    auto permUsedType = static_cast<PermissionUsedType>(AccessTokenKit::GetPermissionUsedType(
        tokenId, isSecureGrant ? SECURE_PASTE_PERMISSION : READ_PASTEBOARD_PERMISSION));
    AddPermParamInfo info;
    info.tokenId = tokenId;
    info.permissionName = READ_PASTEBOARD_PERMISSION;
    info.successCount = 1;
    info.failCount = 0;
    info.type = permUsedType;
    int32_t result = PrivacyKit::AddPermissionUsedRecord(info);
    if (result != RET_SUCCESS) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "add record failed, result is %{public}d", result);
    }
    return;
}

int32_t PasteboardService::GetData(uint32_t tokenId, PasteData &data, int32_t &syncTime)
{
    CalculateTimeConsuming::SetBeginTime();
    auto appInfo = GetAppInfo(tokenId);
    int32_t result = static_cast<int32_t>(PasteboardError::E_OK);
    std::string peerNetId = "";
    std::string peerUdid = "";
    std::string pasteId = data.GetPasteId();
    auto [distRet, distEvt] = GetValidDistributeEvent(appInfo.userId);
    if (distRet != static_cast<int32_t>(PasteboardError::E_OK) ||
        GetCurrentScreenStatus() != ScreenEvent::ScreenUnlocked) {
        result = GetLocalData(appInfo, data);
    } else {
        result = GetRemoteData(appInfo.userId, distEvt, data, syncTime);
        peerNetId = distEvt.deviceId;
        peerUdid = DMAdapter::GetInstance().GetUdidByNetworkId(peerNetId);
    }
    if (observerEventMap_.size() != 0) {
        std::string targetBundleName = GetAppBundleName(appInfo);
        NotifyObservers(targetBundleName, PasteboardEventStatus::PASTEBOARD_READ);
    }

    RADAR_REPORT(DFX_GET_PASTEBOARD, DFX_GET_DATA_INFO, DFX_SUCCESS, CONCURRENT_ID, pasteId, GET_DATA_APP,
        appInfo.bundleName, LOCAL_DEV_TYPE, DMAdapter::GetInstance().GetLocalDeviceType(),
        NETWORK_DEV_NUM, DMAdapter::GetInstance().GetNetworkIds().size(), PEER_NET_ID,
        PasteboardDfxUntil::GetAnonymousID(peerNetId), PEER_UDID, PasteboardDfxUntil::GetAnonymousID(peerUdid));

    if (result != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get local or remote data err:%{public}d", result);
        return result;
    }
    int64_t fileSize = data.GetFileSize();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "fileSize=%{public}" PRId64 ", isRemote=%{public}d", fileSize,
        static_cast<int>(data.IsRemote()));
    GetPasteDataDot(data, appInfo.bundleName);
    std::vector<Uri> grantUris = CheckUriPermission(data, appInfo.bundleName);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGD(!grantUris.empty(), static_cast<int32_t>(PasteboardError::E_OK),
        PASTEBOARD_MODULE_SERVICE, "no uri");
    if (data.IsRemote()) {
        data.SetPasteId(pasteId);
        data.deviceId_ = distEvt.deviceId;
        EstablishP2PLink(data.deviceId_, data.GetPasteId());
    }
    return GrantUriPermission(grantUris, appInfo.bundleName);
}

PasteboardService::RemoteDataTaskManager::DataTask PasteboardService::RemoteDataTaskManager::GetRemoteDataTask(
    const Event &event)
{
    auto key = event.deviceId + std::to_string(event.seqId);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = dataTasks_.find(key);
    if (it == dataTasks_.end()) {
        it = dataTasks_.emplace(key, std::make_shared<TaskContext>()).first;
    }

    if (it == dataTasks_.end()) {
        return std::make_pair(nullptr, false);
    }

    return std::make_pair(it->second, it->second->pasting_.exchange(true));
}

void PasteboardService::RemoteDataTaskManager::Notify(const Event &event, std::shared_ptr<PasteDateTime> data)
{
    auto key = event.deviceId + std::to_string(event.seqId);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = dataTasks_.find(key);
    if (it == dataTasks_.end()) {
        return;
    }
    auto &task = it->second;
    task->data_ = data;
    task->getDataBlocks_.ForEach([](const auto &key, auto value) -> bool {
        value->SetValue(true);
        return false;
    });
}

std::shared_ptr<PasteDateTime> PasteboardService::RemoteDataTaskManager::WaitRemoteData(const Event &event)
{
    std::shared_ptr<PasteboardService::RemoteDataTaskManager::TaskContext> task;
    {
        auto key = event.deviceId + std::to_string(event.seqId);
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = dataTasks_.find(key);
        if (it == dataTasks_.end()) {
            return nullptr;
        }

        task = it->second;
    }

    auto key = ++mapKey_;
    auto block = std::make_shared<BlockObject<bool>>(GET_REMOTE_DATA_WAIT_TIME);
    task->getDataBlocks_.InsertOrAssign(key, block);
    block->GetValue();

    task->getDataBlocks_.Erase(key);
    return task->data_;
}

void PasteboardService::RemoteDataTaskManager::ClearRemoteDataTask(const Event &event)
{
    auto key = event.deviceId + std::to_string(event.seqId);
    std::lock_guard<std::mutex> lock(mutex_);
    dataTasks_.erase(key);
}

int32_t PasteboardService::GetRemoteData(int32_t userId, const Event &event, PasteData &data, int32_t &syncTime)
{
    syncTime = -1;
    auto [task, isPasting] = taskMgr_.GetRemoteDataTask(event);
    if (task == nullptr) {
        return static_cast<int32_t>(PasteboardError::REMOTE_TASK_ERROR);
    }

    if (isPasting) {
        auto value = taskMgr_.WaitRemoteData(event);
        if (value != nullptr && value->data != nullptr) {
            syncTime = value->syncTime;
            data = *(value->data);
            return static_cast<int32_t>(PasteboardError::E_OK);
        }
        return static_cast<int32_t>(PasteboardError::TASK_PROCESSING);
    }

    auto [distRet, distEvt] = GetValidDistributeEvent(userId);
    if (distRet != static_cast<int32_t>(PasteboardError::E_OK) || !(distEvt == event)) {
        int32_t ret = distRet == static_cast<int32_t>(PasteboardError::E_OK) ?
            static_cast<int32_t>(PasteboardError::INVALID_EVENT_ERROR) : distRet;
        auto it = clips_.Find(userId);
        if (it.first) {
            data = *it.second;
            ret = static_cast<int32_t>(PasteboardError::E_OK);
        }
        taskMgr_.ClearRemoteDataTask(event);
        return ret;
    }

    return GetRemotePasteData(userId, event, data, syncTime);
}

int32_t PasteboardService::GetRemotePasteData(int32_t userId, const Event &event, PasteData &data, int32_t &syncTime)
{
    auto block = std::make_shared<BlockObject<std::shared_ptr<PasteDateTime>>>(GET_REMOTE_DATA_WAIT_TIME);
    std::thread thread([this, event, block, userId]() mutable {
        auto result = GetDistributedData(event, userId);
        auto [distRet, distEvt] = GetValidDistributeEvent(userId);
        std::shared_ptr<PasteDateTime> pasteDataTime = std::make_shared<PasteDateTime>();
        if (result.first != nullptr) {
            result.first->SetRemote(true);
            if (distEvt == event) {
                clips_.InsertOrAssign(userId, result.first);
                IncreaseChangeCount(userId);
                auto curTime =
                    static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
                copyTime_.InsertOrAssign(userId, curTime);
            }
            pasteDataTime->syncTime = result.second.syncTime;
            pasteDataTime->data = result.first;
            pasteDataTime->errorCode = result.second.errorCode;
            taskMgr_.Notify(event, pasteDataTime);
        } else {
            pasteDataTime->data = nullptr;
            pasteDataTime->errorCode = result.second.errorCode;
            taskMgr_.Notify(event, pasteDataTime);
        }
        block->SetValue(pasteDataTime);
        taskMgr_.ClearRemoteDataTask(event);
    });
    thread.detach();
    auto value = block->GetValue();
    if (value != nullptr && value->data != nullptr) {
        syncTime = value->syncTime;
        data = std::move(*(value->data));
        return value->errorCode;
    } else if (value != nullptr && value->data == nullptr) {
        return value->errorCode;
    }
    return static_cast<int32_t>(PasteboardError::TIMEOUT_ERROR);
}

int32_t PasteboardService::GetLocalData(const AppInfo &appInfo, PasteData &data)
{
    std::string pasteId = data.GetPasteId();
    auto it = clips_.Find(appInfo.userId);
    auto tempTime = copyTime_.Find(appInfo.userId);
    if (!it.first || !tempTime.first) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "no data userId is %{public}d.", appInfo.userId);
        return static_cast<int32_t>(PasteboardError::NO_DATA_ERROR);
    }
    auto ret = IsDataVaild(*(it.second), appInfo.tokenId);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "paste data is invaild. ret is %{public}d", ret);
        return ret;
    }
    data = *(it.second);
    auto originBundleName = it.second->GetBundleName();
    if (it.second->IsDelayData()) {
        GetDelayPasteData(appInfo.userId, data);
        RADAR_REPORT(DFX_GET_PASTEBOARD, DFX_CHECK_GET_DELAY_PASTE, DFX_SUCCESS, CONCURRENT_ID, pasteId);
    }
    if (it.second->IsDelayRecord()) {
        GetDelayPasteRecord(appInfo.userId, data);
    }
    data.SetBundleName(appInfo.bundleName);
    auto result = copyTime_.Find(appInfo.userId);
    if (!result.first) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId not found userId is %{public}d", appInfo.userId);
        return static_cast<int32_t>(PasteboardError::INVALID_USERID_ERROR);
    }
    auto curTime = result.second;
    if (tempTime.second == curTime) {
        bool isNotify = false;
        clips_.ComputeIfPresent(appInfo.userId, [&data, &isNotify](auto &key, auto &value) {
            if (value->IsDelayData()) {
                value = std::make_shared<PasteData>(data);
                isNotify = true;
            }
            if (value->IsDelayRecord()) {
                value = std::make_shared<PasteData>(data);
            }
            return true;
        });
        if (isNotify) {
            NotifyObservers(originBundleName, PasteboardEventStatus::PASTEBOARD_WRITE);
        }
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetPasteData success.");
    SetLocalPasteFlag(data.IsRemote(), appInfo.tokenId, data);
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardService::GetDelayPasteData(int32_t userId, PasteData &data)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "get delay data start");
    delayGetters_.ComputeIfPresent(userId, [this, &data](auto, auto &delayGetter) {
        PasteData delayData;
        if (delayGetter.first != nullptr) {
            delayGetter.first->GetPasteData("", delayData);
        }
        if (delayGetter.second != nullptr) {
            delayGetter.first->AsObject()->RemoveDeathRecipient(delayGetter.second);
        }
        delayData.SetDelayData(false);
        delayData.SetBundleName(data.GetBundleName());
        delayData.SetOriginAuthority(data.GetOriginAuthority());
        delayData.SetTime(data.GetTime());
        delayData.SetTokenId(data.GetTokenId());
        PasteboardWebController::GetInstance().SplitWebviewPasteData(delayData);
        PasteboardWebController::GetInstance().SetWebviewPasteData(delayData, data.GetOriginAuthority());
        PasteboardWebController::GetInstance().CheckAppUriPermission(delayData);
        data = delayData;
        return false;
    });
}

int32_t PasteboardService::GetDelayPasteRecord(int32_t userId, PasteData &data)
{
    auto [hasGetter, getter] = entryGetters_.Find(userId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(hasGetter && getter.first,
        static_cast<int32_t>(PasteboardError::NO_DELAY_GETTER), PASTEBOARD_MODULE_SERVICE,
        "entry getter not find, userId=%{public}d, dataId=%{public}u", userId, data.GetDataId());

    for (auto record : data.AllRecords()) {
        if (!(record->HasEmptyEntry())) {
            PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "record do not has empty value.");
            continue;
        }
        if (!record->IsDelayRecord()) {
            PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "record is not DelayRecord.");
            continue;
        }
        auto entries = record->GetEntries();
        if (entries.empty()) {
            PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "record size is 0.");
            continue;
        }
        if (!std::holds_alternative<std::monostate>(entries[0]->GetValue())) {
            continue;
        }
        auto result = getter.first->GetRecordValueByType(record->GetRecordId(), *entries[0]);
        if (result != static_cast<int32_t>(PasteboardError::E_OK)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
                "get record value fail, dataId is %{public}d, recordId is %{public}d", data.GetDataId(),
                record->GetRecordId());
            continue;
        }
        record->AddEntry(entries[0]->GetUtdId(), entries[0]);
    }
    PasteboardWebController::GetInstance().SplitWebviewPasteData(data);
    PasteboardWebController::GetInstance().SetWebviewPasteData(data, data.GetOriginAuthority());
    PasteboardWebController::GetInstance().CheckAppUriPermission(data);
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardService::EstablishP2PLink(const std::string &networkId, const std::string &pasteId)
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    auto callPid = IPCSkeleton::GetCallingPid();
    p2pMap_.Compute(networkId, [pasteId, callPid](const auto &key, auto &value) {
        value.Compute(pasteId, [callPid](const auto &key, auto &value) {
            value = callPid;
            return true;
        });
        return true;
    });
    if (ffrtTimer_ != nullptr) {
        FFRTTask task = [this, networkId, pasteId] {
            PasteComplete(networkId, pasteId);
        };
        ffrtTimer_->SetTimer(pasteId, task, MIN_TRANMISSION_TIME);
    }
    DmDeviceInfo remoteDevice;
    auto ret = DMAdapter::GetInstance().GetRemoteDeviceInfo(networkId, remoteDevice);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "remote device is not exist");
        p2pMap_.Erase(networkId);
        return;
    }
    auto plugin = GetClipPlugin();
    if (plugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "plugin is not exist");
        p2pMap_.Erase(networkId);
        return;
    }
    auto status = DistributedFileDaemonManager::GetInstance().OpenP2PConnection(remoteDevice);
    if (status != RESULT_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "open p2p error, status:%{public}d", status);
        p2pMap_.Erase(networkId);
        return;
    }
    status = plugin->PublishServiceState(networkId, ClipPlugin::ServiceStatus::CONNECT_SUCC);
    if (status != RESULT_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Publish state connect_succ error, status:%{public}d", status);
    }
#endif
}

void PasteboardService::CloseP2PLink(const std::string &networkId)
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    DmDeviceInfo remoteDevice;
    auto ret = DMAdapter::GetInstance().GetRemoteDeviceInfo(networkId, remoteDevice);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "remote device is not exist");
        return;
    }
    auto status = DistributedFileDaemonManager::GetInstance().CloseP2PConnection(remoteDevice);
    if (status != RESULT_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "close p2p error, status:%{public}d", status);
    }
    auto plugin = GetClipPlugin();
    if (plugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "plugin is not exist");
        return;
    }
    status = plugin->PublishServiceState(networkId, ClipPlugin::ServiceStatus::IDLE);
    if (status != RESULT_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Publish state idle error, status:%{public}d", status);
    }
#endif
}

void PasteboardService::PasteStart(const std::string &pasteId)
{
    if (ffrtTimer_ != nullptr) {
        ffrtTimer_->CancelTimer(pasteId);
    }
}

void PasteboardService::PasteComplete(const std::string &deviceId, const std::string &pasteId)
{
    if (deviceId.empty()) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "deviceId is empty");
        return;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "deviceId is %{public}.6s, taskId is %{public}s", deviceId.c_str(),
        pasteId.c_str());
    RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, RadarReporter::DFX_DISTRIBUTED_FILE_END, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, pasteId);
    p2pMap_.ComputeIfPresent(deviceId, [pasteId, deviceId, this](const auto &key, auto &value) {
        value.ComputeIfPresent(pasteId, [deviceId](const auto &key, auto &value) {
            return false;
        });
        if (value.Empty()) {
            CloseP2PLink(deviceId);
            return false;
        }
        return true;
    });
}

int32_t PasteboardService::GetRemoteDeviceName(std::string &deviceName, bool &isRemote)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "enter");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto appInfo = GetAppInfo(tokenId);
    auto event = GetValidDistributeEvent(appInfo.userId);
    DmDeviceInfo remoteDevice;
    if (!event.second.deviceId.empty()) {
        auto ret = DMAdapter::GetInstance().GetRemoteDeviceInfo(event.second.deviceId, remoteDevice);
        if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "remote device is not exist");
            return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
        }
        deviceName = remoteDevice.deviceName;
        isRemote = true;
    } else {
        deviceName = "local";
        isRemote = false;
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::GrantUriPermission(const std::vector<Uri> &grantUris, const std::string &targetBundleName)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGD(!grantUris.empty(), static_cast<int32_t>(PasteboardError::E_OK),
        PASTEBOARD_MODULE_SERVICE, "no uri");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "uri size=%{public}zu, target=%{public}s",
        grantUris.size(), targetBundleName.c_str());
    bool hasGranted = false;
    int32_t ret = 0;
    size_t offset = 0;
    size_t length = grantUris.size();
    size_t count = PasteData::URI_BATCH_SIZE;
    while (length > offset) {
        if (length - offset < PasteData::URI_BATCH_SIZE) {
            count = length - offset;
        }
        auto sendValues = std::vector<Uri>(grantUris.begin() + offset, grantUris.begin() + offset + count);
        int32_t permissionCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermissionPrivileged(
            sendValues, AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION, targetBundleName);
        hasGranted = hasGranted || (permissionCode == 0);
        ret = permissionCode == 0 ? ret : permissionCode;
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "permissionCode is %{public}d", permissionCode);
        offset += count;
    }
    if (hasGranted) {
        std::lock_guard<std::mutex> lock(readBundleMutex_);
        if (readBundles_.count(targetBundleName) == 0) {
            readBundles_.insert(targetBundleName);
        }
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "leave, ret=%{public}d", ret);
    return ret == 0 ? static_cast<int32_t>(PasteboardError::E_OK) : ret;
}

std::vector<Uri> PasteboardService::CheckUriPermission(PasteData &data, const std::string &targetBundleName)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "enter");
    std::vector<Uri> grantUris;
    for (size_t i = 0; i < data.GetRecordCount(); i++) {
        auto item = data.GetRecordAt(i);
        if (item == nullptr || (!data.IsRemote() && targetBundleName.compare(data.GetOriginAuthority()) == 0)) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "local dev & local app");
            continue;
        }
        std::shared_ptr<OHOS::Uri> uri = nullptr;
        if (!item->isConvertUriFromRemote && !item->GetConvertUri().empty()) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "clear local disUri");
            item->SetConvertUri("");
        }
        if (item->isConvertUriFromRemote && !item->GetConvertUri().empty()) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "get remote disUri");
            uri = std::make_shared<OHOS::Uri>(item->GetConvertUri());
        } else if (!item->isConvertUriFromRemote && item->GetOriginUri() != nullptr) {
            uri = item->GetOriginUri();
        }
        if (uri == nullptr) {
            continue;
        }
        auto hasGrantUriPermission = item->HasGrantUriPermission();
        const std::string &bundleName = data.GetOriginAuthority();
        if (!IsBundleOwnUriPermission(bundleName, *uri) && !hasGrantUriPermission) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "uri:%{private}s, bundleName:%{public}s, has grant:%{public}d",
                uri->ToString().c_str(), bundleName.c_str(), hasGrantUriPermission);
            continue;
        }
        grantUris.emplace_back(*uri);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "leave, grant:%{public}zu", grantUris.size());
    return grantUris;
}

void PasteboardService::RevokeUriPermission(std::shared_ptr<PasteData> pasteData)
{
    std::set<std::string> bundles;
    {
        std::lock_guard<std::mutex> lock(readBundleMutex_);
        bundles = std::move(readBundles_);
    }
    if (pasteData == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "pasteData is null");
        return;
    }
    if (bundles.empty()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "bundles empty");
        return;
    }
    std::thread thread([pasteData, bundles]() {
        auto &permissionClient = AAFwk::UriPermissionManagerClient::GetInstance();
        for (size_t i = 0; i < pasteData->GetRecordCount(); i++) {
            auto item = pasteData->GetRecordAt(i);
            if (item == nullptr || item->GetOriginUri() == nullptr) {
                continue;
            }
            Uri &uri = *(item->GetOriginUri());
            for (std::set<std::string>::iterator it = bundles.begin(); it != bundles.end(); it++) {
                auto permissionCode = permissionClient.RevokeUriPermissionManually(uri, *it);
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "permissionCode is %{public}d", permissionCode);
            }
        }
    });
    thread.detach();
}

bool PasteboardService::IsBundleOwnUriPermission(const std::string &bundleName, Uri &uri)
{
    return (bundleName.compare(uri.GetAuthority()) == 0);
}

void PasteboardService::ShowHintToast(uint32_t tokenId, uint32_t pid)
{
    PasteBoardDialog::ToastMessageInfo message;
    message.appName = GetAppLabel(tokenId);
    PasteBoardDialog::GetInstance().ShowToast(message);
}

bool PasteboardService::HasPasteData()
{
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return false;
    }

    if (GetCurrentScreenStatus() == ScreenEvent::ScreenUnlocked) {
        auto [distRet, distEvt] = GetValidDistributeEvent(userId);
        if (distRet == static_cast<int32_t>(PasteboardError::E_OK)) {
            return true;
        }
    }

    auto it = clips_.Find(userId);
    if (it.first && (it.second != nullptr)) {
        auto tokenId = IPCSkeleton::GetCallingTokenID();
        auto ret = IsDataVaild(*(it.second), tokenId);
        if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
                "pasteData is invalid, tokenId: %{public}d, userId: %{public}d,"
                "ret is %{public}d", tokenId, userId, ret);
            return false;
        }
        return true;
    }
    return false;
}

int32_t PasteboardService::SaveData(PasteData &pasteData,
    const sptr<IPasteboardDelayGetter> delayGetter, const sptr<IPasteboardEntryGetter> entryGetter)
{
    PasteboardTrace tracer("PasteboardService, SetPasteData");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!IsCopyable(tokenId)) {
        RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_CHECK_SET_AUTHORITY, DFX_SUCCESS);
        return static_cast<int32_t>(PasteboardError::PROHIBIT_COPY);
    }
    if (setting_.exchange(true)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "is setting.");
        return static_cast<int32_t>(PasteboardError::TASK_PROCESSING);
    }
    CalculateTimeConsuming::SetBeginTime();
    auto appInfo = GetAppInfo(tokenId);
    if (appInfo.userId == ERROR_USERID) {
        setting_.store(false);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return static_cast<int32_t>(PasteboardError::INVALID_USERID_ERROR);
    }
    setPasteDataUId_ = IPCSkeleton::GetCallingUid();
    RemovePasteData(appInfo);
    pasteData.SetBundleName(appInfo.bundleName);
    pasteData.SetOriginAuthority(appInfo.bundleName);
    std::string time = GetTime();
    pasteData.SetTime(time);
    pasteData.SetScreenStatus(GetCurrentScreenStatus());
    pasteData.SetTokenId(tokenId);
    auto dataId = ++dataId_;
    pasteData.SetDataId(dataId);
    for (auto &record : pasteData.AllRecords()) {
        record->SetDataId(dataId);
    }
    UpdateShareOption(pasteData);
    PasteboardWebController::GetInstance().SetWebviewPasteData(pasteData, appInfo.bundleName);
    PasteboardWebController::GetInstance().CheckAppUriPermission(pasteData);
    clips_.InsertOrAssign(appInfo.userId, std::make_shared<PasteData>(pasteData));
    IncreaseChangeCount(appInfo.userId);
    RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_CHECK_SET_DELAY_COPY, static_cast<int>(pasteData.IsDelayData()),
        SET_DATA_APP, appInfo.bundleName, LOCAL_DEV_TYPE, DMAdapter::GetInstance().GetLocalDeviceType(),
        NETWORK_DEV_NUM, DMAdapter::GetInstance().GetNetworkIds().size()
    );
    HandleDelayDataAndRecord(pasteData, delayGetter, entryGetter, appInfo);
    auto curTime = static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    copyTime_.InsertOrAssign(appInfo.userId, curTime);
    if (!(pasteData.IsDelayData())) {
        SetDistributedData(appInfo.userId, pasteData);
        NotifyObservers(appInfo.bundleName, PasteboardEventStatus::PASTEBOARD_WRITE);
    }
    SetPasteDataDot(pasteData);
    setting_.store(false);
    SubscribeKeyboardEvent();
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardService::HandleDelayDataAndRecord(PasteData &pasteData, const sptr<IPasteboardDelayGetter> delayGetter,
    const sptr<IPasteboardEntryGetter> entryGetter, const AppInfo &appInfo)
{
    if (pasteData.IsDelayData() && delayGetter != nullptr) {
        sptr<DelayGetterDeathRecipient> deathRecipient = new (std::nothrow)
            DelayGetterDeathRecipient(appInfo.userId, *this);
        delayGetter->AsObject()->AddDeathRecipient(deathRecipient);
        delayGetters_.InsertOrAssign(appInfo.userId, std::make_pair(delayGetter, deathRecipient));
    }
    if (pasteData.IsDelayRecord() && entryGetter != nullptr) {
        sptr<EntryGetterDeathRecipient> deathRecipient = new (std::nothrow)
            EntryGetterDeathRecipient(appInfo.userId, *this);
        entryGetter->AsObject()->AddDeathRecipient(deathRecipient);
        entryGetters_.InsertOrAssign(appInfo.userId, std::make_pair(entryGetter, deathRecipient));
    }
}

bool PasteboardService::IsBasicType(const std::string &mimeType)
{
    if (mimeType == MIMETYPE_TEXT_HTML || mimeType == MIMETYPE_TEXT_PLAIN || mimeType == MIMETYPE_TEXT_URI) {
        return true;
    }
    return false;
}

std::vector<std::string> PasteboardService::GetMimeTypes()
{
    if (GetCurrentScreenStatus() == ScreenEvent::ScreenUnlocked) {
        auto userId = GetCurrentAccountId();
        auto [distRet, distEvt] = GetValidDistributeEvent(userId);
        if (distRet == static_cast<int32_t>(PasteboardError::E_OK)) {
            PasteData data;
            int32_t syncTime = 0;
            int32_t ret = GetRemoteData(userId, distEvt, data, syncTime);
            PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), {},
                PASTEBOARD_MODULE_SERVICE, "get remote data failed, ret=%{public}d", ret);
        }
    }
    return GetLocalMimeTypes();
}

bool PasteboardService::HasDataType(const std::string &mimeType)
{
    if (GetCurrentScreenStatus() == ScreenEvent::ScreenUnlocked) {
        auto userId = GetCurrentAccountId();
        auto [distRet, distEvt] = GetValidDistributeEvent(userId);
        if (distRet == static_cast<int32_t>(PasteboardError::E_OK)) {
            auto it = std::find(distEvt.dataType.begin(), distEvt.dataType.end(), mimeType);
            if (it != distEvt.dataType.end()) {
                return true;
            }
            if (IsBasicType(mimeType)) {
                return false;
            }
            PasteData data;
            int32_t syncTime = 0;
            if (GetRemoteData(userId, distEvt, data, syncTime) != static_cast<int32_t>(PasteboardError::E_OK)) {
                return false;
            }
        }
    }
    return HasLocalDataType(mimeType);
}

std::set<Pattern> PasteboardService::DetectPatterns(const std::set<Pattern> &patternsToCheck)
{
    bool hasPlain = HasLocalDataType(MIMETYPE_TEXT_PLAIN);
    bool hasHTML = HasLocalDataType(MIMETYPE_TEXT_HTML);
    if (!hasHTML && !hasPlain) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "no text");
        return {};
    }
    int32_t userId = GetCurrentAccountId();
    auto it = clips_.Find(userId);
    if (!it.first) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "error, no PasteData!");
        return {};
    }
    std::shared_ptr<PasteData> pasteData = it.second;
    return OHOS::MiscServices::PatternDetection::Detect(patternsToCheck, *pasteData, hasHTML, hasPlain);
}

std::pair<int32_t, ClipPlugin::GlobalEvent> PasteboardService::GetValidDistributeEvent(int32_t user)
{
    Event evt;
    auto plugin = GetClipPlugin();
    if (plugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "plugin is null");
        return std::make_pair(static_cast<int32_t>(PasteboardError::PLUGIN_IS_NULL), evt);
    }
    auto events = plugin->GetTopEvents(1, user);
    if (events.empty()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "plugin event is empty");
        return std::make_pair(static_cast<int32_t>(PasteboardError::PLUGIN_EVENT_EMPTY), evt);
    }

    evt = events[0];
    if (evt.deviceId == DMAdapter::GetInstance().GetLocalNetworkId() || evt.expiration < currentEvent_.expiration) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get local data");
        return std::make_pair(static_cast<int32_t>(PasteboardError::GET_LOCAL_DATA), evt);
    }
    if (evt.account != AccountManager::GetInstance().GetCurrentAccount()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "account error");
        return std::make_pair(static_cast<int32_t>(PasteboardError::INVALID_EVENT_ACCOUNT), evt);
    }
    DmDeviceInfo remoteDevice;
    int32_t ret = DMAdapter::GetInstance().GetRemoteDeviceInfo(evt.deviceId, remoteDevice);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "deviceId: %{public}.6s is offline", evt.deviceId.c_str());
        return std::make_pair(ret, evt);
    }

    if (evt.deviceId == currentEvent_.deviceId && evt.seqId == currentEvent_.seqId &&
        evt.expiration == currentEvent_.expiration) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get same remote data");
        return std::make_pair(static_cast<int32_t>(PasteboardError::GET_SAME_REMOTE_DATA), evt);
    }

    uint64_t curTime =
        static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    ret = evt.status == ClipPlugin::EVT_NORMAL ? ret : static_cast<int32_t>(PasteboardError::INVALID_EVENT_STATUS);
    ret = curTime < evt.expiration ? ret : static_cast<int32_t>(PasteboardError::DATA_EXPIRED_ERROR);
    return std::make_pair(ret, evt);
}

std::vector<std::string> PasteboardService::GetLocalMimeTypes()
{
    auto userId = GetCurrentAccountId();
    auto it = clips_.Find(userId);
    if (!it.first) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "can not find data. userId: %{public}d", userId);
        return {};
    }
    if (it.second == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "data is nullptr. userId: %{public}d", userId);
        return {};
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto ret = IsDataVaild(*(it.second), tokenId);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "pasteData is invalid, tokenId is %{public}d, userId: %{public}d, ret is %{public}d",
            tokenId, userId, ret);
        return {};
    }
    return it.second->GetMimeTypes();
}

bool PasteboardService::HasLocalDataType(const std::string &mimeType)
{
    auto userId = GetCurrentAccountId();
    auto it = clips_.Find(userId);
    if (!it.first) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "can not find data. userId: %{public}d, mimeType: %{public}s",
            userId, mimeType.c_str());
        return false;
    }
    if (it.second == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "data is nullptr. userId: %{public}d, mimeType: %{public}s",
            userId, mimeType.c_str());
        return false;
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto ret = IsDataVaild(*(it.second), tokenId);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "pasteData is invalid, tokenId is %{public}d, userId: %{public}d,"
            "mimeType: %{public}s, ret is %{public}d",
            tokenId, userId, mimeType.c_str(), ret);
        return false;
    }
    auto screenStatus = GetCurrentScreenStatus();
    if (it.second->GetScreenStatus() > screenStatus) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "current screen is %{public}d, set data screen is %{public}d."
            "userId: %{public}d, mimeType: %{public}s",
            screenStatus, it.second->GetScreenStatus(), userId, mimeType.c_str());
        return false;
    }
    std::vector<std::string> mimeTypes = it.second->GetMimeTypes();
    auto isExistType = std::find(mimeTypes.begin(), mimeTypes.end(), mimeType) != mimeTypes.end();
    return isExistType;
}

bool PasteboardService::IsRemoteData()
{
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId is error.");
        return false;
    }
    auto it = clips_.Find(userId);
    if (!it.first) {
        auto [distRet, distEvt] = GetValidDistributeEvent(userId);
        return distRet == static_cast<int32_t>(PasteboardError::E_OK);
    }
    return it.second->IsRemote();
}

int32_t PasteboardService::GetDataSource(std::string &bundleName)
{
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        return static_cast<int32_t>(PasteboardError::INVALID_USERID_ERROR);
    }
    auto it = clips_.Find(userId);
    if (!it.first) {
        return static_cast<int32_t>(PasteboardError::NO_USER_DATA_ERROR);
    }
    auto data = it.second;
    if (data->IsRemote()) {
        return static_cast<int32_t>(PasteboardError::REMOTE_EXCEPTION);
    }
    auto tokenId = data->GetTokenId();
    bundleName = GetAppLabel(tokenId);
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::SetPasteData(PasteData &pasteData,
    const sptr<IPasteboardDelayGetter> delayGetter, const sptr<IPasteboardEntryGetter> entryGetter)
{
    PasteboardWebController::GetInstance().SplitWebviewPasteData(pasteData);
    auto ret = SaveData(pasteData, delayGetter, entryGetter);
    if (entityObserverMap_.Size() != 0 && pasteData.HasMimeType(MIMETYPE_TEXT_PLAIN)) {
        RecognizePasteData(pasteData);
    }
    ReportUeCopyEvent(pasteData, ret);
    return ret;
}

void PasteboardService::RemovePasteData(const AppInfo &appInfo)
{
    clips_.ComputeIfPresent(appInfo.userId, [this](auto, auto &clip) {
        RevokeUriPermission(clip);
        return false;
    });
    delayGetters_.ComputeIfPresent(appInfo.userId, [](auto, auto &delayGetter) {
        RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_CHECK_SET_DELAY_COPY, DFX_SUCCESS, COVER_DELAY_DATA, DFX_SUCCESS);
        if (delayGetter.first != nullptr && delayGetter.second != nullptr) {
            delayGetter.first->AsObject()->RemoveDeathRecipient(delayGetter.second);
        }
        return false;
    });
    entryGetters_.ComputeIfPresent(appInfo.userId, [](auto, auto &entryGetter) {
        if (entryGetter.first != nullptr && entryGetter.second != nullptr) {
            entryGetter.first->AsObject()->RemoveDeathRecipient(entryGetter.second);
        }
        return false;
    });
}

int32_t PasteboardService::GetCurrentAccountId()
{
    if (currentUserId_ != ERROR_USERID) {
        return currentUserId_;
    }
    std::vector<int32_t> accountIds;
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(accountIds);
    if (ret != ERR_OK || accountIds.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "query active user failed errCode=%{public}d", ret);
        return ERROR_USERID;
    }
    currentUserId_ = accountIds.front();
    return currentUserId_;
}

ScreenEvent PasteboardService::GetCurrentScreenStatus()
{
    if (currentScreenStatus != ScreenEvent::Default) {
        return currentScreenStatus;
    }

    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "query current screen status failed.");
    return ScreenEvent::Default;
}

bool PasteboardService::IsCopyable(uint32_t tokenId) const
{
#ifdef WITH_DLP
    bool copyable = false;
    auto ret = Security::DlpPermission::DlpPermissionKit::QueryDlpFileCopyableByTokenId(copyable, tokenId);
    if (ret != Security::DlpPermission::DLP_OK || !copyable) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "tokenId = 0x%{public}x ret = %{public}d, copyable = %{public}d.",
            tokenId, ret, copyable);
        return false;
    }
#endif
    return true;
}

void PasteboardService::SetInputMethodPid(pid_t callPid)
{
    auto imc = InputMethodController::GetInstance();
    if (imc == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "InputMethodController is nullptr!");
        return;
    }
    auto isImePid = imc->IsCurrentImeByPid(callPid);
    if (isImePid) {
        std::lock_guard lock(imeMutex_);
        imePid_ = callPid;
        hasImeObserver_ = true;
    }
}

void PasteboardService::ClearInputMethodPidByPid(pid_t callPid)
{
    if (callPid == imePid_) {
        std::lock_guard lock(imeMutex_);
        imePid_ = -1;
        hasImeObserver_ = false;
    }
}

void PasteboardService::ClearInputMethodPid()
{
    std::lock_guard lock(imeMutex_);
    imePid_ = -1;
    hasImeObserver_ = false;
}

void PasteboardService::SubscribeObserver(PasteboardObserverType type, const sptr<IPasteboardChangedObserver> &observer)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    SetInputMethodPid(callPid);
    bool isEventType = static_cast<uint32_t>(type) & static_cast<uint32_t>(PasteboardObserverType::OBSERVER_EVENT);
    int32_t userId = isEventType ? COMMON_USERID : GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return;
    }
    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(PasteboardObserverType::OBSERVER_LOCAL)) {
        AddObserver(userId, observer, observerLocalChangedMap_);
    }

    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(PasteboardObserverType::OBSERVER_REMOTE)) {
        AddObserver(userId, observer, observerRemoteChangedMap_);
    }

    if (isEventType && IsCallerUidValid()) {
        AddObserver(userId, observer, observerEventMap_);
    }
}

void PasteboardService::UnsubscribeObserver(
    PasteboardObserverType type, const sptr<IPasteboardChangedObserver> &observer)
{
    auto callPid = IPCSkeleton::GetCallingPid();
    ClearInputMethodPidByPid(callPid);
    bool isEventType = static_cast<uint32_t>(type) & static_cast<uint32_t>(PasteboardObserverType::OBSERVER_EVENT);
    int32_t userId = isEventType ? COMMON_USERID : GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return;
    }
    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(PasteboardObserverType::OBSERVER_LOCAL)) {
        RemoveSingleObserver(userId, observer, observerLocalChangedMap_);
    }

    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(PasteboardObserverType::OBSERVER_REMOTE)) {
        RemoveSingleObserver(userId, observer, observerRemoteChangedMap_);
    }

    if (isEventType && IsCallerUidValid()) {
        RemoveSingleObserver(userId, observer, observerEventMap_);
    }
}

void PasteboardService::UnsubscribeAllObserver(PasteboardObserverType type)
{
    ClearInputMethodPid();
    bool isEventType = static_cast<uint32_t>(type) & static_cast<uint32_t>(PasteboardObserverType::OBSERVER_EVENT);
    int32_t userId = isEventType ? COMMON_USERID : GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return;
    }
    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(PasteboardObserverType::OBSERVER_LOCAL)) {
        RemoveAllObserver(userId, observerLocalChangedMap_);
    }

    if (static_cast<uint32_t>(type) & static_cast<uint32_t>(PasteboardObserverType::OBSERVER_REMOTE)) {
        RemoveAllObserver(userId, observerRemoteChangedMap_);
    }

    if (isEventType && IsCallerUidValid()) {
        RemoveAllObserver(userId, observerEventMap_);
    }
}

uint32_t PasteboardService::GetAllObserversSize(int32_t userId, pid_t pid)
{
    auto localObserverSize = GetObserversSize(userId, pid, observerLocalChangedMap_);
    auto remoteObserverSize = GetObserversSize(userId, pid, observerRemoteChangedMap_);
    auto eventObserverSize = GetObserversSize(COMMON_USERID, pid, observerEventMap_);
    return localObserverSize + remoteObserverSize + eventObserverSize;
}

uint32_t PasteboardService::GetObserversSize(int32_t userId, pid_t pid, ObserverMap &observerMap)
{
    auto countKey = std::make_pair(userId, pid);
    auto it = observerMap.find(countKey);
    if (it != observerMap.end()) {
        return it->second->size();
    }
    return 0;
}

void PasteboardService::AddObserver(
    int32_t userId, const sptr<IPasteboardChangedObserver> &observer, ObserverMap &observerMap)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "observer null.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto callPid = IPCSkeleton::GetCallingPid();
    auto callObserverKey = std::make_pair(userId, callPid);
    auto it = observerMap.find(callObserverKey);
    std::shared_ptr<std::set<sptr<IPasteboardChangedObserver>, classcomp>> observers;
    if (it != observerMap.end()) {
        observers = it->second;
    } else {
        observers = std::make_shared<std::set<sptr<IPasteboardChangedObserver>, classcomp>>();
        observerMap.insert(std::make_pair(callObserverKey, observers));
    }
    auto allObserverCount = GetAllObserversSize(userId, callPid);
    if (allObserverCount >= MAX_OBSERVER_COUNT) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "observer count over limit. callPid:%{public}d", callPid);
        return;
    }
    observers->insert(observer);
    RADAR_REPORT(DFX_OBSERVER, DFX_ADD_OBSERVER, DFX_SUCCESS);
    PASTEBOARD_HILOGI(
        PASTEBOARD_MODULE_SERVICE, "observers->size = %{public}u.", static_cast<unsigned int>(observers->size()));
}

void PasteboardService::RemoveSingleObserver(
    int32_t userId, const sptr<IPasteboardChangedObserver> &observer, ObserverMap &observerMap)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "observer null.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto callPid = IPCSkeleton::GetCallingPid();
    auto callObserverKey = std::make_pair(userId, callPid);
    auto it = observerMap.find(callObserverKey);
    if (it == observerMap.end()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "user id not found userId is %{public}d", userId);
        return;
    }
    auto observers = it->second;
    PASTEBOARD_HILOGD(
        PASTEBOARD_MODULE_SERVICE, "observers size: %{public}u.", static_cast<unsigned int>(observers->size()));
    auto eraseNum = observers->erase(observer);
    RADAR_REPORT(DFX_OBSERVER, DFX_REMOVE_SINGLE_OBSERVER, DFX_SUCCESS);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers size = %{public}u, eraseNum = %{public}zu",
        static_cast<unsigned int>(observers->size()), eraseNum);
}

void PasteboardService::RemoveAllObserver(int32_t userId, ObserverMap &observerMap)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    for (auto it = observerMap.begin(); it != observerMap.end();) {
        if (it->first.first == userId) {
            it = observerMap.erase(it);
        } else {
            ++it;
        }
    }
    RADAR_REPORT(DFX_OBSERVER, DFX_REMOVE_ALL_OBSERVER, DFX_SUCCESS);
}

int32_t PasteboardService::SetGlobalShareOption(const std::map<uint32_t, ShareOption> &globalShareOptions)
{
    if (!IsCallerUidValid()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "No Permission");
        return static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR);
    }
    for (const auto &[tokenId, shareOption] : globalShareOptions) {
        GlobalShareOption option = {.source = MDM, .shareOption = shareOption};
        globalShareOptions_.InsertOrAssign(tokenId, option);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Set %{public}zu global shareOption.", globalShareOptions.size());
    return ERR_OK;
}

int32_t PasteboardService::RemoveGlobalShareOption(const std::vector<uint32_t> &tokenIds)
{
    if (!IsCallerUidValid()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "No Permission");
        return static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR);
    }
    int32_t count = 0;
    for (const uint32_t &tokenId : tokenIds) {
        globalShareOptions_.ComputeIfPresent(tokenId, [&count](const uint32_t &key, GlobalShareOption &value) {
            count++;
            return false;
        });
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Remove %{public}d global shareOption.", count);
    return ERR_OK;
}

std::map<uint32_t, ShareOption> PasteboardService::GetGlobalShareOption(const std::vector<uint32_t> &tokenIds)
{
    if (!IsCallerUidValid()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "No Permission");
        return {};
    }
    std::map<uint32_t, ShareOption> result;
    if (tokenIds.empty()) {
        globalShareOptions_.ForEach([&result](const uint32_t &key, GlobalShareOption &value) {
            result[key] = value.shareOption;
            return false;
        });
        return result;
    }
    for (const uint32_t &tokenId : tokenIds) {
        globalShareOptions_.ComputeIfPresent(tokenId, [&result](const uint32_t &key, GlobalShareOption &value) {
            result[key] = value.shareOption;
            return true;
        });
    }
    return result;
}

bool PasteboardService::IsSystemAppByFullTokenID(uint64_t tokenId)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "called token id: %{public}" PRIu64, tokenId);
    return (tokenId & SYSTEM_APP_MASK) == SYSTEM_APP_MASK;
}

int32_t PasteboardService::SetAppShareOptions(const ShareOption &shareOptions)
{
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!IsSystemAppByFullTokenID(fullTokenId)) {
        if (shareOptions != InApp) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "param is invalid");
            return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
        }
        auto isManageGrant = IsPermissionGranted(MANAGE_PASTEBOARD_APP_SHARE_OPTION_PERMISSION, tokenId);
        if (!isManageGrant) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "No permission, token id: 0x%{public}x.", tokenId);
            return static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR);
        }
    }
    GlobalShareOption option = {.source = APP, .shareOption = shareOptions};
    auto isAbsent = globalShareOptions_.ComputeIfAbsent(tokenId, [&option](const uint32_t &tokenId) {
        return option;
    });
    if (!isAbsent) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Settings already exist, token id: 0x%{public}x.", tokenId);
        return static_cast<int32_t>(PasteboardError::INVALID_OPERATION_ERROR);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Set token id: 0x%{public}x share options: %{public}d success.",
        tokenId, shareOptions);
    return 0;
}

int32_t PasteboardService::RemoveAppShareOptions()
{
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!IsSystemAppByFullTokenID(fullTokenId)) {
        auto isManageGrant = IsPermissionGranted(MANAGE_PASTEBOARD_APP_SHARE_OPTION_PERMISSION, tokenId);
        if (!isManageGrant) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "No permission, token id: 0x%{public}x.", tokenId);
            return static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR);
        }
    }
    std::map<uint32_t, GlobalShareOption> result;
    globalShareOptions_.ComputeIfPresent(tokenId, [&result](const uint32_t &key, GlobalShareOption &value) {
        result[key] = value;
        return true;
    });
    if (!result.empty()) {
        if (result[tokenId].source == APP) {
            globalShareOptions_.Erase(tokenId);
            PASTEBOARD_HILOGI(
                PASTEBOARD_MODULE_SERVICE, "Remove token id: 0x%{public}x share options success.", tokenId);
            return 0;
        } else {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Can not remove token id: 0x%{public}x.", tokenId);
            return 0;
        }
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "This token id: 0x%{public}x not set.", tokenId);
    return 0;
}

void PasteboardService::UpdateShareOption(PasteData &pasteData)
{
    globalShareOptions_.ComputeIfPresent(
        pasteData.GetTokenId(), [&pasteData](const uint32_t &tokenId, GlobalShareOption &option) {
            pasteData.SetShareOption(option.shareOption);
            return true;
        });
}

bool PasteboardService::CheckMdmShareOption(PasteData &pasteData)
{
    bool result = false;
    globalShareOptions_.ComputeIfPresent(
        pasteData.GetTokenId(), [&result](const uint32_t &tokenId, GlobalShareOption &option) {
            if (option.source == MDM) {
                result = true;
            }
            return true;
        });
    return result;
}

inline bool PasteboardService::IsCallerUidValid()
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid == EDM_UID || (uid_ != -1 && callingUid == uid_)) {
        return true;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "callingUid error: %{public}d.", callingUid);
    return false;
}

void PasteboardService::ThawInputMethod()
{
    auto type = ResourceSchedule::ResType::RES_TYPE_SA_CONTROL_APP_EVENT;
    auto status = ResourceSchedule::ResType::SaControlAppStatus::SA_START_APP;
    pid_t imePid = -1;
    {
        std::lock_guard lock(imeMutex_);
        imePid = imePid_;
    }
    std::unordered_map<std::string, std::string> payload = {
        { "saId", std::to_string(PASTEBOARD_SERVICE_ID) },
        { "saName", PASTEBOARD_SERVICE_SA_NAME },
        { "extensionType", std::to_string(static_cast<int32_t>(AppExecFwk::ExtensionAbilityType::INPUTMETHOD)) },
        { "pid", std::to_string(imePid) },
        { "isDelay", std::to_string(true) } };
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "report RSS need thaw:pid = %{public}d", imePid);
    ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, status, payload);
}

bool PasteboardService::IsNeedThaw()
{
    auto imc = InputMethodController::GetInstance();
    if (imc == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "InputMethodController is nullptr!");
        return false;
    }

    std::shared_ptr<Property> property;
    int32_t ret = imc->GetDefaultInputMethod(property);
    if (ret != ErrorCode::NO_ERROR || property == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "default input method is nullptr!");
        return false;
    }
    return true;
}
void PasteboardService::NotifyObservers(std::string bundleName, PasteboardEventStatus status)
{
    if (hasImeObserver_ && IsNeedThaw()) {
        ThawInputMethod();
    }
    std::thread thread([this, bundleName, status]() {
        std::lock_guard<std::mutex> lock(observerMutex_);
        for (auto &observers : observerLocalChangedMap_) {
            for (const auto &observer : *(observers.second)) {
                if (status != PasteboardEventStatus::PASTEBOARD_READ) {
                    observer->OnPasteboardChanged();
                }
            }
        }
        for (auto &observers : observerEventMap_) {
            for (const auto &observer : *(observers.second)) {
                observer->OnPasteboardEvent(bundleName, static_cast<int32_t>(status));
            }
        }
    });
    thread.detach();
}

size_t PasteboardService::GetDataSize(PasteData &data) const
{
    if (data.GetRecordCount() != 0) {
        size_t counts = data.GetRecordCount() - 1;
        std::shared_ptr<PasteDataRecord> records = data.GetRecordAt(counts);
        std::string text = records->ConvertToText();
        size_t textSize = text.size();
        return textSize;
    }
    return 0; // get wrong size
}

bool PasteboardService::SetPasteboardHistory(HistoryInfo &info)
{
    std::string history = std::move(info.time) + " " + std::move(info.bundleName) + " " + std::move(info.state) + " " +
                          " " + std::move(info.remote);
    constexpr const size_t DATA_HISTORY_SIZE = 10;
    std::lock_guard<decltype(historyMutex_)> lg(historyMutex_);
    if (dataHistory_.size() == DATA_HISTORY_SIZE) {
        dataHistory_.erase(dataHistory_.begin());
    }
    dataHistory_.push_back(std::move(history));
    return true;
}

int PasteboardService::Dump(int fd, const std::vector<std::u16string> &args)
{
    int uid = static_cast<int>(IPCSkeleton::GetCallingUid());
    const int maxUid = 10000;
    if (uid > maxUid) {
        return 0;
    }

    std::vector<std::string> argsStr;
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }

    if (PasteboardDumpHelper::GetInstance().Dump(fd, argsStr)) {
        return 0;
    }
    return 0;
}

std::string PasteboardService::GetTime()
{
    constexpr int USEC_TO_MSEC = 1000;
    time_t timeSeconds = time(0);
    if (timeSeconds == -1) {
        return FAIL_TO_GET_TIME_STAMP;
    }
    struct tm nowTime;
    localtime_r(&timeSeconds, &nowTime);

    struct timeval timeVal = { 0, 0 };
    gettimeofday(&timeVal, nullptr);

    std::string targetTime = std::to_string(nowTime.tm_year + 1900) + "-" + std::to_string(nowTime.tm_mon + 1) + "-" +
                             std::to_string(nowTime.tm_mday) + " " + std::to_string(nowTime.tm_hour) + ":" +
                             std::to_string(nowTime.tm_min) + ":" + std::to_string(nowTime.tm_sec) + "." +
                             std::to_string(timeVal.tv_usec / USEC_TO_MSEC);
    return targetTime;
}

std::string PasteboardService::DumpHistory() const
{
    std::string result;
    std::lock_guard<decltype(historyMutex_)> lg(historyMutex_);
    if (!dataHistory_.empty()) {
        result.append("Access history last ten times: ").append("\n");
        for (auto iter = dataHistory_.rbegin(); iter != dataHistory_.rend(); ++iter) {
            result.append("          ").append(*iter).append("\n");
        }
    } else {
        result.append("Access history fail! dataHistory_ no data.").append("\n");
    }
    return result;
}

std::string PasteboardService::DumpData()
{
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "query active user failed.");
        return "";
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "id = %{public}d", userId);
    auto it = clips_.Find(userId);
    std::string result;
    if (it.first && it.second != nullptr) {
        size_t recordCounts = it.second->GetRecordCount();
        auto property = it.second->GetProperty();
        std::string shareOption;
        PasteData::ShareOptionToString(property.shareOption, shareOption);
        std::string sourceDevice;
        if (property.isRemote) {
            sourceDevice = "remote";
        } else {
            sourceDevice = "local";
        }
        result.append("|Owner       :  ")
            .append(property.bundleName)
            .append("\n")
            .append("|Timestamp   :  ")
            .append(property.setTime)
            .append("\n")
            .append("|Share Option:  ")
            .append(shareOption)
            .append("\n")
            .append("|Record Count:  ")
            .append(std::to_string(recordCounts))
            .append("\n")
            .append("|Mime types  :  {");
        if (!property.mimeTypes.empty()) {
            for (size_t i = 0; i < property.mimeTypes.size(); ++i) {
                result.append(property.mimeTypes[i]).append(",");
            }
        }
        result.append("}").append("\n").append("|source device:  ").append(sourceDevice);
    } else {
        result.append("No copy data.").append("\n");
    }
    return result;
}

bool PasteboardService::IsFocusedApp(uint32_t tokenId)
{
    if (AccessTokenKit::GetTokenTypeFlag(tokenId) != ATokenTypeEnum::TOKEN_HAP) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "caller is not application");
        return true;
    }
    FocusChangeInfo info;
#ifdef SCENE_BOARD_ENABLE
    WindowManagerLite::GetInstance().GetFocusWindowInfo(info);
#else
    WindowManager::GetInstance().GetFocusWindowInfo(info);
#endif
    auto callPid = IPCSkeleton::GetCallingPid();
    if (callPid == info.pid_) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "pid is same, it is focused app");
        return true;
    }
    bool isFocused = false;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->CheckUIExtensionIsFocused(tokenId, isFocused);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "check result:%{public}d, isFocused:%{public}d", ret, isFocused);
    return ret == NO_ERROR && isFocused;
}

FocusedAppInfo PasteboardService::GetFocusedAppInfo(void) const
{
    FocusedAppInfo appInfo = { 0 };
    FocusChangeInfo info;
#ifdef SCENE_BOARD_ENABLE
    WindowManagerLite::GetInstance().GetFocusWindowInfo(info);
#else
    WindowManager::GetInstance().GetFocusWindowInfo(info);
#endif
    appInfo.windowId = info.windowId_;
    appInfo.abilityToken = info.abilityToken_;
    return appInfo;
}

void PasteboardService::SetPasteDataDot(PasteData &pasteData)
{
    auto bundleName = pasteData.GetBundleName();
    HistoryInfo info{ pasteData.GetTime(), bundleName, "set", "" };
    SetPasteboardHistory(info);

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetPasteData Report!");
    Reporter::GetInstance().PasteboardBehaviour().Report(
        { static_cast<int>(BehaviourPasteboardState::BPS_COPY_STATE), bundleName });

    int state = static_cast<int>(StatisticPasteboardState::SPS_COPY_STATE);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetPasteData GetDataSize!");
    size_t dataSize = GetDataSize(pasteData);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetPasteData timeC!");
    CalculateTimeConsuming timeC(dataSize, state);
}

void PasteboardService::GetPasteDataDot(PasteData &pasteData, const std::string &bundleName)
{
    std::string remote;
    if (pasteData.IsRemote()) {
        remote = "remote";
    }
    std::string time = GetTime();
    HistoryInfo info{ time, bundleName, "get", remote };
    SetPasteboardHistory(info);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData Report!");
    int pState = StatisticPasteboardState::SPS_INVALID_STATE;
    int bState = BehaviourPasteboardState::BPS_INVALID_STATE;
    if (pasteData.IsRemote()) {
        pState = static_cast<int>(StatisticPasteboardState::SPS_REMOTE_PASTE_STATE);
        bState = static_cast<int>(BehaviourPasteboardState::BPS_REMOTE_PASTE_STATE);
    } else {
        pState = static_cast<int>(StatisticPasteboardState::SPS_PASTE_STATE);
        bState = static_cast<int>(BehaviourPasteboardState::BPS_PASTE_STATE);
    };

    Reporter::GetInstance().PasteboardBehaviour().Report({ bState, bundleName });
    size_t dataSize = GetDataSize(pasteData);
    CalculateTimeConsuming timeC(dataSize, pState);
}

std::pair<std::shared_ptr<PasteData>, PasteDateResult> PasteboardService::GetDistributedData(
    const Event &event, int32_t user)
{
    auto clipPlugin = GetClipPlugin();
    PasteDateResult pasteDateResult;
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        pasteDateResult.syncTime = -1;
        pasteDateResult.errorCode = static_cast<int32_t>(PasteboardError::REMOTE_TASK_ERROR);
        return std::make_pair(nullptr, pasteDateResult);
    }
    std::vector<uint8_t> rawData;
    auto result = clipPlugin->GetPasteData(event, rawData);
    if (result.first != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get data failed");
        Reporter::GetInstance().PasteboardFault().Report({ user, "GET_REMOTE_DATA_FAILED" });
        pasteDateResult.syncTime = -1;
        pasteDateResult.errorCode = result.first;
        return std::make_pair(nullptr, pasteDateResult);
    }

    currentEvent_ = std::move(event);
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->Decode(rawData);
    pasteData->SetOriginAuthority(pasteData->GetBundleName());
    for (size_t i = 0; i < pasteData->GetRecordCount(); i++) {
        auto item = pasteData->GetRecordAt(i);
        if (item == nullptr || item->GetConvertUri().empty()) {
            continue;
        }
        item->isConvertUriFromRemote = true;
    }
    pasteDateResult.syncTime = result.second;
    pasteDateResult.errorCode = static_cast<int32_t>(PasteboardError::E_OK);
    return std::make_pair(pasteData, pasteDateResult);
}

bool PasteboardService::IsAllowSendData()
{
    auto controlType =
        system::GetIntParameter(TRANSMIT_CONTROL_PROP_KEY, CONTROL_TYPE_ALLOW_SEND_RECEIVE, INT_MIN, INT_MAX);
    if (controlType != CONTROL_TYPE_ALLOW_SEND_RECEIVE) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "control type is: %{public}d.", controlType);
        return false;
    }
    return true;
}

uint8_t PasteboardService::GenerateDataType(PasteData &data)
{
    std::vector<std::string> mimeTypes = data.GetMimeTypes();
    if (mimeTypes.empty()) {
        return 0;
    }
    std::bitset<MAX_INDEX_LENGTH> dataType(0);
    for (size_t i = 0; i < mimeTypes.size(); i++) {
        auto it = typeMap_.find(mimeTypes[i]);
        if (it == typeMap_.end()) {
            continue;
        }
        auto index = it->second;
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "mimetype is exist index=%{public}d", index);
        if (it->second == HTML_INDEX && data.GetTag() == PasteData::WEBVIEW_PASTEDATA_TAG) {
            dataType.reset();
            dataType.set(index);
            break;
        }
        dataType.set(index);
    }
    auto types = dataType.to_ulong();
    uint8_t value = types & 0xff;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "value = %{public}d", value);
    return value;
}

bool PasteboardService::IsDisallowDistributed()
{
    pid_t uid = IPCSkeleton::GetCallingUid();
    if (uid == DEVICE_COLLABORATION_UID) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "uid from device collaboration");
        return true;
    }
    return false;
}

bool PasteboardService::SetDistributedData(int32_t user, PasteData &data)
{
    if (!IsAllowSendData() || IsDisallowDistributed()) {
        return false;
    }
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_CHECK_ONLINE_DEVICE, DFX_SUCCESS);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clip plugin is null, dataId:%{public}u", data.GetDataId());
        return false;
    }
    ShareOption shareOpt = data.GetShareOption();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(shareOpt != InApp, false, PASTEBOARD_MODULE_SERVICE,
        "data share option is in app, dataId:%{public}u", data.GetDataId());
    if (CheckMdmShareOption(data)) {
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(shareOpt != LocalDevice, false, PASTEBOARD_MODULE_SERVICE,
            "data share option is local device, dataId:%{public}u", data.GetDataId());
    }
    auto networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(!networkId.empty(), false, PASTEBOARD_MODULE_SERVICE, "networkId is empty.");
    Event event;
    event.user = user;
    event.seqId = ++sequenceId_;
    auto expiration =
        duration_cast<milliseconds>((system_clock::now() + minutes(EXPIRATION_INTERVAL)).time_since_epoch()).count();
    event.expiration = static_cast<uint64_t>(expiration);
    event.deviceId = networkId;
    event.account = AccountManager::GetInstance().GetCurrentAccount();
    event.status = ClipPlugin::EVT_NORMAL;
    event.dataType = data.GetMimeTypes();
    event.isDelay = data.IsDelayRecord();
    event.dataId = data.GetDataId();
    currentEvent_ = event;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "dataId:%{public}u, seqId:%{public}hu, isDelay:%{public}d,"
        "expiration:%{public}" PRIu64, event.dataId, event.seqId, event.isDelay, event.expiration);
    return SetCurrentDistributedData(data, event);
}

bool PasteboardService::SetCurrentDistributedData(PasteData &data, Event event)
{
    std::thread thread([this, data, event]() mutable {
        {
            std::lock_guard<std::mutex> lock(setDistributedMemory_.mutex);
            setDistributedMemory_.event = event;
            setDistributedMemory_.data = std::make_shared<PasteData>(data);
            if (setDistributedMemory_.isRunning) {
                return;
            }
            setDistributedMemory_.isRunning = true;
        }
        bool isNeedCheck = false;
        while (true) {
            auto block = std::make_shared<BlockObject<bool>>(SET_DISTRIBUTED_DATA_INTERVAL, false);
            {
                std::lock_guard<std::mutex> lock(setDistributedMemory_.mutex);
                if ((event.seqId == setDistributedMemory_.event.seqId && isNeedCheck) ||
                    setDistributedMemory_.data == nullptr) {
                    setDistributedMemory_.data = nullptr;
                    setDistributedMemory_.isRunning = false;
                    break;
                }
                if (!isNeedCheck) {
                    isNeedCheck = true;
                }
                std::thread thread([this, &event, &block]() mutable {
                    PasteData data;
                    {
                        std::lock_guard<std::mutex> lock(setDistributedMemory_.mutex);
                        if (setDistributedMemory_.data == nullptr) {
                            block->SetValue(true);
                            return;
                        }
                        event = setDistributedMemory_.event;
                        data = *setDistributedMemory_.data;
                    }
                    auto result = SetCurrentData(event, data);
                    block->SetValue(true);
                });
                thread.detach();
            }
            bool ret = block->GetValue();
            if (!ret) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "SetCurrentData timeout,seqId:%{public}hu", event.seqId);
            }
        }
    });
    thread.detach();
    return true;
}

bool PasteboardService::SetCurrentData(Event event, PasteData &data)
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_CHECK_ONLINE_DEVICE, DFX_SUCCESS);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clip plugin is null, dataId:%{public}u", data.GetDataId());
        return false;
    }
    RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_LOAD_DISTRIBUTED_PLUGIN, DFX_SUCCESS);
    bool needFull = data.IsDelayRecord() && moduleConfig_.GetRemoteDeviceMinVersion() == DevProfile::FIRST_VERSION;
    if (needFull) {
        GetFullDelayPasteData(event.user, data);
        event.isDelay = false;
        PasteboardWebController::GetInstance().SplitWebviewPasteData(data);
        PasteboardWebController::GetInstance().SetWebviewPasteData(data, data.GetOriginAuthority());
        PasteboardWebController::GetInstance().CheckAppUriPermission(data);
    }
    GenerateDistributedUri(data);
    std::vector<uint8_t> rawData;
    if (!data.Encode(rawData)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "distributed data encode failed, dataId:%{public}u, seqId:%{public}hu", event.dataId, event.seqId);
        return false;
    }
    if (data.IsDelayRecord() && !needFull) {
        clipPlugin->RegisterDelayCallback(
            std::bind(&PasteboardService::GetDistributedDelayData, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3),
            std::bind(&PasteboardService::GetDistributedDelayEntry, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    }
    clipPlugin->SetPasteData(event, rawData);
    return true;
}

int32_t PasteboardService::GetDistributedDelayEntry(const Event &evt, uint32_t recordId, const std::string &utdId,
    std::vector<uint8_t> &rawData)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "dataId:%{public}u, seqId:%{public}hu, expiration:%{public}" PRIu64
        ", recordId:%{public}u, type:%{public}s", evt.dataId, evt.seqId, evt.expiration, recordId, utdId.c_str());
    auto [hasData, data] = clips_.Find(evt.user);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(hasData && data, static_cast<int32_t>(PasteboardError::NO_DATA_ERROR),
        PASTEBOARD_MODULE_SERVICE, "data not find, userId=%{public}u", evt.user);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(evt.dataId == data->GetDataId(),
        static_cast<int32_t>(PasteboardError::INVALID_DATA_ID), PASTEBOARD_MODULE_SERVICE,
        "dataId=%{public}u mismatch, local=%{public}u", evt.dataId, data->GetDataId());

    auto record = data->GetRecordById(recordId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(record != nullptr, static_cast<int32_t>(PasteboardError::INVALID_RECORD_ID),
        PASTEBOARD_MODULE_SERVICE, "recordId=%{public}u invalid, max=%{public}zu", recordId, data->GetRecordCount());

    PasteDataEntry entry;
    entry.SetUtdId(utdId);
    int32_t ret = GetLocalEntryValue(evt.user, *data, *record, entry);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
        PASTEBOARD_MODULE_SERVICE, "get local entry failed, seqId=%{public}hu, dataId=%{public}u, recordId=%{public}u"
        ", type=%{public}s, ret=%{public}d", evt.seqId, evt.dataId, recordId, utdId.c_str(), ret);

    std::string mimeType = entry.GetMimeType();
    if (mimeType == MIMETYPE_TEXT_URI) {
        ret = ProcessDistributedDelayUri(evt.user, *data, entry, rawData);
    } else if (mimeType == MIMETYPE_TEXT_HTML) {
        ret = ProcessDistributedDelayHtml(*data, entry, rawData);
    } else {
        ret = ProcessDistributedDelayEntry(entry, rawData);
    }
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
        PASTEBOARD_MODULE_SERVICE, "process distributed entry failed, seqId=%{public}hu, dataId=%{public}u, "
        "recordId=%{public}u, type=%{public}s, ret=%{public}d", evt.seqId, evt.dataId, recordId, utdId.c_str(), ret);

    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "type=%{public}s, size=%{public}zu", utdId.c_str(), rawData.size());
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::ProcessDistributedDelayUri(int32_t userId, PasteData &data, PasteDataEntry &entry,
    std::vector<uint8_t> &rawData)
{
    auto uri = entry.ConvertToUri();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(uri != nullptr, static_cast<int32_t>(PasteboardError::GET_ENTRY_VALUE_FAILED),
        PASTEBOARD_MODULE_SERVICE, "convert entry to uri failed");

    PasteboardWebController::GetInstance().CheckAppUriPermission(data);
    std::string localUri = uri->ToString();
    HmdfsUriInfo dfsUri;
    int32_t ret = RemoteFileShare::GetDfsUriFromLocal(localUri, userId, dfsUri);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == 0, ret, PASTEBOARD_MODULE_SERVICE,
        "generate distributed uri failed, uri=%{private}s", localUri.c_str());

    std::string distributedUri = dfsUri.uriStr;
    size_t fileSize = dfsUri.fileSize;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "uri: %{private}s -> %{private}s, fileSize=%{public}zu",
        localUri.c_str(), distributedUri.c_str(), fileSize);

    auto entryValue = entry.GetValue();
    if (std::holds_alternative<std::string>(entryValue)) {
        entry.SetValue(distributedUri);
    } else if (std::holds_alternative<std::shared_ptr<Object>>(entryValue)) {
        auto object = std::get<std::shared_ptr<Object>>(entryValue);
        auto newObject = std::make_shared<Object>();
        newObject->value_ = object->value_;
        newObject->value_[UDMF::FILE_URI_PARAM] = distributedUri;
        entry.SetValue(newObject);
        entry.SetFileSize(static_cast<int64_t>(fileSize));
    }

    bool encodeSucc = entry.Encode(rawData);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(encodeSucc, static_cast<int32_t>(PasteboardError::DATA_ENCODE_ERROR),
        PASTEBOARD_MODULE_SERVICE, "encode uri failed");
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::ProcessDistributedDelayHtml(PasteData &data, PasteDataEntry &entry,
    std::vector<uint8_t> &rawData)
{
    if (PasteboardWebController::GetInstance().SplitWebviewPasteData(data)) {
        PasteboardWebController::GetInstance().SetWebviewPasteData(data, data.GetOriginAuthority());
        PasteboardWebController::GetInstance().CheckAppUriPermission(data);
    }

    PasteData tmp;
    std::shared_ptr<std::string> html = entry.ConvertToHtml();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(html != nullptr, static_cast<int32_t>(PasteboardError::GET_ENTRY_VALUE_FAILED),
        PASTEBOARD_MODULE_SERVICE, "convert to html failed");

    tmp.AddHtmlRecord(*html);
    tmp.SetBundleName(data.GetBundleName());
    tmp.SetOriginAuthority(data.GetOriginAuthority());
    tmp.SetTokenId(data.GetTokenId());
    if (PasteboardWebController::GetInstance().SplitWebviewPasteData(tmp)) {
        PasteboardWebController::GetInstance().SetWebviewPasteData(tmp, data.GetOriginAuthority());
        PasteboardWebController::GetInstance().CheckAppUriPermission(tmp);
        GenerateDistributedUri(tmp);
    }

    bool encodeSucc = tmp.Encode(rawData);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(encodeSucc, static_cast<int32_t>(PasteboardError::DATA_ENCODE_ERROR),
        PASTEBOARD_MODULE_SERVICE, "encode html failed");
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::ProcessDistributedDelayEntry(PasteDataEntry &entry, std::vector<uint8_t> &rawData)
{
    bool encodeSucc = entry.Encode(rawData);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(encodeSucc, static_cast<int32_t>(PasteboardError::DATA_ENCODE_ERROR),
        PASTEBOARD_MODULE_SERVICE, "encode entry failed, type=%{public}s", entry.GetUtdId().c_str());
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::GetDistributedDelayData(const Event &evt, uint8_t version, std::vector<uint8_t> &rawData)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "dataId:%{public}u, seqId:%{public}hu, expiration:%{public}" PRIu64,
        evt.dataId, evt.seqId, evt.expiration);
    auto [hasData, data] = clips_.Find(evt.user);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(hasData && data, static_cast<int32_t>(PasteboardError::NO_DATA_ERROR),
        PASTEBOARD_MODULE_SERVICE, "data not find, userId=%{public}u", evt.user);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(evt.dataId == data->GetDataId(),
        static_cast<int32_t>(PasteboardError::INVALID_DATA_ID), PASTEBOARD_MODULE_SERVICE,
        "dataId=%{public}u mismatch, local=%{public}u", evt.dataId, data->GetDataId());

    int32_t ret = static_cast<int32_t>(PasteboardError::E_OK);
    if (version == 0) {
        ret = GetFullDelayPasteData(evt.user, *data);
    } else if (version == 1) {
        ret = GetDelayPasteRecord(evt.user, *data);
    }
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
        PASTEBOARD_MODULE_SERVICE, "get delay data failed, version=%{public}hhu", version);

    data->SetBundleName(data->GetOriginAuthority());
    PasteboardWebController::GetInstance().SplitWebviewPasteData(*data);
    PasteboardWebController::GetInstance().SetWebviewPasteData(*data, data->GetOriginAuthority());
    PasteboardWebController::GetInstance().CheckAppUriPermission(*data);
    GenerateDistributedUri(*data);

    bool encodeSucc = data->Encode(rawData);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(encodeSucc, static_cast<int32_t>(PasteboardError::DATA_ENCODE_ERROR),
        PASTEBOARD_MODULE_SERVICE, "encode data failed, dataId:%{public}u, seqId:%{public}hu", evt.dataId, evt.seqId);

    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "size=%{public}zu", rawData.size());
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::GetLocalEntryValue(int32_t userId, PasteData &data, PasteDataRecord &record,
    PasteDataEntry &value)
{
    std::string utdId = value.GetUtdId();
    auto entry = record.GetEntry(utdId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(entry != nullptr, static_cast<int32_t>(PasteboardError::INVALID_MIMETYPE),
        PASTEBOARD_MODULE_SERVICE, "entry is null, recordId=%{public}u, type=%{public}s", record.GetRecordId(),
        utdId.c_str());

    std::string mimeType = entry->GetMimeType();
    value.SetMimeType(mimeType);
    if (entry->HasContent(utdId)) {
        value.SetValue(entry->GetValue());
        return static_cast<int32_t>(PasteboardError::E_OK);
    }

    auto [hasGetter, getter] = entryGetters_.Find(userId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(hasGetter && getter.first,
        static_cast<int32_t>(PasteboardError::NO_DELAY_GETTER), PASTEBOARD_MODULE_SERVICE,
        "entry getter not find, userId=%{public}d, dataId=%{public}u", userId, data.GetDataId());

    int32_t ret = getter.first->GetRecordValueByType(record.GetRecordId(), value);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
        PASTEBOARD_MODULE_SERVICE, "get local entry failed, type=%{public}s, ret=%{public}d", utdId.c_str(), ret);

    record.AddEntry(utdId, std::make_shared<PasteDataEntry>(value));
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::GetRemoteEntryValue(const AppInfo &appInfo, PasteData &data, PasteDataRecord &record,
    PasteDataEntry &entry)
{
    auto clipPlugin = GetClipPlugin();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(clipPlugin != nullptr, static_cast<int32_t>(PasteboardError::PLUGIN_IS_NULL),
        PASTEBOARD_MODULE_SERVICE, "plugin is null");

    auto [distRet, distEvt] = GetValidDistributeEvent(appInfo.userId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(distRet == static_cast<int32_t>(PasteboardError::E_OK) ||
        distRet == static_cast<int32_t>(PasteboardError::GET_SAME_REMOTE_DATA), distRet,
        PASTEBOARD_MODULE_SERVICE, "get distribute event failed, ret=%{public}d", distRet);

    std::vector<uint8_t> rawData;
    std::string utdId = entry.GetUtdId();
    int32_t ret = clipPlugin->GetPasteDataEntry(distEvt, record.GetRecordId(), utdId, rawData);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == 0, ret, PASTEBOARD_MODULE_SERVICE, "get remote raw data failed");

    std::string mimeType = entry.GetMimeType();
    if (mimeType == MIMETYPE_TEXT_HTML) {
        ret = ProcessRemoteDelayHtml(distEvt.deviceId, appInfo.bundleName, rawData, data, record, entry);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
            PASTEBOARD_MODULE_SERVICE, "process remote delay html failed");
        return static_cast<int32_t>(PasteboardError::E_OK);
    }

    PasteDataEntry tmpEntry;
    tmpEntry.Decode(rawData);
    entry.SetValue(tmpEntry.GetValue());
    record.AddEntry(utdId, std::make_shared<PasteDataEntry>(entry));

    if (mimeType != MIMETYPE_TEXT_URI) {
        return static_cast<int32_t>(PasteboardError::E_OK);
    }

    auto uri = entry.ConvertToUri();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(uri != nullptr, static_cast<int32_t>(PasteboardError::GET_ENTRY_VALUE_FAILED),
        PASTEBOARD_MODULE_SERVICE, "convert entry to uri failed");
    std::string distributedUri = uri->ToString();
    record.SetConvertUri(distributedUri);
    record.isConvertUriFromRemote = true;
    record.SetGrantUriPermission(true);

    int64_t uriFileSize = entry.GetFileSize();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "uri=%{private}s, fileSize=%{public}" PRId64,
        distributedUri.c_str(), uriFileSize);
    if (uriFileSize > 0) {
        int64_t dataFileSize = data.GetFileSize();
        int64_t fileSize = (uriFileSize > INT64_MAX - dataFileSize) ? INT64_MAX : uriFileSize + dataFileSize;
        data.SetFileSize(fileSize);
    }
    std::vector<Uri> grantUris = CheckUriPermission(data, appInfo.bundleName);
    if (!grantUris.empty()) {
        EstablishP2PLink(distEvt.deviceId, data.GetPasteId());
        ret = GrantUriPermission(grantUris, appInfo.bundleName);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
            PASTEBOARD_MODULE_SERVICE, "grant remote uri failed, uri=%{private}s, ret=%{public}d",
            distributedUri.c_str(), ret);
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::ProcessRemoteDelayHtml(const std::string &remoteDeviceId, const std::string &bundleName,
    const std::vector<uint8_t> &rawData, PasteData &data, PasteDataRecord &record, PasteDataEntry &entry)
{
    PasteData tmpData;
    tmpData.Decode(rawData);
    auto htmlRecord = tmpData.GetRecordById(1);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(htmlRecord != nullptr,
        static_cast<int32_t>(PasteboardError::GET_ENTRY_VALUE_FAILED), PASTEBOARD_MODULE_SERVICE, "record is null");
    auto entryValue = htmlRecord->GetUDMFValue();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(entryValue != nullptr,
        static_cast<int32_t>(PasteboardError::GET_ENTRY_VALUE_FAILED), PASTEBOARD_MODULE_SERVICE, "udmfValue is null");
    entry.SetValue(*entryValue);
    record.AddEntry(entry.GetUtdId(), std::make_shared<PasteDataEntry>(entry));

    if (htmlRecord->GetFrom() == 0) {
        return static_cast<int32_t>(PasteboardError::E_OK);
    }

    data.SetTag(PasteData::WEBVIEW_PASTEDATA_TAG);
    uint32_t htmlRecordId = record.GetRecordId();
    record.SetFrom(htmlRecordId);
    for (auto &recordItem : tmpData.AllRecords()) {
        if (recordItem == nullptr) {
            continue;
        }
        if (!recordItem->GetConvertUri().empty()) {
            recordItem->isConvertUriFromRemote = true;
        }
        if (recordItem->GetFrom() > 0 && recordItem->GetRecordId() != recordItem->GetFrom()) {
            recordItem->SetFrom(htmlRecordId);
            data.AddRecord(*recordItem);
        }
    }

    int64_t htmlFileSize = tmpData.GetFileSize();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "htmlFileSize=%{public}" PRId64, htmlFileSize);
    if (htmlFileSize > 0) {
        int64_t dataFileSize = data.GetFileSize();
        int64_t fileSize = (htmlFileSize > INT64_MAX - dataFileSize) ? INT64_MAX : htmlFileSize + dataFileSize;
        data.SetFileSize(fileSize);
    }

    std::vector<Uri> grantUris = CheckUriPermission(data, bundleName);
    if (!grantUris.empty()) {
        EstablishP2PLink(remoteDeviceId, data.GetPasteId());
        int32_t ret = GrantUriPermission(grantUris, bundleName);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
            PASTEBOARD_MODULE_SERVICE, "grant to %{public}s failed, ret=%{public}d", bundleName.c_str(), ret);
    }

    tmpData.SetOriginAuthority(data.GetOriginAuthority());
    tmpData.SetTokenId(data.GetTokenId());
    tmpData.SetRemote(data.IsRemote());
    int32_t ret = PostProcessDelayHtmlEntry(tmpData, bundleName, entry);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == static_cast<int32_t>(PasteboardError::E_OK), ret,
        PASTEBOARD_MODULE_SERVICE, "post process remote html failed, ret=%{public}d", ret);
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::GetFullDelayPasteData(int32_t userId, PasteData &data)
{
    auto [hasGetter, getter] = entryGetters_.Find(userId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(hasGetter && getter.first,
        static_cast<int32_t>(PasteboardError::NO_DELAY_GETTER), PASTEBOARD_MODULE_SERVICE,
        "entry getter not find, userId=%{public}d, dataId=%{public}u", userId, data.GetDataId());

    for (auto record : data.AllRecords()) {
        if (!record->IsDelayRecord()) {
            continue;
        }
        auto recordId = record->GetRecordId();
        auto mimeType = record->GetMimeType();
        for (auto entry : record->GetEntries()) {
            if (!std::holds_alternative<std::monostate>(entry->GetValue())) {
                continue;
            }
            auto result = getter.first->GetRecordValueByType(recordId, *entry);
            if (result != static_cast<int32_t>(PasteboardError::E_OK)) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
                    "get record value fail, dataId is %{public}d, recordId is %{public}d, mimeType is %{public}s",
                    data.GetDataId(), record->GetRecordId(), entry->GetMimeType().c_str());
                continue;
            }
            if (entry->GetMimeType() == mimeType) {
                record->AddEntry(entry->GetUtdId(), std::make_shared<PasteDataEntry>(*entry));
            }
        }
    }
    PasteboardWebController::GetInstance().SplitWebviewPasteData(data);
    PasteboardWebController::GetInstance().SetWebviewPasteData(data, data.GetOriginAuthority());
    PasteboardWebController::GetInstance().CheckAppUriPermission(data);
    clips_.ComputeIfPresent(userId, [&data](auto, auto &value) {
        if (data.GetDataId() != value->GetDataId()) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
                "set data fail, data is out time, pre dataId is %{public}d, cur dataId is %{public}d",
                data.GetDataId(), value->GetDataId());
            return true;
        }
        value = std::make_shared<PasteData>(data);
        return true;
    });
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardService::GenerateDistributedUri(PasteData &data)
{
    std::vector<std::string> uris;
    std::vector<size_t> indexes;
    auto userId = GetCurrentAccountId();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(userId != ERROR_USERID, PASTEBOARD_MODULE_SERVICE, "invalid userId");
    for (size_t i = 0; i < data.GetRecordCount(); i++) {
        auto item = data.GetRecordAt(i);
        if (item == nullptr) {
            continue;
        }
        const auto &uri = item->GetOriginUri();
        if (uri == nullptr) {
            continue;
        }
        auto hasGrantUriPermission = item->HasGrantUriPermission();
        const std::string &bundleName = data.GetOriginAuthority();
        if (!IsBundleOwnUriPermission(bundleName, *uri) && !hasGrantUriPermission) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "uri:%{private}s, bundleName:%{public}s, has grant:%{public}d",
                uri->ToString().c_str(), bundleName.c_str(), hasGrantUriPermission);
            continue;
        }
        uris.emplace_back(uri->ToString());
        indexes.emplace_back(i);
    }
    size_t fileSize = 0;
    std::unordered_map<std::string, HmdfsUriInfo> dfsUris;
    if (!uris.empty()) {
        int ret = RemoteFileShare::GetDfsUrisFromLocal(uris, userId, dfsUris);
        PASTEBOARD_CHECK_AND_RETURN_LOGE((ret == 0 && !dfsUris.empty()), PASTEBOARD_MODULE_SERVICE,
            "Get remoteUri failed, ret:%{public}d, userId:%{public}d, uri size:%{public}zu.", ret, userId, uris.size());
        for (size_t i = 0; i < indexes.size(); i++) {
            auto item = data.GetRecordAt(indexes[i]);
            if (item == nullptr || item->GetOriginUri() == nullptr) {
                continue;
            }
            auto it = dfsUris.find(item->GetOriginUri()->ToString());
            if (it != dfsUris.end()) {
                item->SetConvertUri(it->second.uriStr);
                fileSize += it->second.fileSize;
            }
        }
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "file size: %{public}zu", fileSize);
    data.SetFileSize(static_cast<int64_t>(fileSize));
}

std::shared_ptr<ClipPlugin> PasteboardService::GetClipPlugin()
{
    auto isOn = moduleConfig_.IsOn();
    if (isOn) {
        auto securityLevel = securityLevel_.GetDeviceSecurityLevel();
        if (securityLevel < DATA_SEC_LEVEL3) {
            return nullptr;
        }
    }
    std::lock_guard<decltype(mutex)> lockGuard(mutex);
    if (!isOn || clipPlugin_ != nullptr) {
        return clipPlugin_;
    }
    Loader loader;
    loader.LoadComponents();
    auto release = [this](ClipPlugin *plugin) {
        std::lock_guard<decltype(mutex)> lockGuard(mutex);
        ClipPlugin::DestroyPlugin(PLUGIN_NAME, plugin);
    };

    clipPlugin_ = std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME), release);
    return clipPlugin_;
}

void PasteboardService::CleanDistributedData(int32_t user)
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return;
    }
    clipPlugin->Clear(user);
}

void PasteboardService::CloseDistributedStore(int32_t user, bool isNeedClear)
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return;
    }
    clipPlugin->Close(user, isNeedClear);
}

void PasteboardService::OnConfigChange(bool isOn)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "ConfigChange isOn: %{public}d.", isOn);
    p2pMap_.Clear();
    std::lock_guard<decltype(mutex)> lockGuard(mutex);
    if (!isOn) {
        int32_t userId = GetCurrentAccountId();
        clipPlugin_->Close(userId, false);
        clipPlugin_ = nullptr;
        return;
    }
    auto securityLevel = securityLevel_.GetDeviceSecurityLevel();
    if (securityLevel < DATA_SEC_LEVEL3) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "device sec level is %{public}u less than 3.", securityLevel);
        return;
    }
    if (clipPlugin_ != nullptr) {
        return;
    }
    SubscribeKeyboardEvent();
    Loader loader;
    loader.LoadComponents();
    auto release = [this](ClipPlugin *plugin) {
        std::lock_guard<decltype(mutex)> lockGuard(mutex);
        ClipPlugin::DestroyPlugin(PLUGIN_NAME, plugin);
    };

    clipPlugin_ = std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME), release);
}

std::string PasteboardService::GetAppLabel(uint32_t tokenId)
{
    auto iBundleMgr = GetAppBundleManager();
    if (iBundleMgr == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, " Failed to cast bundle mgr service.");
        return PasteBoardDialog::DEFAULT_LABEL;
    }
    AppInfo info = GetAppInfo(tokenId);
    AppExecFwk::ApplicationInfo appInfo;
    auto result = iBundleMgr->GetApplicationInfo(info.bundleName, 0, info.userId, appInfo);
    if (!result) {
        return PasteBoardDialog::DEFAULT_LABEL;
    }
    auto &resource = appInfo.labelResource;
    auto label = iBundleMgr->GetStringById(resource.bundleName, resource.moduleName, resource.id, info.userId);
    return label.empty() ? PasteBoardDialog::DEFAULT_LABEL : label;
}

sptr<AppExecFwk::IBundleMgr> PasteboardService::GetAppBundleManager()
{
    auto systemAbilityManager = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, " Failed to get SystemAbilityManager.");
        return nullptr;
    }
    auto remoteObject = systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (remoteObject == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, " Failed to get bundle mgr service.");
        return nullptr;
    }
    return OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
}

void PasteboardService::ChangeStoreStatus(int32_t userId)
{
    PasteboardService::currentUserId_ = userId;
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return;
    }
    clipPlugin->ChangeStoreStatus(userId);
}

void PasteBoardCommonEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    auto want = data.GetWant();
    std::string action = want.GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        std::lock_guard<std::mutex> lock(mutex_);
        int32_t userId = data.GetCode();
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "user id switched: %{public}d", userId);
        if (pasteboardService_ != nullptr) {
            pasteboardService_->ChangeStoreStatus(userId);
        }
        auto accountId = pasteboardService_->GetCurrentAccountId();
        pasteboardService_->switch_.DeInit();
        pasteboardService_->switch_.Init(accountId);
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "SetSwitch end");
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_STOPPING) {
        std::lock_guard<std::mutex> lock(mutex_);
        int32_t userId = data.GetCode();
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "user id is stopping: %{public}d", userId);
        if (pasteboardService_ != nullptr) {
            pasteboardService_->Clear();
        }
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED) {
        std::lock_guard<std::mutex> lock(mutex_);
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "screen is locked");
        PasteboardService::currentScreenStatus = ScreenEvent::ScreenLocked;
    } else if (action == EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED) {
        std::lock_guard<std::mutex> lock(mutex_);
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "screen is unlocked");
        PasteboardService::currentScreenStatus = ScreenEvent::ScreenUnlocked;
    }
}

void PasteBoardAccountStateSubscriber::OnStateChanged(const AccountSA::OsAccountStateData &data)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "state: %{public}d, fromId: %{public}d, toId: %{public}d,"
        "callback is nullptr: %{public}d", data.state, data.fromId, data.toId, data.callback == nullptr);
    if (data.state == AccountSA::OsAccountState::STOPPING && pasteboardService_ != nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        pasteboardService_->CloseDistributedStore(data.fromId, true);
    }
    if (data.callback != nullptr) {
        data.callback->OnComplete();
    }
}

bool PasteboardService::SubscribeKeyboardEvent()
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    if (inputEventCallback_ != nullptr) {
        return true;
    }
    inputEventCallback_ = std::make_shared<InputEventCallback>();
    int32_t monitorId = MMI::InputManager::GetInstance()->AddMonitor(
        std::static_pointer_cast<MMI::IInputEventConsumer>(inputEventCallback_), MMI::HANDLE_EVENT_TYPE_KEY);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "add monitor ret is: %{public}d", monitorId);
    return monitorId >= 0;
}

void PasteboardService::PasteboardEventSubscriber()
{
    EventCenter::GetInstance().Subscribe(PasteboardEvent::DISCONNECT, [this](const OHOS::MiscServices::Event &event) {
        auto &evt = static_cast<const PasteboardEvent &>(event);
        auto networkId = evt.GetNetworkId();
        if (networkId.empty()) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "networkId is empty.");
            return;
        }
        p2pMap_.EraseIf([networkId, this](auto &key, auto &value) {
            if (key == networkId) {
                CloseP2PLink(networkId);
                return true;
            }
            return false;
        });
    });
}

void PasteboardService::CommonEventSubscriber()
{
    if (commonEventSubscriber_ != nullptr) {
        return;
    }
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_STOPPING);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    commonEventSubscriber_ = std::make_shared<PasteBoardCommonEventSubscriber>(subscribeInfo, this);
    EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
}

void PasteboardService::AccountStateSubscriber()
{
    if (accountStateSubscriber_ != nullptr) {
        return;
    }
    std::set<AccountSA::OsAccountState> states = { AccountSA::OsAccountState::STOPPING,
        AccountSA::OsAccountState::CREATED, AccountSA::OsAccountState::SWITCHING,
        AccountSA::OsAccountState::SWITCHED, AccountSA::OsAccountState::UNLOCKED,
        AccountSA::OsAccountState::STOPPED, AccountSA::OsAccountState::REMOVED };
    AccountSA::OsAccountSubscribeInfo subscribeInfo(states, true);
    accountStateSubscriber_ = std::make_shared<PasteBoardAccountStateSubscriber>(subscribeInfo, this);
    AccountSA::OsAccountManager::SubscribeOsAccount(accountStateSubscriber_);
}

void PasteboardService::RemoveObserverByPid(int32_t userId, pid_t pid, ObserverMap &observerMap)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto callObserverKey = std::make_pair(userId, pid);
    auto it = observerMap.find(callObserverKey);
    if (it == observerMap.end()) {
        return;
    }
    observerMap.erase(callObserverKey);
}

int32_t PasteboardService::AppExit(pid_t pid)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "pid %{public}d exit.", pid);
    int32_t userId = GetCurrentAccountId();
    RemoveObserverByPid(userId, pid, observerLocalChangedMap_);
    RemoveObserverByPid(userId, pid, observerRemoteChangedMap_);
    RemoveObserverByPid(COMMON_USERID, pid, observerEventMap_);
    entityObserverMap_.Erase(pid);
    std::vector<std::string> networkIds;
    p2pMap_.EraseIf([pid, &networkIds, this](auto &networkId, auto &pidMap) {
        pidMap.EraseIf([pid, this](const auto &key, const auto &value) {
            if (value == pid) {
                PasteStart(key);
                return true;
            }
            return false;
        });
        if (pidMap.Empty()) {
            networkIds.emplace_back(networkId);
            return true;
        }
        return false;
    });
    for (const auto &id : networkIds) {
        CloseP2PLink(id);
    }
    clients_.Erase(pid);
    return ERR_OK;
}

void PasteboardService::PasteboardDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    (void)remote;
    service_.AppExit(pid_);
}

PasteboardService::PasteboardDeathRecipient::PasteboardDeathRecipient(
    PasteboardService &service, sptr<IRemoteObject> observer, pid_t pid)
    : service_(service), observer_(observer), pid_(pid)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Construct Pasteboard Client Death Recipient, pid: %{public}d", pid);
}

int32_t PasteboardService::RegisterClientDeathObserver(sptr<IRemoteObject> observer)
{
    pid_t pid = IPCSkeleton::GetCallingPid();
    sptr<PasteboardDeathRecipient> deathRecipient = new (std::nothrow) PasteboardDeathRecipient(*this, observer, pid);
    observer->AddDeathRecipient(deathRecipient);
    clients_.InsertOrAssign(pid, std::move(deathRecipient));
    return ERR_OK;
}

std::function<void(const OHOS::MiscServices::Event &)> PasteboardService::RemotePasteboardChange()
{
    return [this](const OHOS::MiscServices::Event &event) {
        (void)event;
        std::lock_guard<std::mutex> lock(observerMutex_);
        for (auto &observers : observerRemoteChangedMap_) {
            for (const auto &observer : *(observers.second)) {
                observer->OnPasteboardChanged();
            }
        }
    };
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    auto keyItems = keyEvent->GetKeyItems();
    if (keyItems.size() != CTRLV_EVENT_SIZE) {
        return;
    }
    if ((keyEvent->GetKeyAction() == MMI::KeyEvent::KEY_ACTION_DOWN) &&
        ((keyItems[0].GetKeyCode() == MMI::KeyEvent::KEYCODE_CTRL_LEFT) ||
        (keyItems[0].GetKeyCode() == MMI::KeyEvent::KEYCODE_CTRL_RIGHT)) &&
        keyItems[1].GetKeyCode() == MMI::KeyEvent::KEYCODE_V) {
        int32_t windowId = keyEvent->GetTargetWindowId();
        std::unique_lock<std::shared_mutex> lock(inputEventMutex_);
        windowPid_ = MMI::InputManager::GetInstance()->GetWindowPid(windowId);
        actionTime_ =
            static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
        std::shared_ptr<BlockObject<bool>> block = nullptr;
        {
            std::unique_lock<std::shared_mutex> blockMapLock(blockMapMutex_);
            auto it = blockMap_.find(windowPid_);
            if (it != blockMap_.end()) {
                block = it->second;
            } else {
                block = std::make_shared<BlockObject<bool>>(WAIT_TIME_OUT, false);
                blockMap_.insert(std::make_pair(windowPid_, block));
            }
        }
        if (block != nullptr) {
            block->SetValue(true);
        }
    }
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const {}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const {}

bool InputEventCallback::IsCtrlVProcess(uint32_t callingPid, bool isFocused)
{
    std::shared_ptr<BlockObject<bool>> block = nullptr;
    {
        std::unique_lock<std::shared_mutex> blockMapLock(blockMapMutex_);
        auto it = blockMap_.find(callingPid);
        if (it != blockMap_.end()) {
            block = it->second;
        } else {
            block = std::make_shared<BlockObject<bool>>(WAIT_TIME_OUT, false);
            blockMap_.insert(std::make_pair(callingPid, block));
        }
    }
    if (block != nullptr) {
        block->GetValue();
    }
    std::shared_lock<std::shared_mutex> lock(inputEventMutex_);
    auto curTime = static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    auto ret = (callingPid == static_cast<uint32_t>(windowPid_) || isFocused) && curTime - actionTime_ < EVENT_TIME_OUT;
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "windowPid is: %{public}d, callingPid is: %{public}d,"
            "curTime is: %{public}llu, actionTime is: %{public}llu, isFocused is: %{public}d", windowPid_, callingPid,
            curTime, actionTime_, isFocused);
    }
    return ret;
}

void InputEventCallback::Clear()
{
    std::unique_lock<std::shared_mutex> lock(inputEventMutex_);
    actionTime_ = 0;
    windowPid_ = 0;
    std::unique_lock<std::shared_mutex> blockMapLock(blockMapMutex_);
    blockMap_.clear();
}
} // namespace MiscServices
} // namespace OHOS
