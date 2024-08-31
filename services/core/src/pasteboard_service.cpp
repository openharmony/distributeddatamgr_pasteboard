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
#include "pasteboard_service.h"

#include <bitset>
#include <unistd.h>

#include "ability_manager_client.h"
#include "tokenid_kit.h"
#include "accesstoken_kit.h"
#include "account_manager.h"
#include "calculate_time_consuming.h"
#include "common_event_manager.h"
#include "dev_profile.h"
#include "device/dm_adapter.h"
#include "dfx_code_constant.h"
#include "dfx_types.h"
#include "distributed_file_daemon_manager.h"
#include "eventcenter/pasteboard_event.h"
#include "eventcenter/event_center.h"
#include "hiview_adapter.h"
#include "iservice_registry.h"
#include "loader.h"
#include "mem_mgr_client.h"
#include "mem_mgr_proxy.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "native_token_info.h"
#include "os_account_manager.h"
#include "parameters.h"
#include "pasteboard_dialog.h"
#include "pasteboard_event_dfx.h"
#include "pasteboard_error.h"
#include "pasteboard_trace.h"
#include "remote_file_share.h"
#include "reporter.h"
#include "screenlock_manager.h"
#include "tokenid_kit.h"
#include "uri_permission_manager_client.h"
#ifdef SCENE_BOARD_ENABLE
#include "window_manager_lite.h"
#else
#include "window_manager.h"
#endif

#ifdef WITH_DLP
#include "dlp_permission_kit.h"
#endif // WITH_DLP

namespace OHOS {
namespace MiscServices {
using namespace Rosen;
using namespace std::chrono;
using namespace Storage::DistributedFile;
using namespace RadarReporter;
namespace {
constexpr const int GET_WRONG_SIZE = 0;
constexpr const size_t MAX_URI_COUNT = 500;
constexpr const int32_t COMMON_USERID = 0;
const std::int32_t INIT_INTERVAL = 10000L;
constexpr const char* PASTEBOARD_SERVICE_NAME = "PasteboardService";
constexpr const char* FAIL_TO_GET_TIME_STAMP = "FAIL_TO_GET_TIME_STAMP";
constexpr const char* SECURE_PASTE_PERMISSION = "ohos.permission.SECURE_PASTE";
constexpr const char* READ_PASTEBOARD_PERMISSION = "ohos.permission.READ_PASTEBOARD";
constexpr const char* TRANSMIT_CONTROL_PROP_KEY = "persist.distributed_scene.datafiles_trans_ctrl";

const std::int32_t INVAILD_VERSION = -1;
const std::int32_t ADD_PERMISSION_CHECK_SDK_VERSION = 12;
const std::int32_t CTRLV_EVENT_SIZE = 2;
const std::int32_t CONTROL_TYPE_ALLOW_SEND_RECEIVE = 1;

const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(new PasteboardService());
} // namespace
using namespace Security::AccessToken;
using namespace OHOS::AppFileService::ModuleRemoteFileShare;
std::mutex PasteboardService::historyMutex_;
std::vector<std::string> PasteboardService::dataHistory_;
std::shared_ptr<Command> PasteboardService::copyHistory;
std::shared_ptr<Command> PasteboardService::copyData;
int32_t PasteboardService::currentUserId = ERROR_USERID;
ScreenEvent PasteboardService::currentScreenStatus = ScreenEvent::Default;

PasteboardService::PasteboardService()
    : SystemAbility(PASTEBOARD_SERVICE_ID, true), state_(ServiceRunningState::STATE_NOT_START)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PasteboardService Start.");
    ServiceListenerFunc_[static_cast<int32_t>(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID)] =
        &PasteboardService::DMAdapterInit;
    ServiceListenerFunc_[static_cast<int32_t>(MEMORY_MANAGER_SA_ID)] = &PasteboardService::NotifySaStatus;
}

PasteboardService::~PasteboardService()
{
}

int32_t PasteboardService::Init()
{
    if (!Publish(this)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OnStart register to system ability manager failed.");
        auto userId = GetCurrentAccountId();
        Reporter::GetInstance().PasteboardFault().Report({ userId, "ERR_INVALID_OPTION" });
        return static_cast<int32_t>(PasteboardError::E_INVALID_OPTION);
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
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "screen status is %{public}d",
        PasteboardService::currentScreenStatus);
#else
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PB_SCREENLOCK_MGR_ENABLE not defined");
    return;
#endif
}

void PasteboardService::OnStart()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardService OnStart.");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "PasteboardService is already running.");
        return;
    }

    InitServiceHandler();
    auto appInfo = GetAppInfo(IPCSkeleton::GetCallingTokenID());
    Loader loader;
    loader.LoadComponents();
    bundles_ = loader.LoadBundles();
    uid_ = loader.LoadUid();
    moduleConfig_.Init();
    auto ret = DATASL_OnStart();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "datasl on start ret:%{public}d", ret);
    moduleConfig_.Watch(std::bind(&PasteboardService::OnConfigChange, this, std::placeholders::_1));
    AddSysAbilityListener();
    if (Init() != ERR_OK) {
        auto callback = [this]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Init failed. Try again 10s later.");
        return;
    }
    switch_.Init();
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
    if (state_ != ServiceRunningState::STATE_RUNNING) {
        return;
    }
    serviceHandler_ = nullptr;
    state_ = ServiceRunningState::STATE_NOT_START;
    DMAdapter::GetInstance().UnInitialize();
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
        PASTEBOARD_HILOGD(
            PASTEBOARD_MODULE_SERVICE, "ret = %{public}d, serviceId = %{public}d.", ret, LISTENING_SERVICE[i]);
    }
}

void PasteboardService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "systemAbilityId = %{public}d added!", systemAbilityId);
    auto itFunc = ServiceListenerFunc_.find(systemAbilityId);
    if (itFunc != ServiceListenerFunc_.end()) {
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

void PasteboardService::DelayGetterDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start");
    (void) remote;
    service_.NotifyDelayGetterDied(userId_);
}

void PasteboardService::NotifyDelayGetterDied(int32_t userId)
{
    if (userId == ERROR_USERID) {
        return;
    }
    delayGetters_.Erase(userId);
}

PasteboardService::EntryGetterDeathRecipient::EntryGetterDeathRecipient(int32_t userId, PasteboardService &service)
    : userId_(userId), service_(service)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Construct Entry Getter Death Recipient");
}

void PasteboardService::EntryGetterDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start");
    (void) remote;
    service_.NotifyEntryGetterDied(userId_);
}

void PasteboardService::NotifyEntryGetterDied(int32_t userId)
{
    if (userId == ERROR_USERID) {
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
        auto appInfo = GetAppInfo(IPCSkeleton::GetCallingTokenID());
        std::string bundleName = GetAppBundleName(appInfo);
        NotifyObservers(bundleName, PasteboardEventStatus::PASTEBOARD_CLEAR);
    }
    CleanDistributedData(userId);
}

