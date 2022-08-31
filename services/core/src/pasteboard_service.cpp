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
#include "pasteboard_service.h"

#include <unistd.h>

#include "accesstoken_kit.h"
#include "account_manager.h"
#include "calculate_time_consuming.h"
#include "dev_manager.h"
#include "dev_profile.h"
#include "dfx_code_constant.h"
#include "dfx_types.h"
#include "distributed_module_config.h"
#include "device/dm_adapter.h"
#include "hiview_adapter.h"
#include "input_method_controller.h"
#include "iservice_registry.h"
#include "loader.h"
#include "native_token_info.h"
#include "os_account_manager.h"
#include "para_handle.h"
#include "pasteboard_common.h"
#include "pasteboard_trace.h"
#include "reporter.h"
#include "system_ability_definition.h"
#ifdef WITH_DLP
#include "dlp_permission_kit.h"
#endif // WITH_DLP

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;
namespace {
constexpr const int GET_WRONG_SIZE = 0;
const std::int32_t INIT_INTERVAL = 10000L;
const std::string PASTEBOARD_SERVICE_NAME = "PasteboardService";
const std::string FAIL_TO_GET_TIME_STAMP = "FAIL_TO_GET_TIME_STAMP";
const std::string DEFAULT_IME_BUNDLE_NAME = "com.example.kikakeyboard";
const std::int32_t ERROR_USERID = -1;
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(new PasteboardService());
} // namespace
using namespace Security::AccessToken;

std::vector<std::shared_ptr<std::string>> PasteboardService::dataHistory_;
std::shared_ptr<Command> PasteboardService::copyHistory;
std::shared_ptr<Command> PasteboardService::copyData;
int32_t PasteboardService::focusAppUid_ = 0;

PasteboardService::PasteboardService()
    : SystemAbility(PASTEBOARD_SERVICE_ID, true),
      state_(ServiceRunningState::STATE_NOT_START)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PasteboardService Start.");
}

PasteboardService::~PasteboardService() {}

int32_t PasteboardService::Init()
{
    if (!Publish(this)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OnStart register to system ability manager failed.");
        auto userId = GetUserId();
        Reporter::GetInstance().InitializationFault().Report({ userId, "ERR_INVALID_OPTION" });
        return ERR_INVALID_OPTION;
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
    AppInfo appInfo;
    GetAppInfoByTokenId(IPCSkeleton::GetCallingTokenID(), appInfo);
    Loader loader;
    loader.LoadComponents();
    DMAdapter::GetInstance().Initialize(appInfo.bundleName);
    DistributedModuleConfig::Watch(std::bind(&PasteboardService::OnConfigChange, this, std::placeholders::_1));

    DevManager::GetInstance().Init();
    ParaHandle::GetInstance().Init();
    DevProfile::GetInstance().Init();

    if (Init() != ERR_OK) {
        auto callback = [this]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Init failed. Try again 10s later.");
        return;
    }

    if (focusChangedListener_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "focusChangedListener_.");
        focusChangedListener_ = new PasteboardService::PasteboardFocusChangedListener();
    }
    Rosen::WindowManager::GetInstance().RegisterFocusChangedListener(focusChangedListener_);

    copyHistory = std::make_shared<Command>(std::vector<std::string>{ "--copy-history" },
        "Dump access history last ten times.",
        [this](const std::vector<std::string> &input, std::string &output) -> bool {
            output = DumpHistory();
            return true;
        });

    copyData = std::make_shared<Command>(std::vector<std::string>{ "--data" }, "Show copy data details.",
        [this](const std::vector<std::string> &input, std::string &output) -> bool {
            output = DunmpData();
            return true;
        });

    PasteboardDumpHelper::GetInstance().RegisterCommand(copyHistory);
    PasteboardDumpHelper::GetInstance().RegisterCommand(copyData);

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
    DevManager::GetInstance().UnregisterDevCallback();

    Rosen::WindowManager::GetInstance().UnregisterFocusChangedListener(focusChangedListener_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OnStop End.");
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

void PasteboardService::InitStorage()
{
    if (pasteboardStorage_ == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Init storage handler.");
    }

    if (pasteboardStorage_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Init storage handler failed.");
        return;
    }
}

void PasteboardService::Clear()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    auto userId = GetUserId();
    if (userId == ERROR_USERID) {
        return;
    }
    std::lock_guard<std::mutex> lock(clipMutex_);
    auto it = clips_.find(userId);
    if (it != clips_.end()) {
        clips_.erase(it);
        NotifyObservers();
    }
    CleanDistributedData();
}

void PasteboardService::PasteboardFocusChangedListener::OnFocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnFocused.");
    focusAppUid_ = focusChangeInfo->uid_;
}

