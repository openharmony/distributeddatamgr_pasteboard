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
#include "common/block_object.h"
#include "dev_profile.h"
#include "device/dm_adapter.h"
#include "dfx_code_constant.h"
#include "dfx_types.h"
#include "distributed_file_daemon_manager.h"
#include "hiview_adapter.h"
#include "iservice_registry.h"
#include "loader.h"
#include "int_wrapper.h"
#include "native_token_info.h"
#include "os_account_manager.h"
#include "parameters.h"
#include "para_handle.h"
#include "pasteboard_dialog.h"
#include "pasteboard_error.h"
#include "pasteboard_trace.h"
#include "remote_file_share.h"
#include "reporter.h"
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

const std::int32_t CALL_UID = 5557;
const std::int32_t INVAILD_VERSION = -1;
const std::int32_t ADD_PERMISSION_CHECK_SDK_VERSION = 12;
const std::int32_t CTRLV_EVENT_SIZE = 2;
const std::int32_t CONTROL_TYPE_ALLOW_SEND_RECEIVE = 1;
const std::int32_t DEVICE_TYPE_PC = 12;
const std::int32_t DEVICE_TYPE_2IN1 = 2607;

const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(new PasteboardService());
} // namespace
using namespace Security::AccessToken;
using namespace OHOS::AppFileService::ModuleRemoteFileShare;
std::mutex PasteboardService::historyMutex_;
std::vector<std::string> PasteboardService::dataHistory_;
std::shared_ptr<Command> PasteboardService::copyHistory;
std::shared_ptr<Command> PasteboardService::copyData;
int32_t PasteboardService::currentUserId = ERROR_USERID;

PasteboardService::PasteboardService()
    : SystemAbility(PASTEBOARD_SERVICE_ID, true), state_(ServiceRunningState::STATE_NOT_START)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PasteboardService Start.");
    ServiceListenerFunc_[static_cast<int32_t>(DISTRIBUTED_DEVICE_PROFILE_SA_ID)] = &PasteboardService::DevProfileInit;
    ServiceListenerFunc_[static_cast<int32_t>(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID)] =
        &PasteboardService::DMAdapterInit;
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
    return ERR_OK;
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
    DMAdapter::GetInstance().Initialize(appInfo.bundleName);
    moduleConfig_.Init();
    moduleConfig_.Watch(std::bind(&PasteboardService::OnConfigChange, this, std::placeholders::_1));
    AddSysAbilityListener();

    if (Init() != ERR_OK) {
        auto callback = [this]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Init failed. Try again 10s later.");
        return;
    }

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

    if (commonEventSubscriber_ == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        commonEventSubscriber_ = std::make_shared<PasteBoardCommonEventSubscriber>(subscribeInfo);
        EventFwk::CommonEventManager::SubscribeCommonEvent(commonEventSubscriber_);
    }
    if (!SubscribeKeyboardEvent()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Subscribe failed.");
        return;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Start PasteboardService success.");
    HiViewAdapter::StartTimerThread();
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

    ParaHandle::GetInstance().WatchEnabledStatus(nullptr);
    if (commonEventSubscriber_ != nullptr) {
        EventFwk::CommonEventManager::UnSubscribeCommonEvent(commonEventSubscriber_);
    }
    moduleConfig_.DeInit();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OnStop End.");
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

void PasteboardService::DMAdapterInit()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin.");
    auto appInfo = GetAppInfo(IPCSkeleton::GetCallingTokenID());
    DMAdapter::GetInstance().Initialize(appInfo.bundleName);
}

void PasteboardService::DevProfileInit()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin.");
    ParaHandle::GetInstance().Init();
    DevProfile::GetInstance().Init();
}