int32_t PasteboardService::GetRecordValueByType(uint32_t dataId, uint32_t recordId, PasteDataEntry& value)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto callPid = IPCSkeleton::GetCallingPid();
    if (!VerifyPermission(tokenId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "check permission failed, calling pid is %{public}d", callPid);
        return static_cast<int32_t>(PasteboardError::E_NO_PERMISSION);
    }
    auto appInfo = GetAppInfo(tokenId);
    auto clip = clips_.Find(appInfo.userId);
    auto tempTime = copyTime_.Find(appInfo.userId);
    auto entryGetter = entryGetters_.Find(appInfo.userId);
    if (!clip.first || !tempTime.first || !entryGetter.first) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "pasteboard has no data or entry getter");
        return static_cast<int32_t>(PasteboardError::E_ERROR);
    }
    auto data = clip.second;
    if (dataId != data->GetDataId()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "get record value fail, data is out time, pre dataId is %{public}d, cur dataId is %{public}d",
            dataId, data->GetDataId());
        return static_cast<int32_t>(PasteboardError::E_ERROR);
    }
    auto getter = entryGetter.second;
    if (getter.first == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "entry getter is nullptr, dataId is %{public}d, recordId is %{public}d", dataId, recordId);
        return static_cast<int32_t>(PasteboardError::E_ERROR);
    }
    auto result = getter.first->GetRecordValueByType(recordId, value);
    if (result != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "get record value fail, dataId is %{public}d, recordId is %{public}d", dataId, recordId);
        return result;
    }
    clips_.ComputeIfPresent(appInfo.userId, [dataId, recordId, value](auto, auto &data) {
        if (dataId != data->GetDataId()) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
                "set record value fail, data is out time, pre dataId is %{public}d, cur dataId is %{public}d",
                dataId, data->GetDataId());
            return true;
        }
        auto record = data->GetRecordAt(recordId - 1);
        if (record != nullptr) {
            record->AddEntry(value.GetUtdId(), std::make_shared<PasteDataEntry>(value));
        }
        return true;
    });
    return static_cast<int32_t>(PasteboardError::E_OK);
}

bool PasteboardService::IsDefaultIME(const AppInfo &appInfo)
{
    if (appInfo.tokenType != ATokenTypeEnum::TOKEN_HAP) {
        return true;
    }
    if (bundles_.empty()) {
        return true;
    }
    auto it = find(bundles_.begin(), bundles_.end(), appInfo.bundleName);
    if (it != bundles_.end()) {
        return true;
    }

    return false;
}

bool PasteboardService::VerifyPermission(uint32_t tokenId)
{
    auto version = GetSdkVersion(tokenId);
    auto callPid = IPCSkeleton::GetCallingPid();
    if (version == INVAILD_VERSION) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "get hap version failed, callPid is %{public}d, tokenId is %{public}d", callPid, tokenId);
        return false;
    }
    auto isReadGrant = IsPermissionGranted(READ_PASTEBOARD_PERMISSION, tokenId);
    auto isSecureGrant = IsPermissionGranted(SECURE_PASTE_PERMISSION, tokenId);
    AddPermissionRecord(tokenId, isReadGrant, isSecureGrant);
    auto isPrivilegeApp = IsDefaultIME(GetAppInfo(tokenId));
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE,
        "isReadGrant is %{public}d, isSecureGrant is %{public}d, isPrivilegeApp is %{public}d", isReadGrant,
        isSecureGrant, isPrivilegeApp);
    bool isCtrlVAction = false;
    if (inputEventCallback_ != nullptr) {
        isCtrlVAction = inputEventCallback_->IsCtrlVProcess(callPid, IsFocusedApp(tokenId));
        inputEventCallback_->Clear();
    }
    auto isGrant = isReadGrant || isSecureGrant || isPrivilegeApp || isCtrlVAction;
    if (!isGrant && version >= ADD_PERMISSION_CHECK_SDK_VERSION) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "no permisssion, callPid is %{public}d, version is %{public}d",
            callPid, version);
        return false;
    }
    return true;
}

bool PasteboardService::IsDataVaild(PasteData &pasteData, uint32_t tokenId)
{
    if (pasteData.IsDraggedData() || !pasteData.IsValid()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "data is invalid");
        return false;
    }
    if (IsDataAged()) {
        return false;
    }
    auto screenStatus = GetCurrentScreenStatus();
    if (pasteData.GetScreenStatus() > screenStatus) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "current screen is %{public}d, set data screen is %{public}d.",
            screenStatus, pasteData.GetScreenStatus());
        return false;
    }
    switch (pasteData.GetShareOption()) {
        case ShareOption::InApp: {
            if (pasteData.GetTokenId() != tokenId) {
                PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "InApp check failed.");
                return false;
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
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
                "tokenId = 0x%{public}x, shareOption = %{public}d is error.", tokenId, pasteData.GetShareOption());
            return false;
        }
    }
    return true;
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
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetHapTokenInfo fail, tokenid is %{public}d, ret is %{public}d.",
            tokenId, ret);
        return INVAILD_VERSION;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ver:%{public}d.", hapTokenInfo.apiVersion);
    return hapTokenInfo.apiVersion;
}