void PasteboardService::PasteboardFocusChangedListener::OnUnfocused(const sptr<Rosen::FocusChangeInfo> &focusChangeInfo)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnUnfocused.");
    focusAppUid_ = 0;
}

bool PasteboardService::IsFocusOrDefaultIme(const AppInfo &appInfo)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "IsFocusOrDefaultIme start.");
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    if (property != nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "packageName = %{public}s.", property->packageName.c_str());
        if (property->packageName == appInfo.bundleName) {
            return true;
        }
    }

    bool isFocusApp = false;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "uid = %{public}d, focusAppUid_ = %{public}d.",
        IPCSkeleton::GetCallingUid(), focusAppUid_);
    if (IPCSkeleton::GetCallingUid() == focusAppUid_) {
        isFocusApp = true;
    }
    return isFocusApp;
}

bool PasteboardService::HasPastePermission(const std::string &appId, ShareOption shareOption)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "shareOption = %{public}d.", static_cast<uint32_t>(shareOption));

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    AppInfo appInfo;
    auto ret = GetAppInfoByTokenId(tokenId, appInfo);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetAppInfoByTokenId failed.");
        return false;
    }
    if (appInfo.tokenType == ATokenTypeEnum::TOKEN_HAP) {
        auto isFocusOrDefaultIme = IsFocusOrDefaultIme(appInfo);
        if (!isFocusOrDefaultIme) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "IsFocusOrDefaultIme check failed.");
            return false;
        }
    }

    switch (shareOption) {
        case ShareOption::InApp: {
            if (appInfo.appId != appId) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
                    "InApp check failed, appId = %{public}s, currentAppId = %{public}s.", appId.c_str(),
                    appInfo.appId.c_str());
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
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "shareOption = %{public}d is error.", shareOption);
            return false;
        }
    }
    return true;
}

bool PasteboardService::GetAppInfoByTokenId(int32_t tokenId, AppInfo &appInfo)
{
    appInfo.tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (appInfo.tokenType) {
        case ATokenTypeEnum::TOKEN_HAP: {
            HapTokenInfo hapInfo;
            if (AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != 0) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get hap token info fail.");
                return false;
            }
            appInfo.appId = hapInfo.appID;
            appInfo.bundleName = hapInfo.bundleName;
            break;
        }
        case ATokenTypeEnum::TOKEN_NATIVE:
        case ATokenTypeEnum::TOKEN_SHELL: {
            NativeTokenInfo tokenInfo;
            if (AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != 0) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get native token info fail.");
                return false;
            }
            appInfo.appId = tokenInfo.processName;
            appInfo.bundleName = tokenInfo.processName;
            break;
        }
        default: {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "token type not match.");
            return false;
        }
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "tokenType = %{public}d, appId = %{public}s, bundleName = %{public}s.",
        appInfo.tokenType, appInfo.appId.c_str(), appInfo.bundleName.c_str());
    return true;
}

bool PasteboardService::GetPasteData(PasteData& data)
{
    PasteboardTrace tracer("PasteboardService, GetPasteData");
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    auto userId = GetUserId();

    GetPasteDataDot();

    if (userId == ERROR_USERID) {
        return false;
    }

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Clips length %{public}d.", static_cast<uint32_t>(clips_.size()));
    std::lock_guard<std::mutex> lock(clipMutex_);
    auto pastData = GetDistributedData();
    if (pastData != nullptr) {
        clips_.insert_or_assign(userId, pastData);
    }
    auto it = clips_.find(userId);
    if (it == clips_.end()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no data.");
        return false;
    }
    auto ret = HasPastePermission(it->second->GetAppId(), it->second->GetShareOption());
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "HasPastePermission failed.");
        return false;
    }
    data = *(it->second);
    return true;
}