void PasteboardService::InitServiceHandler()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "InitServiceHandler started.");
    if (serviceHandler_ != nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Already init.");
        return;
    }
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(PASTEBOARD_SERVICE_NAME);
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
    auto deviceType = DMAdapter::GetInstance().GetLocalDeviceType();
    if (deviceType == DEVICE_TYPE_PC || deviceType == DEVICE_TYPE_2IN1) {
        return true;
    }
    auto isReadGrant = IsPermissionGranted(READ_PASTEBOARD_PERMISSION, tokenId);
    auto isSecureGrant = IsPermissionGranted(SECURE_PASTE_PERMISSION, tokenId);
    AddPermissionRecord(tokenId, isReadGrant, isSecureGrant);
    auto isPrivilegeApp = IsDefaultIME(GetAppInfo(tokenId));
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE,
        "isReadGrant is %{public}d, isSecureGrant is %{public}d, isPrivilegeApp is %{public}d", isReadGrant,
        isSecureGrant, isPrivilegeApp);
    auto isGrant = isReadGrant || isSecureGrant || isPrivilegeApp;
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
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetHapTokenInfo version=%{public}d.", hapTokenInfo.apiVersion);
    return hapTokenInfo.apiVersion;
}

bool PasteboardService::IsPermissionGranted(const std::string& perm, uint32_t tokenId)
{
    ATokenTypeEnum type = AccessTokenKit::GetTokenTypeFlag(tokenId);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "check grant permission, perm=%{public}s type=%{public}d",
        perm.c_str(), type);
    int32_t result = PermissionState::PERMISSION_DENIED;
    switch (type) {
        case ATokenTypeEnum::TOKEN_HAP:
            result = AccessTokenKit::VerifyAccessToken(tokenId, perm);
            break;
        case ATokenTypeEnum::TOKEN_NATIVE:
        case ATokenTypeEnum::TOKEN_SHELL:
            result = PermissionState::PERMISSION_GRANTED;
            break;
        case ATokenTypeEnum::TOKEN_INVALID:
        case ATokenTypeEnum::TOKEN_TYPE_BUTT:
            break;
        default:
            break;
    }
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

int32_t PasteboardService::GetPasteData(PasteData &data)
{
    PasteboardTrace tracer("PasteboardService GetPasteData");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto callPid = IPCSkeleton::GetCallingPid();
    if (!VerifyPermission(tokenId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "check permission failed, callingPid is %{public}d", callPid);
        return static_cast<int32_t>(PasteboardError::E_NO_PERMISSION);
    }
    auto ret = GetData(tokenId, data);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "data is invalid, callPid is %{public}d, tokenId is %{public}d",
            callPid, tokenId);
    }
    return ret;
}

void PasteboardService::AddPermissionRecord(uint32_t tokenId, bool isReadGrant, bool isSecureGrant)
{
    std::string permissionName = READ_PASTEBOARD_PERMISSION;
    bool isGrant = isReadGrant;
    if (isSecureGrant) {
        permissionName = SECURE_PASTE_PERMISSION;
        isGrant = isSecureGrant;
    }
    auto permUsedType = AccessTokenKit::GetUserGrantedPermissionUsedType(tokenId, permissionName);
    AddPermParamInfo info;
    info.tokenId = tokenId;
    info.permissionName = permissionName;
    info.successCount = isGrant ? 1 : 0;
    info.failCount = isGrant ? 0 : 1;
    info.type = static_cast<PermissionUsedType>(permUsedType);
    int32_t result = PrivacyKit::AddPermissionUsedRecord(info);
    if (result != RET_SUCCESS) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "add record failed, result is %{public}d", result);
    }
    return;
}

int32_t PasteboardService::GetData(uint32_t tokenId, PasteData &data)
{
    CalculateTimeConsuming::SetBeginTime();
    auto appInfo = GetAppInfo(tokenId);

    bool result = false;
    auto event = GetValidDistributeEvent(appInfo.userId);
    if (!event.first) {
        result = GetLocalData(appInfo, data);
    } else {
        result = GetRemoteData(appInfo.userId, event.second, data);
    }
    if (observerEventMap_.size() != 0) {
        std::string targetBundleName = GetAppBundleName(appInfo);
        NotifyObservers(targetBundleName, PasteboardEventStatus::PASTEBOARD_READ);
    }
    GetPasteDataDot(data, appInfo.bundleName);
    GrantUriPermission(data, appInfo.bundleName);
    return result ? static_cast<int32_t>(PasteboardError::E_OK) : static_cast<int32_t>(PasteboardError::E_ERROR);
}