bool PasteboardService::IsPermissionGranted(const std::string& perm, uint32_t tokenId)
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
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "copyTime = %{public}" PRIu64, copyTime);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "curTime = %{public}" PRIu64, curTime);
    if (curTime > copyTime && curTime - copyTime > ONE_HOUR_MILLISECONDS) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "data is out of the time");
        auto data = clips_.Find(userId);
        if (data.first) {
            RevokeUriPermission(data.second);
            clips_.Erase(userId);
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

int32_t PasteboardService::GetPasteData(PasteData &data, int32_t &syncTime)
{
    PasteboardTrace tracer("PasteboardService GetPasteData");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto callPid = IPCSkeleton::GetCallingPid();
    auto appInfo = GetAppInfo(tokenId);
    bool developerMode = OHOS::system::GetBoolParameter("const.security.developermode.state", false);
    bool isTestServerSetPasteData = developerMode && setPasteDataUId_ == TESE_SERVER_UID;
    if (!VerifyPermission(tokenId) && !isTestServerSetPasteData) {
        RADAR_REPORT(DFX_GET_PASTEBOARD, DFX_CHECK_GET_AUTHORITY, DFX_SUCCESS, GET_DATA_APP, appInfo.bundleName,
            RadarReporter::CONCURRENT_ID, data.GetPasteId());
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "check permission failed, callingPid is %{public}d", callPid);
        return static_cast<int32_t>(PasteboardError::E_NO_PERMISSION);
    }
    auto ret = GetData(tokenId, data, syncTime);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "data is invalid, ret is %{public}d, callPid is %{public}d, tokenId is %{public}d", ret, callPid, tokenId);
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
    auto permUsedType = static_cast<PermissionUsedType>(AccessTokenKit::GetPermissionUsedType(tokenId,
        isSecureGrant ? SECURE_PASTE_PERMISSION : READ_PASTEBOARD_PERMISSION));
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
    std::string pasteId = data.GetPasteId();
    auto event = GetValidDistributeEvent(appInfo.userId);
    if (!event.first || GetCurrentScreenStatus() != ScreenEvent::ScreenUnlocked) {
        result = GetLocalData(appInfo, data);
    } else {
        result = GetRemoteData(appInfo.userId, event.second, data, syncTime);
        peerNetId = DMAdapter::GetInstance().GetUdidByNetworkId(event.second.deviceId);
    }
    if (observerEventMap_.size() != 0) {
        std::string targetBundleName = GetAppBundleName(appInfo);
        NotifyObservers(targetBundleName, PasteboardEventStatus::PASTEBOARD_READ);
    }
    RADAR_REPORT(DFX_GET_PASTEBOARD, DFX_GET_DATA_INFO, DFX_SUCCESS, CONCURRENT_ID, pasteId, GET_DATA_APP,
        appInfo.bundleName, GET_DATA_TYPE, GenerateDataType(data), LOCAL_DEV_TYPE,
        DMAdapter::GetInstance().GetLocalDeviceType(), PEER_NET_ID, PasteboardDfxUntil::GetAnonymousID(peerNetId));
    if (result != static_cast<int32_t>(PasteboardError::E_OK)) {
        return result;
    }
    int64_t fileSize = GetFileSize(data);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "fileSize=%{public}" PRId64", isremote=%{public}d", fileSize,
        static_cast<int>(data.IsRemote()));
    if (data.IsRemote() && fileSize > 0) {
        data.SetPasteId(pasteId);
        data.deviceId_ = event.second.deviceId;
        EstablishP2PLink(data.deviceId_, data.GetPasteId());
    }
    GetPasteDataDot(data, appInfo.bundleName);
    return GrantUriPermission(data, appInfo.bundleName);
}

int64_t PasteboardService::GetFileSize(PasteData &data)
{
    int64_t fileSize = 0L;
    auto value = data.GetProperty().additions.GetParam(PasteData::REMOTE_FILE_SIZE_LONG);
    AAFwk::ILong *ao = AAFwk::ILong::Query(value);
    if (ao != nullptr) {
        fileSize = AAFwk::Long::Unbox(ao);
    } else {
        fileSize = data.GetProperty().additions.GetIntParam(PasteData::REMOTE_FILE_SIZE, -1);
    }
    return fileSize;
}

PasteboardService::RemoteDataTaskManager::DataTask PasteboardService::RemoteDataTaskManager::GetRemoteDataTask(const
    Event &event)
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
    auto& task = it->second;
    task->data_ = data;
    task->getDataBlocks_.ForEach([](const auto& key, auto value) -> bool {
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
        return static_cast<int32_t>(PasteboardError::E_REMOTE_TASK);
    }

    if (isPasting) {
        auto value = taskMgr_.WaitRemoteData(event);
        if (value != nullptr && value->data != nullptr) {
            syncTime = value->syncTime;
            data = *(value->data);
            return static_cast<int32_t>(PasteboardError::E_OK);
        }
        return static_cast<int32_t>(PasteboardError::E_IS_BEGING_PROCESSED);
    }

    auto curEvent = GetValidDistributeEvent(userId);
    if (!curEvent.first || !(curEvent.second == event)) {
        auto it = clips_.Find(userId);
        if (it.first) {
            data = *it.second;
        }
        taskMgr_.ClearRemoteDataTask(event);
        int32_t res = it.first ? static_cast<int32_t>(PasteboardError::E_OK) :
            static_cast<int32_t>(PasteboardError::E_INVALID_EVENT);
        return res;
    }

    return GetRemotePasteData(userId, event, data, syncTime);
}

int32_t PasteboardService::GetRemotePasteData(int32_t userId, const Event &event, PasteData &data, int32_t &syncTime)
{
    auto block = std::make_shared<BlockObject<std::shared_ptr<PasteDateTime>>>(GET_REMOTE_DATA_WAIT_TIME);
    std::thread thread([this, event, block, userId]() mutable {
        auto result = GetDistributedData(event, userId);
        auto validEvent = GetValidDistributeEvent(userId);
        std::shared_ptr<PasteDateTime> pasteDataTime = std::make_shared<PasteDateTime>();
        if (result.first != nullptr) {
            result.first->SetRemote(true);
            if (validEvent.second == event) {
                clips_.InsertOrAssign(userId, result.first);
                auto curTime = static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch())
                        .count());
                copyTime_.InsertOrAssign(userId, curTime);
            }
            pasteDataTime->syncTime = result.second;
            pasteDataTime->data = result.first;
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
        return static_cast<int32_t>(PasteboardError::E_OK);
    }
    return static_cast<int32_t>(PasteboardError::E_GET_REMOTE_DATA);
}