bool PasteboardService::HasPasteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    auto userId = GetUserId();
    if (userId == ERROR_USERID) {
        return false;
    }
    std::lock_guard<std::mutex> lock(clipMutex_);
    auto it = clips_.find(userId);
    if (it == clips_.end()) {
        return HasDistributedData();
    }
    return HasPastePermission(it->second->GetAppId(), it->second->GetShareOption());
}

void PasteboardService::SetPasteData(PasteData& pasteData)
{
    PasteboardTrace tracer("PasteboardService, SetPasteData");
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
#ifdef WITH_DLP
    auto callingToken = IPCSkeleton::GetCallingTokenID();
    bool copyable = false;
    auto ret = Security::DlpPermission::DlpPermissionKit::QueryDlpFileCopyableByTokenId(copyable, callingToken);
    if (ret != 0 || !copyable) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ret = %{public}d, copyable = %{public}d.", ret, copyable);
        return;
    }
#endif
    auto userId = GetUserId();

    SetPasteDataDot(pasteData);

    if (userId == ERROR_USERID) {
        return;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    AppInfo appInfo;
    auto ret = GetAppInfoByTokenId(tokenId, appInfo);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetAppInfoByTokenId failed.");
        return;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "appId = %{public}s.", appInfo.appId.c_str());
    pasteData.SetAppId(appInfo.appId);

    std::lock_guard<std::mutex> lock(clipMutex_);
    auto it = clips_.find(userId);
    if (it != clips_.end()) {
        clips_.erase(it);
    }
    clips_.insert_or_assign(userId, std::make_shared<PasteData>(pasteData));
    SetDistributedData(userId, pasteData);
    NotifyObservers();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Clips length %{public}d.", static_cast<uint32_t>(clips_.size()));
}

int32_t PasteboardService::GetUserId()
{
    int32_t userId = ERROR_USERID;
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto result = AccountSA::OsAccountManager::GetOsAccountLocalIdFromUid(uid, userId);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE,
        "Get UserId, uid = %{public}d, userId = %{public}d, result = %{public}d.", uid, userId, result);
    return userId;
}

void PasteboardService::AddPasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "nullptr.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto userId = GetUserId();
    if (userId == ERROR_USERID) {
        return;
    }
    auto it = observerMap_.find(userId);
    std::shared_ptr<std::set<sptr<IPasteboardChangedObserver>, classcomp>> observers;
    if (it != observerMap_.end()) {
        observers = it->second;
    } else {
        observers = std::make_shared<std::set<sptr<IPasteboardChangedObserver>, classcomp>>();
        observerMap_.insert(std::make_pair(userId, observers));
    }
    observers->insert(observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE,
        " observer = %{public}p, observers->size = %{public}d,",
        observer.GetRefPtr(),
        static_cast<unsigned int>(observerMap_.size()));
}
void PasteboardService::RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    if (observer == nullptr) {
        return;
    }
    auto userId = GetUserId();
    if (userId == ERROR_USERID) {
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap_.find(userId);
    if (it == observerMap_.end()) {
        return;
    }
    auto observers = it->second;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers->size: %{public}d.",
        static_cast<unsigned int>(observers->size()));
    auto eraseNum = observers->erase(observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE,
        " callback = %{public}p, listeners.size = %{public}d,"
        " eraseNum = %{public}zu",
        observer.GetRefPtr(),
        static_cast<unsigned int>(observers->size()),
        eraseNum);
}

void PasteboardService::RemoveAllChangedObserver()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    auto userId = GetUserId();
    if (userId == ERROR_USERID) {
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap_.find(userId);
    if (it == observerMap_.end()) {
        return;
    }
    observerMap_.erase(userId);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "end.");
}

void PasteboardService::NotifyObservers()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    std::lock_guard<std::mutex> lock(observerMutex_);
    for (auto &observers : observerMap_) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "notify uid : %{public}d.", observers.first);
        for (const auto &observer : *(observers.second)) {
            observer->OnPasteboardChanged();
        }
    }
}