bool PasteboardService::GetRemoteData(int32_t userId, const Event &event, PasteData &data)
{
    std::lock_guard<std::mutex> lock(remoteMutex_);
    auto block = std::make_shared<BlockObject<std::shared_ptr<PasteData>>>(PasteBoardDialog::POPUP_INTERVAL);
    std::thread thread([this, event, block, userId]() mutable {
        std::shared_ptr<PasteData> pasteData = GetDistributedData(event, userId);
        if (pasteData != nullptr) {
            pasteData->SetRemote(true);
            clips_.InsertOrAssign(userId, pasteData);
            auto curTime = static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch())
                    .count());
            copyTime_.InsertOrAssign(userId, curTime);
        }
        block->SetValue(pasteData);
    });
    thread.detach();
    auto value = block->GetValue();
    if (value == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "continue to get remote data");
        block->SetInterval(PasteBoardDialog::MAX_LIFE_TIME);
        value = block->GetValue();
    }
    if (value != nullptr) {
        auto ret = value->IsValid();
        data = std::move(*value);
        return ret;
    }
    return false;
}

bool PasteboardService::GetLocalData(const AppInfo &appInfo, PasteData &data)
{
    auto it = clips_.Find(appInfo.userId);
    auto tempTime = copyTime_.Find(appInfo.userId);
    if (!it.first || !tempTime.first) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no data.");
        return false;
    }
    auto ret = IsDataVaild(*(it.second), appInfo.tokenId);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "paste data is invaild.");
        return false;
    }
    data = *(it.second);
    auto originBundleName = it.second->GetBundleName();
    auto isDelayData = it.second->IsDelayData();
    if (isDelayData) {
        GetDelayPasteData(appInfo, data);
    }
    data.SetBundleName(appInfo.bundleName);
    auto curTime = copyTime_[appInfo.userId];
    if (tempTime.second == curTime && clips_[appInfo.userId]->IsDelayData()) {
        clips_[appInfo.userId] = std::make_shared<PasteData>(data);
        NotifyObservers(originBundleName, PasteboardEventStatus::PASTEBOARD_WRITE);
    }
    
    auto fileSize = data.GetProperty().additions.GetIntParam(PasteData::REMOTE_FILE_SIZE, -1);
    if (data.IsRemote() && fileSize > 0) {
        EstablishP2PLink();
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetPasteData success.");
    SetLocalPasteFlag(data.IsRemote(), appInfo.tokenId, data);
    return true;
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

void PasteboardService::EstablishP2PLink()
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "EstablishP2PLink");
    DmDeviceInfo remoteDevice;
    auto ret = DMAdapter::GetInstance().GetRemoteDeviceInfo(currentEvent_.deviceId, remoteDevice);
    if (ret != RESULT_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "remote device is not exist");
        return;
    }
    DistributedFileDaemonManager::GetInstance().OpenP2PConnection(remoteDevice);
    auto it = p2pMap_.find(currentEvent_.deviceId);
    if (it != p2pMap_.end() && it->second == 1) {
        return;
    }
    p2pMap_.insert_or_assign(currentEvent_.deviceId, 1);
    std::thread thread([this, remoteDevice]() mutable {
        std::this_thread::sleep_for(std::chrono::seconds(MIN_TRANMISSION_TIME));
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "CloseP2PLink");
        DistributedFileDaemonManager::GetInstance().CloseP2PConnection(remoteDevice);
        p2pMap_.erase(currentEvent_.deviceId);
    });
    thread.detach();
#endif
}