int32_t PasteboardService::GetLocalData(const AppInfo &appInfo, PasteData &data)
{
    std::string pasteId = data.GetPasteId();
    auto it = clips_.Find(appInfo.userId);
    auto tempTime = copyTime_.Find(appInfo.userId);
    if (!it.first || !tempTime.first) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no data.");
        return static_cast<int32_t>(PasteboardError::E_NO_DATA);
    }
    auto ret = IsDataVaild(*(it.second), appInfo.tokenId);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "paste data is invaild.");
        return static_cast<int32_t>(PasteboardError::E_INVALID_VALUE);
    }
    data = *(it.second);
    auto originBundleName = it.second->GetBundleName();
    auto isDelayData = it.second->IsDelayData();
    if (isDelayData) {
        GetDelayPasteData(appInfo, data);
        RADAR_REPORT(DFX_GET_PASTEBOARD, DFX_CHECK_GET_DELAY_PASTE, DFX_SUCCESS, CONCURRENT_ID, pasteId);
    }
    bool isDelayRecordPadding = false;
    if (it.second->IsDelayRecord()) {
        isDelayRecordPadding = GetDelayPasteRecord(appInfo, data);
    }
    data.SetBundleName(appInfo.bundleName);
    auto result = copyTime_.Find(appInfo.userId);
    if (!result.first) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "userId not found");
        return static_cast<int32_t>(PasteboardError::E_INVALID_USERID);
    }
    auto curTime = result.second;
    if (tempTime.second == curTime) {
        bool isNotify = false;
        clips_.ComputeIfPresent(appInfo.userId, [&data, &isNotify, &isDelayRecordPadding](auto &key, auto &value) {
            if (value->IsDelayData() || (value->IsDelayRecord() && isDelayRecordPadding)) {
                value = std::make_shared<PasteData>(data);
                isNotify = true;
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

void PasteboardService::GetDelayPasteData(const AppInfo &appInfo, PasteData &data)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "get delay data start");
    delayGetters_.ComputeIfPresent(appInfo.userId, [this, &data](auto, auto &delayGetter) {
        PasteData delayData;
        if (delayGetter.first != nullptr) {
            delayGetter.first->GetPasteData("", delayData);
        }
        if (delayGetter.second != nullptr) {
            delayGetter.first->AsObject()->RemoveDeathRecipient(delayGetter.second);
        }
        delayData.SetDelayData(false);
        delayData.SetBundleName(data.GetBundleName());
        delayData.SetOrginAuthority(data.GetOrginAuthority());
        delayData.SetTime(data.GetTime());
        delayData.SetTokenId(data.GetTokenId());
        CheckAppUriPermission(delayData);
        SetWebViewPasteData(delayData, data.GetBundleName());
        data = delayData;
        return false;
    });
}

bool PasteboardService::GetDelayPasteRecord(const AppInfo &appInfo, PasteData &data)
{
    auto entryGetter = entryGetters_.Find(appInfo.userId);
    if (!entryGetter.first) {
        return false;
    }
    auto getter = entryGetter.second;
    if (getter.first == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "entry getter is nullptr, dataId is %{public}d", data.GetDataId());
        return false;
    }
    bool isPadding = false;
    for (auto record : data.AllRecords()) {
        if (!(record->IsEmpty())) {
            continue;
        }
        if (!record->IsDelayRecord()) {
            continue;
        }
        auto entries = record->GetEntries();
        if (entries.size() <= 0) {
            continue;
        }
        auto result = getter.first->GetRecordValueByType(record->GetRecordId(), *entries[0]);
        if (result != static_cast<int32_t>(PasteboardError::E_OK)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
                "get record value fail, dataId is %{public}d, recordId is %{public}d",
                data.GetDataId(), record->GetRecordId());
            continue;
        }
        record->AddEntry(entries[0]->GetUtdId(), entries[0]);
        isPadding = true;
    }
    return isPadding;
}

void PasteboardService::EstablishP2PLink(const std::string &networkId, const std::string &pasteId)
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    auto callPid = IPCSkeleton::GetCallingPid();
    p2pMap_.Compute(networkId, [pasteId, callPid] (const auto& key, auto& value) {
        value.Compute(pasteId, [callPid] (const auto& key, auto& value) {
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
    if (ret != RESULT_OK) {
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
    if (ret != RESULT_OK) {
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
        return;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "deviceId is %{public}.6s, taskId is %{public}s",
        deviceId.c_str(), pasteId.c_str());
    RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, RadarReporter::DFX_DISTRIBUTED_FILE_END, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, pasteId);
    p2pMap_.ComputeIfPresent(deviceId, [pasteId, deviceId, this] (const auto& key, auto& value) {
        value.ComputeIfPresent(pasteId, [deviceId] (const auto& key, auto& value) {
            return false;
        });
        if (value.Empty()) {
            CloseP2PLink(deviceId);
            return false;
        }
        return true;
    });
}

int32_t PasteboardService::GrantUriPermission(PasteData &data, const std::string &targetBundleName)
{
    std::vector<Uri> grantUris;
    CheckUriPermission(data, grantUris, targetBundleName);
    if (grantUris.size() == 0) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no uri.");
        return static_cast<int32_t>(PasteboardError::E_OK);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "uri size: %{public}u, targetBundleName is %{public}s",
        static_cast<uint32_t>(grantUris.size()), targetBundleName.c_str());
    size_t offset = 0;
    size_t length = grantUris.size();
    size_t count = MAX_URI_COUNT;
    bool grantSuccess = true;
    while (length > offset) {
        if (length - offset < MAX_URI_COUNT) {
            count = length - offset;
        }
        auto sendValues = std::vector<Uri>(grantUris.begin() + offset, grantUris.begin() + offset + count);
        auto permissionCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermissionPrivileged(sendValues,
            AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION, targetBundleName);
        if (permissionCode == 0) {
            std::lock_guard<std::mutex> lock(bundleMutex_);
            if (readBundles_.count(targetBundleName) == 0) {
                readBundles_.insert(targetBundleName);
            }
        }
        grantSuccess = grantSuccess && (permissionCode == 0);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "permissionCode is %{public}d", permissionCode);
        offset += count;
    }
    return grantSuccess ? static_cast<int32_t>(PasteboardError::E_OK) :
        static_cast<int32_t>(PasteboardError::E_URI_GRANT_ERROR);
}

void PasteboardService::CheckUriPermission(PasteData &data, std::vector<Uri> &grantUris,
    const std::string &targetBundleName)
{
    for (size_t i = 0; i < data.GetRecordCount(); i++) {
        auto item = data.GetRecordAt(i);
        if (item == nullptr || (!data.IsRemote() && targetBundleName.compare(data.GetOrginAuthority()) == 0)) {
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
        } else if (!item->isConvertUriFromRemote && item->GetOrginUri() != nullptr) {
            uri = item->GetOrginUri();
        }
        auto hasGrantUriPermission = item->HasGrantUriPermission();
        if (uri == nullptr || (!IsBundleOwnUriPermission(data.GetOrginAuthority(), *uri) && !hasGrantUriPermission)) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "uri is null:%{public}d, not grant permission: %{public}d.",
                uri == nullptr, hasGrantUriPermission);
            continue;
        }
        grantUris.emplace_back(*uri);
    }
}