size_t PasteboardService::GetDataSize(PasteData& data) const
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

bool PasteboardService::GetBundleNameByUid(int32_t uid, std::string &bundleName)
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> remoteObject =
        systemAbilityManager->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);

    sptr<AppExecFwk::IBundleMgr> iBundleMgr = OHOS::iface_cast<AppExecFwk::IBundleMgr>(remoteObject);
    if (iBundleMgr == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, " permission check failed, cannot get IBundleMgr.");
        return false;
    }
    return iBundleMgr->GetBundleNameForUid(uid, bundleName);
}

bool PasteboardService::SetPasteboardHistory(int32_t uid, std::string state, std::string timeStamp)
{
    constexpr const size_t DATA_HISTORY_SIZE = 10;
    std::string bundleName;
    if (GetBundleNameByUid(uid, bundleName)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "get bundleName success!");
    } else {
        bundleName = "com.pasteboard.default";
    }

    std::string bundleNameState = timeStamp + "  " + bundleName + "    " + state;
    std::shared_ptr<std::string> pBundleNameState = std::make_shared<std::string>(bundleNameState);
    if (dataHistory_.size() == DATA_HISTORY_SIZE) {
        dataHistory_.erase(dataHistory_.begin());
    }
    dataHistory_.push_back(pBundleNameState);
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

    std::string targetTime = std::to_string(nowTime.tm_year + 1900) + "-"
                             + std::to_string(nowTime.tm_mon + 1) + "-"
                             + std::to_string(nowTime.tm_mday) + " "
                             + std::to_string(nowTime.tm_hour) + ":"
                             + std::to_string(nowTime.tm_min) + ":"
                             + std::to_string(nowTime.tm_sec) + "."
                             + std::to_string(timeVal.tv_usec / USEC_TO_MSEC);
    return targetTime;
}

std::string PasteboardService::DumpHistory() const
{
    std::string result;
    if (!dataHistory_.empty()) {
    result.append("Access history last ten times: ").append("\n");
    for (auto iter = dataHistory_.rbegin(); iter != dataHistory_.rend(); ++iter) {
        result.append("          ")
            .append(**iter)
            .append("\n");
        }
    } else {
        result.append("Access history fail! dataHistory_ no data.").append("\n");
    }
    return result;
}

std::string PasteboardService::DunmpData()
{
    std::string result;
    std::vector<std::string> mimeTypes;
    std::string bundleName;
    if (!clips_.empty()) {
        size_t recordCounts = clips_.rbegin()->second->GetRecordCount();
        mimeTypes = clips_.rbegin()->second->GetMimeTypes();
        if (GetBundleNameByUid(uIdForLastCopy_, bundleName)) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "get bundleName success!");
        } else {
            bundleName = "com.pasteboard.default";
        }

        result.append("|Owner       :  ")
         .append(bundleName).append("\n")
         .append("|Timestamp   :  ")
         .append(timeForLastCopy_).append("\n")
         .append("|Share Option: ")
         .append(" CrossDevice").append("\n")
         .append("|Record Count:  ")
         .append(std::to_string(recordCounts)).append("\n")
         .append("|Mime types  :  {");
        if (!mimeTypes.empty()) {
            for (size_t i = 0; i < mimeTypes.size(); ++i) {
                result.append(mimeTypes[i]).append(",");
            }
        }
        result.append("}");
    } else {
        result.append("No copy data.").append("\n");
    }
    return result;
}

void PasteboardService::SetPasteDataDot(PasteData& pasteData)
{
    int32_t uId = IPCSkeleton::GetCallingUid();
    uIdForLastCopy_ = uId;
    std::string time = GetTime();
    timeForLastCopy_ = time;
    SetPasteboardHistory(uId, "Set", time);
    std::string bundleName;
    if (GetBundleNameByUid(uId, bundleName)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "get bundleName success!");
    } else {
        bundleName = "com.pasteboard.default";
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "default bundleName!");
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetPasteData Report!");
    Reporter::GetInstance().PasteboardBehaviour().Report(
        { static_cast<int>(BehaviourPasteboardState::BPS_COPY_STATE), bundleName });

    int state = static_cast<int>(StatisticPasteboardState::SPS_COPY_STATE);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetPasteData GetDataSize!");
    size_t dataSize = GetDataSize(pasteData);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetPasteData timeC!");
    CalculateTimeConsuming timeC(dataSize, state);
}