void PasteboardService::GrantUriPermission(PasteData &data, const std::string &targetBundleName)
{
    std::vector<Uri> grantUris;
    CheckUriPermission(data, grantUris, targetBundleName);
    if (grantUris.size() == 0) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no uri.");
        return;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "uri size: %{public}u, targetBundleName is %{public}s",
        static_cast<uint32_t>(grantUris.size()), targetBundleName.c_str());
    size_t offset = 0;
    size_t length = grantUris.size();
    size_t count = MAX_URI_COUNT;
    while (length > offset) {
        if (length - offset < MAX_URI_COUNT) {
            count = length - offset;
        }
        auto sendValues = std::vector<Uri>(grantUris.begin() + offset, grantUris.begin() + offset + count);
        auto permissionCode = AAFwk::UriPermissionManagerClient::GetInstance().GrantUriPermissionPrivileged(sendValues,
            AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION, targetBundleName);
        if (permissionCode == 0 && readBundles_.count(targetBundleName) == 0) {
            readBundles_.insert(targetBundleName);
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "permissionCode is %{public}d", permissionCode);
        offset += count;
    }
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
        if (uri == nullptr || (!isBundleOwnUriPermission(data.GetOrginAuthority(), *uri) && !hasGrantUriPermission)) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "uri is null:%{public}d, not grant permission: %{public}d.",
                uri == nullptr, hasGrantUriPermission);
            continue;
        }
        grantUris.emplace_back(*uri);
    }
}

void PasteboardService::RevokeUriPermission(std::shared_ptr<PasteData> pasteData)
{
    if (readBundles_.size() == 0 || pasteData == nullptr) {
        return;
    }
    decltype(readBundles_) bundles(std::move(readBundles_));
    readBundles_.clear();
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

bool PasteboardService::isBundleOwnUriPermission(const std::string &bundleName, Uri &uri)
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
        auto evt = GetValidDistributeEvent(userId);
        return evt.first;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    return IsDataVaild(*(it.second), tokenId);
}

int32_t PasteboardService::SetPasteData(PasteData &pasteData, const sptr<IPasteboardDelayGetter> delayGetter)
{
    auto data = std::make_shared<PasteData>(pasteData);
    return SavePasteData(data);
}

bool PasteboardService::HasDataType(const std::string &mimeType)
{
    auto userId = GetCurrentAccountId();
    auto event = GetValidDistributeEvent(userId);
    if (event.first) {
        PasteData data;
        if (!GetRemoteData(userId, event.second, data)) {
            return false;
        }
    }
    return HasLocalDataType(mimeType);
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

    auto &tmpEvent = events[0];
    if (tmpEvent.deviceId == DMAdapter::GetInstance().GetLocalNetworkId() ||
        tmpEvent.expiration < currentEvent_.expiration) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get local data.");
        return std::make_pair(false, evt);
    }
    if (tmpEvent.account != AccountManager::GetInstance().GetCurrentAccount()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "account error.");
        return std::make_pair(false, evt);
    }

    if (tmpEvent.deviceId == currentEvent_.deviceId && tmpEvent.seqId == currentEvent_.seqId &&
        tmpEvent.expiration == currentEvent_.expiration) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get same remote data.");
        return std::make_pair(false, evt);
    }

    uint64_t curTime =
        static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    if ((curTime < tmpEvent.expiration) && (tmpEvent.status == ClipPlugin::EVT_NORMAL)) {
        evt = tmpEvent;
        return std::make_pair(true, evt);
    }

    return std::make_pair(false, evt);
}