void PasteboardService::RevokeUriPermission(std::shared_ptr<PasteData> pasteData)
{
    std::set<std::string> bundles;
    {
        std::lock_guard<std::mutex> lock(bundleMutex_);
        bundles = std::move(readBundles_);
    }
    if (pasteData == nullptr || bundles.empty()) {
        return;
    }
    std::thread thread([pasteData, bundles] () {
        auto& permissionClient = AAFwk::UriPermissionManagerClient::GetInstance();
        for (size_t i = 0; i < pasteData->GetRecordCount(); i++) {
            auto item = pasteData->GetRecordAt(i);
            if (item == nullptr || item->GetOrginUri() == nullptr) {
                continue;
            }
            Uri &uri = *(item->GetOrginUri());
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
    auto authority = uri.GetAuthority();
    if (bundleName.compare(authority) != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "grant error, uri:%{public}s, orgin:%{public}s",
            authority.c_str(), bundleName.c_str());
        return false;
    }
    return true;
}

void PasteboardService::CheckAppUriPermission(PasteData &data)
{
    std::vector<std::string> uris;
    std::vector<size_t> indexs;
    std::vector<bool> checkResults;
    for (size_t i = 0; i < data.GetRecordCount(); i++) {
        auto item = data.GetRecordAt(i);
        if (item == nullptr || item->GetOrginUri() == nullptr) {
            continue;
        }
        auto uri = item->GetOrginUri()->ToString();
        uris.emplace_back(uri);
        indexs.emplace_back(i);
    }
    if (uris.empty()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no uri.");
        return;
    }
    size_t offset = 0;
    size_t length = uris.size();
    size_t count = MAX_URI_COUNT;
    while (length > offset) {
        if (length - offset < MAX_URI_COUNT) {
            count = length - offset;
        }
        auto sendValues = std::vector<std::string>(uris.begin() + offset, uris.begin() + offset + count);
        std::vector<bool> ret = AAFwk::UriPermissionManagerClient::GetInstance().CheckUriAuthorization(sendValues,
            AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION, data.GetTokenId());
        checkResults.insert(checkResults.end(), ret.begin(), ret.end());
        offset += count;
    }
    for (size_t i = 0; i < indexs.size(); i++) {
        auto item = data.GetRecordAt(indexs[i]);
        if (item == nullptr || item->GetOrginUri() == nullptr) {
            continue;
        }
        item->SetGrantUriPermission(checkResults[i]);
    }
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
    auto it = clips_.Find(userId);
    if (!it.first) {
        ScreenEvent screenStatus = GetCurrentScreenStatus();
        if (screenStatus != ScreenEvent::ScreenUnlocked) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "screenStatus:%{public}d.", screenStatus);
            return false;
        }
        auto evt = GetValidDistributeEvent(userId);
        return evt.first;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    return IsDataVaild(*(it.second), tokenId);
}

int32_t PasteboardService::SetPasteData(PasteData &pasteData, const sptr<IPasteboardDelayGetter> delayGetter,
    const sptr<IPasteboardEntryGetter> entryGetter)
{
    auto data = std::make_shared<PasteData>(pasteData);
    return SavePasteData(data, delayGetter, entryGetter);
}

bool PasteboardService::HasDataType(const std::string &mimeType)
{
    if (GetCurrentScreenStatus() == ScreenEvent::ScreenUnlocked) {
        auto userId = GetCurrentAccountId();
        auto event = GetValidDistributeEvent(userId);
        if (event.first) {
            auto it = std::find(event.second.dataType.begin(), event.second.dataType.end(), mimeType);
            if (it != event.second.dataType.end()) {
                return true;
            }
            PasteData data;
            int32_t syncTime = 0;
            if (GetRemoteData(userId, event.second, data, syncTime) != static_cast<int32_t>(PasteboardError::E_OK)) {
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

std::pair<bool, ClipPlugin::GlobalEvent> PasteboardService::GetValidDistributeEvent(int32_t user)
{
    Event evt;
    auto plugin = GetClipPlugin();
    if (plugin == nullptr) {
        return std::make_pair(false, evt);
    }

    auto events = plugin->GetTopEvents(1, user);
    if (events.empty()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "events empty.");
        return std::make_pair(false, evt);
    }

    evt = events[0];
    if (evt.deviceId == DMAdapter::GetInstance().GetLocalNetworkId() ||
        evt.expiration < currentEvent_.expiration) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get local data.");
        return std::make_pair(false, evt);
    }
    if (evt.account != AccountManager::GetInstance().GetCurrentAccount()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "account error.");
        return std::make_pair(false, evt);
    }

    if (evt.deviceId == currentEvent_.deviceId && evt.seqId == currentEvent_.seqId &&
        evt.expiration == currentEvent_.expiration) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get same remote data.");
        return std::make_pair(false, evt);
    }

    uint64_t curTime =
        static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    if ((curTime < evt.expiration) && (evt.status == ClipPlugin::EVT_NORMAL)) {
        return std::make_pair(true, evt);
    }

    return std::make_pair(false, evt);
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
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "pasteData is invalid, tokenId is %{public}d, userId: %{public}d,"
            "mimeType: %{public}s", tokenId, userId, mimeType.c_str());
        return false;
    }
    auto screenStatus = GetCurrentScreenStatus();
    if (it.second->GetScreenStatus() > screenStatus) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "current screen is %{public}d, set data screen is %{public}d."
            "userId: %{public}d, mimeType: %{public}s",
            screenStatus, it.second->GetScreenStatus(), userId, mimeType.c_str());
        return false;
    }
    std::vector<std::string> mimeTypes = it.second->GetMimeTypes();
    auto isWebData = it.second->GetTag() == PasteData::WEBVIEW_PASTEDATA_TAG;
    auto isExistType = std::find(mimeTypes.begin(), mimeTypes.end(), mimeType) != mimeTypes.end();
    if (isWebData) {
        return mimeType == MIMETYPE_TEXT_HTML && isExistType;
    }
    return isExistType;
}

bool PasteboardService::IsRemoteData()
{
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        return false;
    }
    auto it = clips_.Find(userId);
    if (!it.first) {
        auto evt =  GetValidDistributeEvent(userId);
        return evt.first;
    }
    return it.second->IsRemote();
}

int32_t PasteboardService::GetDataSource(std::string &bundleName)
{
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        return static_cast<int32_t>(PasteboardError::E_ERROR);
    }
    auto it = clips_.Find(userId);
    if (!it.first) {
        return static_cast<int32_t>(PasteboardError::E_REMOTE);
    }
    auto data = it.second;
    if (data->IsRemote()) {
        return static_cast<int32_t>(PasteboardError::E_REMOTE);
    }
    auto tokenId = data->GetTokenId();
    bundleName = GetAppLabel(tokenId);
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::SavePasteData(std::shared_ptr<PasteData> &pasteData,
    sptr<IPasteboardDelayGetter> delayGetter, sptr<IPasteboardEntryGetter> entryGetter)
{
    PasteboardTrace tracer("PasteboardService, SetPasteData");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!IsCopyable(tokenId)) {
        RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_CHECK_SET_AUTHORITY, DFX_SUCCESS);
        return static_cast<int32_t>(PasteboardError::E_COPY_FORBIDDEN);
    }
    if (setting_.exchange(true)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "is setting.");
        return static_cast<int32_t>(PasteboardError::E_IS_BEGING_PROCESSED);
    }
    CalculateTimeConsuming::SetBeginTime();
    auto appInfo = GetAppInfo(tokenId);
    if (appInfo.userId == ERROR_USERID) {
        setting_.store(false);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return static_cast<int32_t>(PasteboardError::E_INVALID_USERID);
    }
    setPasteDataUId_ = IPCSkeleton::GetCallingUid();
    RemovePasteData(appInfo);
    pasteData->SetBundleName(appInfo.bundleName);
    pasteData->SetOrginAuthority(appInfo.bundleName);
    std::string time = GetTime();
    pasteData->SetTime(time);
    pasteData->SetScreenStatus(GetCurrentScreenStatus());
    pasteData->SetTokenId(tokenId);
    auto dataId = ++dataId_;
    pasteData->SetDataId(dataId);
    for (auto &record : pasteData->AllRecords()) {
        record->SetDataId(dataId);
    }
    UpdateShareOption(*pasteData);
    CheckAppUriPermission(*pasteData);
    SetWebViewPasteData(*pasteData, appInfo.bundleName);
    clips_.InsertOrAssign(appInfo.userId, pasteData);
    RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_CHECK_SET_DELAY_COPY, static_cast<int>(pasteData->IsDelayData()), SET_DATA_APP,
        appInfo.bundleName, LOCAL_DEV_TYPE, DMAdapter::GetInstance().GetLocalDeviceType());
    if (pasteData->IsDelayData()) {
        sptr<DelayGetterDeathRecipient> deathRecipient =
            new (std::nothrow) DelayGetterDeathRecipient(appInfo.userId, *this);
        delayGetter->AsObject()->AddDeathRecipient(deathRecipient);
        delayGetters_.InsertOrAssign(appInfo.userId, std::make_pair(delayGetter, deathRecipient));
    }
    if (pasteData->IsDelayRecord()) {
        sptr<EntryGetterDeathRecipient> deathRecipient =
            new (std::nothrow) EntryGetterDeathRecipient(appInfo.userId, *this);
        entryGetter->AsObject()->AddDeathRecipient(deathRecipient);
        entryGetters_.InsertOrAssign(appInfo.userId, std::make_pair(entryGetter, deathRecipient));
    }
    auto curTime = static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "curTime = %{public}" PRIu64, curTime);
    copyTime_.InsertOrAssign(appInfo.userId, curTime);
    if (!(pasteData->IsDelayData())) {
        SetDistributedData(appInfo.userId, *pasteData);
        NotifyObservers(appInfo.bundleName, PasteboardEventStatus::PASTEBOARD_WRITE);
    }
    SetPasteDataDot(*pasteData);
    setting_.store(false);
    SubscribeKeyboardEvent();
    return static_cast<int32_t>(PasteboardError::E_OK);
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