void PasteboardService::GetPasteDataDot()
{
    int32_t uId = IPCSkeleton::GetCallingUid();
    std::string bundleName;
    std::string time = GetTime();
    SetPasteboardHistory(uId, "Get", time);
    if (GetBundleNameByUid(uId, bundleName)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "get bundleName success!");
    } else {
        bundleName = "com.pasteboard.default";
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "defaulit bundlename");
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData Report!");
    Reporter::GetInstance().PasteboardBehaviour().Report(
        { static_cast<int>(BehaviourPasteboardState::BPS_PASTE_STATE), bundleName });

    if (!clips_.empty()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData GetDataSize");
        int state = static_cast<int>(StatisticPasteboardState::SPS_PASTE_STATE);
        size_t dataSize = GetDataSize(*(clips_.rbegin()->second));
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData timeC");
        CalculateTimeConsuming timeC(dataSize, state);
    }
}

std::shared_ptr<PasteData> PasteboardService::GetDistributedData()
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        return nullptr;
    }
    ClipPlugin::GlobalEvent event;
    auto isExpiration = GetDistributedEvent(clipPlugin, event);
    if (event.status == ClipPlugin::EVT_INVALID) {
        return nullptr;
    }

    std::vector<uint8_t> rawData = std::move(event.addition);
    if (!isExpiration) {
        currentEvent_ = std::move(event);
        currentEvent_.status = ClipPlugin::EVT_TIMEOUT;
        return nullptr;
    }

    if (event.frameNum > 0) {
        clipPlugin->GetPasteData(event, rawData);
    }

    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->Decode(rawData);
    return pasteData;
}

bool PasteboardService::SetDistributedData(int32_t user, PasteData &data)
{
    std::vector<uint8_t> rawData;
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr || (!data.Encode(rawData))) {
        return false;
    }

    uint64_t expiration =
        duration_cast<milliseconds>((system_clock::now() + minutes(EXPIRATION_INTERVAL)).time_since_epoch()).count();
    ClipPlugin::GlobalEvent event;
    event.seqId = ++sequenceId_;
    event.expiration = expiration;
    event.deviceId = DMAdapter::GetInstance().GetLocalDevice();
    event.account = AccountManager::GetInstance().GetCurrentAccount();
    event.status = ClipPlugin::EVT_NORMAL;
    currentEvent_ = event;
    clipPlugin->SetPasteData(event, rawData);
    return true;
}

bool PasteboardService::HasDistributedData()
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        return false;
    }
    ClipPlugin::GlobalEvent event;
    return GetDistributedEvent(clipPlugin, event);
}

std::shared_ptr<ClipPlugin> PasteboardService::GetClipPlugin()
{
    auto isOn = DistributedModuleConfig::IsOn();
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

bool PasteboardService::CleanDistributedData()
{
    currentEvent_.status = ClipPlugin::EVT_CLEANED;
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        return true;
    }
    clipPlugin->Clear();
    return true;
}

void PasteboardService::OnConfigChange(bool isOn)
{
    if (isOn) {
        return;
    }
    std::lock_guard<decltype(mutex)> lockGuard(mutex);
    clipPlugin_ = nullptr;
}

bool PasteboardService::GetDistributedEvent(std::shared_ptr<ClipPlugin> plugin, ClipPlugin::GlobalEvent &event)
{
    auto events = plugin->GetTopEvents(1);
    if (events.empty()) {
        return false;
    }

    auto &tmpEvent = events[0];
    if (tmpEvent.deviceId == DMAdapter::GetInstance().GetLocalDevice() ||
        tmpEvent.account != AccountManager::GetInstance().GetCurrentAccount() ||
        (tmpEvent.deviceId == currentEvent_.deviceId && tmpEvent.seqId == currentEvent_.seqId)) {
        return false;
    }

    event = std::move(tmpEvent);
    uint64_t curTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return (curTime < event.expiration);
}
} // namespace MiscServices
} // namespace OHOS