bool PasteboardService::HasLocalDataType(const std::string &mimeType)
{
    auto userId = GetCurrentAccountId();
    auto it = clips_.Find(userId);
    if (!it.first) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "can not find data");
        return false;
    }
    if (it.second == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "data is nullptr");
        return false;
    }
    std::vector<std::string> mimeTypes = it.second->GetMimeTypes();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "type is %{public}s", mimeType.c_str());
    auto isWebData = it.second->GetTag() == PasteData::WEBVIEW_PASTEDATA_TAG;
    auto isExistType = std::find(mimeTypes.begin(), mimeTypes.end(), mimeType) != mimeTypes.end();
    if (isWebData) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "is Web Data");
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
    sptr<IPasteboardDelayGetter> delayGetter)
{
    PasteboardTrace tracer("PasteboardService, SetPasteData");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!IsCopyable(tokenId)) {
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
        return static_cast<int32_t>(PasteboardError::E_ERROR);
    }
    RemovePasteData(appInfo);
    pasteData->SetBundleName(appInfo.bundleName);
    pasteData->SetOrginAuthority(appInfo.bundleName);
    std::string time = GetTime();
    pasteData->SetTime(time);
    pasteData->SetTokenId(tokenId);
    UpdateShareOption(*pasteData);
    CheckAppUriPermission(*pasteData);
    SetWebViewPasteData(*pasteData, appInfo.bundleName);
    clips_.InsertOrAssign(appInfo.userId, pasteData);
    if (pasteData->IsDelayData()) {
        auto deathRecipient = new DelayGetterDeathRecipient(appInfo.userId, *this);
        delayGetter->AsObject()->AddDeathRecipient(deathRecipient);
        delayGetters_.InsertOrAssign(appInfo.userId,
            std::make_pair(delayGetter, deathRecipient));
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
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Clips length %{public}d.", static_cast<uint32_t>(clips_.Size()));
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardService::RemovePasteData(const AppInfo &appInfo)
{
    auto it = clips_.Find(appInfo.userId);
    auto getter = delayGetters_.Find(appInfo.userId);
    if (it.first) {
        RevokeUriPermission(it.second);
        clips_.Erase(appInfo.userId);
    }
    if (getter.first) {
        if (getter.second.first != nullptr && getter.second.second != nullptr) {
            getter.second.first->AsObject()->RemoveDeathRecipient(getter.second.second);
        }
        delayGetters_.Erase(appInfo.userId);
    }
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

void PasteboardService::AddPasteboardChangedObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    AddObserver(observer, observerChangedMap_);
}

void PasteboardService::RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    RemoveSingleObserver(observer, observerChangedMap_);
}

void PasteboardService::RemoveAllChangedObserver()
{
    RemoveAllObserver(observerChangedMap_);
}

void PasteboardService::AddPasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    if (!IsCallerUidValid()) {
        return;
    }
    AddObserver(observer, observerEventMap_);
}

void PasteboardService::RemovePasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    if (!IsCallerUidValid()) {
        return;
    }
    RemoveSingleObserver(observer, observerEventMap_);
}

void PasteboardService::RemoveAllEventObserver()
{
    if (!IsCallerUidValid()) {
        return;
    }
    RemoveAllObserver(observerEventMap_);
}

void PasteboardService::AddObserver(const sptr<IPasteboardChangedObserver> &observer, ObserverMap &observerMap)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "observer null.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap.find(COMMON_USERID);
    std::shared_ptr<std::set<sptr<IPasteboardChangedObserver>, classcomp>> observers;
    if (it != observerMap.end()) {
        observers = it->second;
    } else {
        observers = std::make_shared<std::set<sptr<IPasteboardChangedObserver>, classcomp>>();
        observerMap.insert(std::make_pair(COMMON_USERID, observers));
    }
    observers->insert(observer);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "observers->size = %{public}u.",
        static_cast<unsigned int>(observers->size()));
}

void PasteboardService::RemoveSingleObserver(const sptr<IPasteboardChangedObserver> &observer, ObserverMap &observerMap)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "observer null.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap.find(COMMON_USERID);
    if (it == observerMap.end()) {
        return;
    }
    auto observers = it->second;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers size: %{public}u.",
        static_cast<unsigned int>(observers->size()));
    auto eraseNum = observers->erase(observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers size = %{public}u, eraseNum = %{public}zu",
        static_cast<unsigned int>(observers->size()), eraseNum);
}