void PasteboardService::SetWebViewPasteData(PasteData &pasteData, const std::string &bundleName)
{
    if (pasteData.GetTag() != PasteData::WEBVIEW_PASTEDATA_TAG) {
        return;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardService for webview.");
    for (auto& item : pasteData.AllRecords()) {
        if (item->GetUri() == nullptr) {
            continue;
        }
        std::shared_ptr<Uri> uri = item->GetUri();
        std::string puri = uri->ToString();
        if (puri.substr(0, PasteData::IMG_LOCAL_URI.size()) == PasteData::IMG_LOCAL_URI &&
            puri.find(PasteData::FILE_SCHEME_PREFIX + PasteData::PATH_SHARE) == std::string::npos) {
            std::string path = uri->GetPath();
            std::string newUriStr = PasteData::FILE_SCHEME_PREFIX + bundleName + path;
            item->SetUri(std::make_shared<OHOS::Uri>(newUriStr));
        }
    }
}

int32_t PasteboardService::GetCurrentAccountId()
{
    if (currentUserId != ERROR_USERID) {
        return currentUserId;
    }
    std::vector<int32_t> accountIds;
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(accountIds);
    if (ret != ERR_OK || accountIds.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "query active user failed errCode=%{public}d", ret);
        return ERROR_USERID;
    }
    currentUserId = accountIds.front();
    return currentUserId;
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
    if (ret != 0 || !copyable) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "tokenId = 0x%{public}x ret = %{public}d, copyable = %{public}d.",
            tokenId, ret, copyable);
        return false;
    }
#endif
    return true;
}

void PasteboardService::SubscribeObserver(PasteboardObserverType type,
    const sptr<IPasteboardChangedObserver> &observer)
{
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

void PasteboardService::UnsubscribeObserver(PasteboardObserverType type,
    const sptr<IPasteboardChangedObserver> &observer)
{
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

void PasteboardService::AddObserver(int32_t userId, const sptr<IPasteboardChangedObserver> &observer,
    ObserverMap &observerMap)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "observer null.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap.find(userId);
    std::shared_ptr<std::set<sptr<IPasteboardChangedObserver>, classcomp>> observers;
    if (it != observerMap.end()) {
        observers = it->second;
    } else {
        observers = std::make_shared<std::set<sptr<IPasteboardChangedObserver>, classcomp>>();
        observerMap.insert(std::make_pair(userId, observers));
    }
    observers->insert(observer);
    RADAR_REPORT(DFX_OBSERVER, DFX_ADD_OBSERVER, DFX_SUCCESS);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "observers->size = %{public}u.",
        static_cast<unsigned int>(observers->size()));
}

void PasteboardService::RemoveSingleObserver(int32_t userId, const sptr<IPasteboardChangedObserver> &observer,
    ObserverMap &observerMap)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "observer null.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap.find(userId);
    if (it == observerMap.end()) {
        return;
    }
    auto observers = it->second;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers size: %{public}u.",
        static_cast<unsigned int>(observers->size()));
    auto eraseNum = observers->erase(observer);
    RADAR_REPORT(DFX_OBSERVER, DFX_REMOVE_SINGLE_OBSERVER, DFX_SUCCESS);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers size = %{public}u, eraseNum = %{public}zu",
        static_cast<unsigned int>(observers->size()), eraseNum);
}

void PasteboardService::RemoveAllObserver(int32_t userId, ObserverMap &observerMap)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap.find(userId);
    if (it == observerMap.end()) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "observer empty.");
        return;
    }
    auto observers = it->second;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers size: %{public}u.",
        static_cast<unsigned int>(observers->size()));
    auto eraseNum = observerMap.erase(userId);
    RADAR_REPORT(DFX_OBSERVER, DFX_REMOVE_ALL_OBSERVER, DFX_SUCCESS);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers size = %{public}u, eraseNum = %{public}zu",
        static_cast<unsigned int>(observers->size()), eraseNum);
}

int32_t PasteboardService::SetGlobalShareOption(const std::map<uint32_t, ShareOption> &globalShareOptions)
{
    if (!IsCallerUidValid()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "No Permission");
        return static_cast<int32_t>(PasteboardError::E_NO_PERMISSION);
    }
    for (const auto &[tokenId,  shareOption] : globalShareOptions) {
        globalShareOptions_.InsertOrAssign(tokenId, shareOption);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Set %{public}zu global shareOption.", globalShareOptions.size());
    return ERR_OK;
}

int32_t PasteboardService::RemoveGlobalShareOption(const std::vector<uint32_t> &tokenIds)
{
    if (!IsCallerUidValid()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "No Permission");
        return static_cast<int32_t>(PasteboardError::E_NO_PERMISSION);
    }
    int32_t count = 0;
    for (const uint32_t &tokenId : tokenIds) {
        globalShareOptions_.ComputeIfPresent(tokenId, [&count](const uint32_t &key, ShareOption &value) {
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
        globalShareOptions_.ForEach([&result](const uint32_t &key, ShareOption &value) {
            result[key] = value;
            return false;
        });
        return result;
    }
    for (const uint32_t &tokenId : tokenIds) {
        globalShareOptions_.ComputeIfPresent(tokenId, [&result](const uint32_t &key, ShareOption &value) {
            result[key] = value;
            return true;
        });
    }
    return result;
}

int32_t PasteboardService::SetAppShareOptions(const ShareOption &shareOptions)
{
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "No permission, full token id: 0x%{public}" PRIx64 "", fullTokenId);
        return static_cast<int32_t>(PasteboardError::E_NO_PERMISSION);
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto isAbsent = globalShareOptions_.ComputeIfAbsent(tokenId, [&shareOptions](const uint32_t &tokenId) {
        return shareOptions;
    });
    if (!isAbsent) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Settings already exist, token id: 0x%{public}x.", tokenId);
        return static_cast<int32_t>(PasteboardError::E_INVALID_OPERATION);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE,
        "Set token id: 0x%{public}x share options: %{public}d success.", tokenId, shareOptions);
    return 0;
}

int32_t PasteboardService::RemoveAppShareOptions()
{
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!OHOS::Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "No permission, full token id: 0x%{public}" PRIx64 "", fullTokenId);
        return static_cast<int32_t>(PasteboardError::E_NO_PERMISSION);
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    globalShareOptions_.Erase(tokenId);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Remove token id: 0x%{public}x share options success.", tokenId);
    return 0;
}

void PasteboardService::UpdateShareOption(PasteData &pasteData)
{
    globalShareOptions_.ComputeIfPresent(pasteData.GetTokenId(),
        [&pasteData](const uint32_t &tokenId, ShareOption &shareOption) {
            pasteData.SetShareOption(shareOption);
            return true;
        });
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

void PasteboardService::NotifyObservers(std::string bundleName, PasteboardEventStatus status)
{
    std::thread thread([this, bundleName, status] () {
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
    return GET_WRONG_SIZE;
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

std::pair<std::shared_ptr<PasteData>, int32_t> PasteboardService::GetDistributedData(const Event &event, int32_t user)
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return std::make_pair(nullptr, -1);
    }
    std::vector<uint8_t> rawData;
    auto result = clipPlugin->GetPasteData(event, rawData);
    if (result.first != 0) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "get data failed");
        Reporter::GetInstance().PasteboardFault().Report({ user, "GET_REMOTE_DATA_FAILED" });
        return std::make_pair(nullptr, -1);
    }

    currentEvent_ = std::move(event);
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->Decode(rawData);
    pasteData->ReplaceShareUri(user);
    pasteData->SetOrginAuthority(pasteData->GetBundleName());
    for (size_t i = 0; i < pasteData->GetRecordCount(); i++) {
        auto item = pasteData->GetRecordAt(i);
        if (item == nullptr || item->GetConvertUri().empty()) {
            continue;
        }
        item->isConvertUriFromRemote = true;
    }
    return std::make_pair(pasteData, result.second);
}

bool PasteboardService::IsAllowSendData()
{
    auto contralType = system::GetIntParameter(TRANSMIT_CONTROL_PROP_KEY, CONTROL_TYPE_ALLOW_SEND_RECEIVE, INT_MIN,
        INT_MAX);
    if (contralType != CONTROL_TYPE_ALLOW_SEND_RECEIVE) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "control type is: %{public}d.", contralType);
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

bool PasteboardService::SetDistributedData(int32_t user, PasteData &data)
{
    if (!IsAllowSendData()) {
        return false;
    }
    std::shared_ptr<std::vector<uint8_t>> rawData = std::make_shared<std::vector<uint8_t>>();
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_CHECK_ONLINE_DEVICE, DFX_SUCCESS);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return false;
    }
    RADAR_REPORT(DFX_SET_PASTEBOARD, DFX_LOAD_DISTRIBUTED_PLUGIN, DFX_SUCCESS);
    GenerateDistributedUri(data);
    if (data.GetShareOption() == InApp || !data.Encode(*rawData)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "InApp data is not supports cross device, or data encode failed.");
        return false;
    }

    auto networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    if (networkId.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "networkId is empty.");
        return false;
    }

    auto expiration =
        duration_cast<milliseconds>((system_clock::now() + minutes(EXPIRATION_INTERVAL)).time_since_epoch()).count();
    Event event;
    event.user = user;
    event.seqId = ++sequenceId_;
    event.expiration = static_cast<uint64_t>(expiration);
    event.deviceId = networkId;
    event.account = AccountManager::GetInstance().GetCurrentAccount();
    event.status = ClipPlugin::EVT_NORMAL;
    event.dataType = data.GetMimeTypes();
    currentEvent_ = event;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "expiration = %{public}" PRIu64, event.expiration);
    std::thread thread([clipPlugin, event, rawData]() mutable {
        clipPlugin->SetPasteData(event, *rawData);
    });
    thread.detach();
    return true;
}

void PasteboardService::GenerateDistributedUri(PasteData &data)
{
    std::vector<std::string> uris;
    std::vector<size_t> indexs;
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return;
    }
    for (size_t i = 0; i < data.GetRecordCount(); i++) {
        auto item = data.GetRecordAt(i);
        if (item == nullptr || item->GetOrginUri() == nullptr) {
            continue;
        }
        Uri uri = *(item->GetOrginUri());
        if (!IsBundleOwnUriPermission(data.GetOrginAuthority(), uri) && !item->HasGrantUriPermission()) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "orginuri=%{public}s, no permission", uri.ToString().c_str());
            continue;
        }
        uris.emplace_back(uri.ToString());
        indexs.emplace_back(i);
    }
    size_t fileSize = 0;
    std::unordered_map<std::string, HmdfsUriInfo> dfsUris;
    if (!uris.empty()) {
        int ret = RemoteFileShare::GetDfsUrisFromLocal(uris, userId, dfsUris);
        if (ret != 0 || dfsUris.empty()) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Get remoteUri failed, ret = %{public}d, userId: %{public}d,"
                "uri size:%{public}zu.", ret, userId, uris.size());
            return;
        }
        for (size_t i = 0; i < indexs.size(); i++) {
            auto item = data.GetRecordAt(indexs[i]);
            if (item == nullptr || item->GetOrginUri() == nullptr) {
                continue;
            }
            auto it = dfsUris.find(item->GetOrginUri()->ToString());
            if (it != dfsUris.end()) {
                item->SetConvertUri(it->second.uriStr);
                fileSize += it->second.fileSize;
            }
        }
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "file size: %{public}zu", fileSize);
    int32_t fileIntSize = (fileSize > INT_MAX) ? INT_MAX : static_cast<int32_t>(fileSize);
    data.SetAddition(PasteData::REMOTE_FILE_SIZE, AAFwk::Integer::Box(fileIntSize));
    data.SetAddition(PasteData::REMOTE_FILE_SIZE_LONG, AAFwk::Long::Box(fileSize));
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
    auto release = [this](ClipPlugin *plugin) {
        std::lock_guard<decltype(mutex)> lockGuard(mutex);
        ClipPlugin::DestroyPlugin(PLUGIN_NAME, plugin);
    };

    clipPlugin_ = std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME), release);
    return clipPlugin_;
}