void PasteboardService::RemoveAllObserver(ObserverMap &observerMap)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap.find(COMMON_USERID);
    if (it == observerMap.end()) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "observer empty.");
        return;
    }
    auto observers = it->second;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers size: %{public}u.",
        static_cast<unsigned int>(observers->size()));
    auto eraseNum = observerMap.erase(COMMON_USERID);
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
    if (callingUid == EDM_UID || callingUid == CALL_UID) {
        return true;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "callingUid error: %{public}d.", callingUid);
    return false;
}

void PasteboardService::NotifyObservers(std::string bundleName, PasteboardEventStatus status)
{
    std::thread thread([this, bundleName, status] () {
        std::lock_guard<std::mutex> lock(observerMutex_);
        for (auto &observers : observerChangedMap_) {
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

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData GetDataSize");
    size_t dataSize = GetDataSize(pasteData);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData timeC");
    CalculateTimeConsuming timeC(dataSize, pState);
}

std::shared_ptr<PasteData> PasteboardService::GetDistributedData(const Event &event, int32_t user)
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return nullptr;
    }
    std::vector<uint8_t> rawData;
    if (clipPlugin->GetPasteData(event, rawData) != 0) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "get data failed");
        Reporter::GetInstance().PasteboardFault().Report({ user, "GET_REMOTE_DATA_FAILED" });
        return nullptr;
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
    return pasteData;
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
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return false;
    }
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
    auto userId = GetCurrentAccountId();
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return;
    }
    size_t fileSize = 0;
    for (size_t i = 0; i < data.GetRecordCount(); i++) {
        auto item = data.GetRecordAt(i);
        if (item == nullptr || item->GetOrginUri() == nullptr) {
            continue;
        }
        Uri uri = *(item->GetOrginUri());
        if (!isBundleOwnUriPermission(data.GetOrginAuthority(), uri) && !item->HasGrantUriPermission()) {
            continue;
        }
        HmdfsUriInfo hui;
        auto ret = RemoteFileShare::GetDfsUriFromLocal(uri.ToString(), userId, hui);
        if (ret != 0) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "creat uri failed: %{public}d", ret);
            continue;
        }
        item->SetConvertUri(hui.uriStr);
        fileSize += hui.fileSize;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "file size: %{public}zu", fileSize);
    data.SetAddition(PasteData::REMOTE_FILE_SIZE, AAFwk::Integer::Box(fileSize));
}

std::shared_ptr<ClipPlugin> PasteboardService::GetClipPlugin()
{
    auto isOn = moduleConfig_.IsOn();
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
    p2pMap_.clear();
    std::lock_guard<decltype(mutex)> lockGuard(mutex);
    if (!isOn) {
        clipPlugin_ = nullptr;
        return;
    }
    if (clipPlugin_ != nullptr) {
        return;
    }
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
    }
}

bool PasteboardService::SubscribeKeyboardEvent()
{
    inputEventCallback_ = std::make_shared<InputEventCallback>();
    int32_t monitorId =
        MMI::InputManager::GetInstance()->AddMonitor(std::static_pointer_cast<MMI::IInputEventConsumer>(
        inputEventCallback_));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "add monitor ret is: %{public}d", monitorId);
    return monitorId >= 0;
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

bool InputEventCallback::IsCtrlVProcess(uint32_t callingPid)
{
    std::shared_lock<std::shared_mutex> lock(inputEventMutex_);
    auto curTime = static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
    return callingPid == static_cast<uint32_t>(windowPid_) && curTime - actionTime_ < EVENT_TIME_OUT;
}

void InputEventCallback::Clear()
{
    std::unique_lock<std::shared_mutex> lock(inputEventMutex_);
    actionTime_ = 0;
    windowPid_ = 0;
}
} // namespace MiscServices
} // namespace OHOS