bool PasteboardService::CleanDistributedData(int32_t user)
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return true;
    }
    clipPlugin->Clear(user);
    return true;
}

void PasteboardService::OnConfigChange(bool isOn)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "ConfigChange isOn: %{public}d.", isOn);
    p2pMap_.Clear();
    std::lock_guard<decltype(mutex)> lockGuard(mutex);
    if (!isOn) {
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

void PasteBoardCommonEventSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    auto want = data.GetWant();
    std::string action = want.GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED) {
        std::lock_guard<std::mutex> lock(mutex_);
        int32_t userId = data.GetCode();
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "user id switched: %{public}d", userId);
        PasteboardService::currentUserId = userId;
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

bool PasteboardService::SubscribeKeyboardEvent()
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    if (inputEventCallback_ != nullptr) {
        return true;
    }
    inputEventCallback_ = std::make_shared<InputEventCallback>();
    int32_t monitorId =
        MMI::InputManager::GetInstance()->AddMonitor(std::static_pointer_cast<MMI::IInputEventConsumer>(
        inputEventCallback_));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "add monitor ret is: %{public}d", monitorId);
    return monitorId >= 0;
}

void PasteboardService::PasteboardEventSubscriber()
{
    EventCenter::GetInstance().Subscribe(PasteboardEvent::DISCONNECT,
        [this](const OHOS::MiscServices::Event& event) {
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
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    commonEventSubscriber_ = std::make_shared<PasteBoardCommonEventSubscriber>(subscribeInfo);
    EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
}

int32_t PasteboardService::AppExit(pid_t uid, pid_t pid, uint32_t token)
{
    std::vector<std::string> networkIds;
    p2pMap_.EraseIf([pid, &networkIds, this](auto &networkId, auto &pidMap) {
        pidMap.EraseIf([pid, this](auto &key, auto &value) {
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
    for (const auto& id: networkIds) {
        CloseP2PLink(id);
    }
    return ERR_OK;
}

PasteboardService::PasteboardClientDeathObserverImpl::PasteboardClientDeathObserverImpl(PasteboardService &service,
    sptr<IRemoteObject> observer) : dataService_(service), observerProxy_(std::move(observer)),
    deathRecipient_(new PasteboardDeathRecipient(*this))
{
    uid_ = IPCSkeleton::GetCallingUid();
    pid_ = IPCSkeleton::GetCallingPid();
    token_ = IPCSkeleton::GetCallingTokenID();
    if (observerProxy_ != nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "add death recipient");
        observerProxy_->AddDeathRecipient(deathRecipient_);
    } else {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "observerProxy_ is nullptr");
    }
}

PasteboardService::PasteboardClientDeathObserverImpl::PasteboardClientDeathObserverImpl(PasteboardService &service)
    : dataService_(service)
{
    Reset();
}

PasteboardService::PasteboardClientDeathObserverImpl::PasteboardClientDeathObserverImpl(
    PasteboardClientDeathObserverImpl &&impl)
    : uid_(impl.uid_), pid_(impl.pid_), token_(impl.token_), dataService_(impl.dataService_)
{
    impl.Reset();
}

PasteboardService::PasteboardClientDeathObserverImpl &PasteboardService::PasteboardClientDeathObserverImpl::operator=(
    PasteboardService::PasteboardClientDeathObserverImpl &&impl)
{
    uid_ = impl.uid_;
    pid_ = impl.pid_;
    token_ = impl.token_;
    impl.Reset();
    return *this;
}

pid_t PasteboardService::PasteboardClientDeathObserverImpl::GetPid() const
{
    return pid_;
}

void PasteboardService::PasteboardClientDeathObserverImpl::Reset()
{
    uid_ = INVALID_UID;
    pid_ = INVALID_PID;
    token_ = INVALID_TOKEN;
}

PasteboardService::PasteboardClientDeathObserverImpl::~PasteboardClientDeathObserverImpl()
{
    if (deathRecipient_ != nullptr && observerProxy_ != nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "remove death recipient");
        observerProxy_->RemoveDeathRecipient(deathRecipient_);
    }
}

PasteboardService::PasteboardClientDeathObserverImpl::PasteboardDeathRecipient::PasteboardDeathRecipient(
    PasteboardClientDeathObserverImpl &pasteboardClientDeathObserverImpl)
    : pasteboardClientDeathObserverImpl_(pasteboardClientDeathObserverImpl)
{
}

void PasteboardService::PasteboardClientDeathObserverImpl::PasteboardDeathRecipient::OnRemoteDied(
    const wptr<IRemoteObject> &remote)
{
    (void) remote;
    auto uid = pasteboardClientDeathObserverImpl_.uid_;
    auto pid = pasteboardClientDeathObserverImpl_.pid_;
    auto token = pasteboardClientDeathObserverImpl_.token_;
    pasteboardClientDeathObserverImpl_.dataService_.AppExit(uid, pid, token);
}

PasteboardService::PasteboardClientDeathObserverImpl::PasteboardDeathRecipient::~PasteboardDeathRecipient()
{
}

int32_t PasteboardService::RegisterClientDeathObserver(sptr<IRemoteObject> observer)
{
    clients_.Emplace(std::piecewise_construct, std::forward_as_tuple(IPCSkeleton::GetCallingPid()),
        std::forward_as_tuple(*this, observer));
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
    if (((keyItems[0].GetKeyCode() == MMI::KeyEvent::KEYCODE_CTRL_LEFT) ||
        (keyItems[0].GetKeyCode() == MMI::KeyEvent::KEYCODE_CTRL_RIGHT)) &&
        keyItems[1].GetKeyCode() == MMI::KeyEvent::KEYCODE_V) {
        int32_t windowId = keyEvent->GetTargetWindowId();
        std::unique_lock<std::shared_mutex> lock(inputEventMutex_);
        windowPid_ = MMI::InputManager::GetInstance()->GetWindowPid(windowId);
        actionTime_ = static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch())
            .count());
    }
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
}

void InputEventCallback::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}

bool InputEventCallback::IsCtrlVProcess(uint32_t callingPid,  bool isFocused)
{
    std::shared_lock<std::shared_mutex> lock(inputEventMutex_);
    auto curTime = static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    return (callingPid == static_cast<uint32_t>(windowPid_) || isFocused) && curTime - actionTime_ < EVENT_TIME_OUT;
}

void InputEventCallback::Clear()
{
    std::unique_lock<std::shared_mutex> lock(inputEventMutex_);
    actionTime_ = 0;
    windowPid_ = 0;
}
} // namespace MiscServices
} // namespace OHOS