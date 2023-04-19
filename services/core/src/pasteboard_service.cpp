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

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "account_manager.h"
#include "calculate_time_consuming.h"
#include "common/block_object.h"
#include "dev_manager.h"
#include "dev_profile.h"
#include "device/dm_adapter.h"
#include "dfx_code_constant.h"
#include "dfx_types.h"
#include "distributed_module_config.h"
#include "hiview_adapter.h"
#include "input_method_controller.h"
#include "iservice_registry.h"
#include "loader.h"
#include "native_token_info.h"
#include "os_account_manager.h"
#include "para_handle.h"
#include "pasteboard_error.h"
#include "pasteboard_dialog.h"
#include "pasteboard_trace.h"
#include "reporter.h"
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
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(new PasteboardService());
} // namespace
using namespace Security::AccessToken;
std::mutex PasteboardService::historyMutex_;
std::vector<std::string> PasteboardService::dataHistory_;
std::shared_ptr<Command> PasteboardService::copyHistory;
std::shared_ptr<Command> PasteboardService::copyData;

PasteboardService::PasteboardService()
    : SystemAbility(PASTEBOARD_SERVICE_ID, true), state_(ServiceRunningState::STATE_NOT_START)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PasteboardService Start.");
    ServiceListenerFunc_[static_cast<int32_t>(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID)] =
        &PasteboardService::DevManagerInit;
    ServiceListenerFunc_[static_cast<int32_t>(DISTRIBUTED_DEVICE_PROFILE_SA_ID)] = &PasteboardService::DevProfileInit;
}

PasteboardService::~PasteboardService()
{
}

int32_t PasteboardService::Init()
{
    if (!Publish(this)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OnStart register to system ability manager failed.");
        auto userId = GetUserIdByToken(IPCSkeleton::GetCallingTokenID());
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
    DMAdapter::GetInstance().Initialize(appInfo.bundleName);
    DistributedModuleConfig::Watch(std::bind(&PasteboardService::OnConfigChange, this, std::placeholders::_1));

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

void PasteboardService::DevManagerInit()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "begin.");
    DevManager::GetInstance().Init();
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
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetUserIdByToken(tokenId);
    if (userId == ERROR_USERID) {
        return;
    }
    std::lock_guard<std::mutex> lock(clipMutex_);
    auto it = clips_.find(userId);
    if (it != clips_.end()) {
        clips_.erase(it);
        std::string bundleName = GetAppBundleName(tokenId);
        NotifyObservers(bundleName, PasteboardEventStatus::PASTEBOARD_CLEAR);
    }
    CleanDistributedData(userId);
}

bool PasteboardService::IsDefaultIME(const AppInfo &appInfo)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "start.");
    if (appInfo.tokenType != ATokenTypeEnum::TOKEN_HAP) {
        return true;
    }
    std::shared_ptr<Property> property = InputMethodController::GetInstance()->GetCurrentInputMethod();
    return property != nullptr && property->name == appInfo.bundleName;
}

bool PasteboardService::IsFocusedApp(int32_t tokenId)
{
    using namespace OHOS::AAFwk;
    AppInfo appInfo = GetAppInfo(tokenId);
    if (appInfo.bundleName.empty()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get bundle name by token failed");
        return false;
    }
    auto elementName = AbilityManagerClient::GetInstance()->GetTopAbility();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, " Top app:%{public}s, caller app:%{public}s",
        elementName.GetBundleName().c_str(), appInfo.bundleName.c_str());
    return elementName.GetBundleName() == appInfo.bundleName;
}

bool PasteboardService::HasPastePermission(uint32_t tokenId, bool isFocusedApp,
    const std::shared_ptr<PasteData> &pasteData)
{
    if (pasteData == nullptr) {
        return false;
    }

    if (!pasteData->IsDraggedData() && (!isFocusedApp && !IsDefaultIME(GetAppInfo(tokenId)))) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "token:0x%{public}x", tokenId);
        return false;
    }
    switch (pasteData->GetShareOption()) {
        case ShareOption::InApp: {
            if (pasteData->GetTokenId() != tokenId) {
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
            PASTEBOARD_HILOGE(
                PASTEBOARD_MODULE_SERVICE, "shareOption = %{public}d is error.", pasteData->GetShareOption());
            return false;
        }
    }
    return true;
}

AppInfo PasteboardService::GetAppInfo(uint32_t tokenId)
{
    AppInfo info;
    info.tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (info.tokenType) {
        case ATokenTypeEnum::TOKEN_HAP: {
            HapTokenInfo hapInfo;
            if (AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != 0) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get hap token info fail.");
                return info;
            }
            info.bundleName = hapInfo.bundleName;
            info.userId = hapInfo.userID;
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
            info.userId = 0;
            break;
        }
        default: {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "tokenType = %{public}d not match.", info.tokenType);
        }
    }
    return info;
}

std::string PasteboardService::GetAppBundleName(uint32_t tokenId)
{
    auto appInfo = GetAppInfo(tokenId);
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
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "isLocalPaste = %{public}d.", pasteData.IsLocalPaste());
}

int32_t PasteboardService::GetPasteData(PasteData &data)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (pasting_.exchange(true)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "is passing.");
        return static_cast<int32_t>(PasteboardError::E_IS_BEGING_PROCESSED);
    }

    CalculateTimeConsuming::SetBeginTime();

    SetDeviceName();
    PasteboardTrace tracer("PasteboardService GetPasteData");

    bool isFocusedApp = IsFocusedApp(tokenId);
    auto block =  std::make_shared<BlockObject<std::shared_ptr<PasteData>>>(PasteBoardDialog::POPUP_INTERVAL);
    std::thread thread([this, block, tokenId, isFocusedApp]() mutable {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData Begin");
        std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
        auto success = GetPasteData(*pasteData, tokenId, isFocusedApp);
        if (!success) {
            pasteData->SetInvalid();
        }
        block->SetValue(pasteData);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData End");
    });
    thread.detach();
    auto value = block->GetValue();
    std::string pop;
    if (value == nullptr) {
        PasteBoardDialog::MessageInfo message;
        message.appName = GetAppLabel(tokenId);
        message.deviceType = GetDeviceName();
        PasteBoardDialog::GetInstance().ShowDialog(message, [block] { block->SetValue(nullptr); });
        pop = "pop";
        block->SetInterval(PasteBoardDialog::MAX_LIFE_TIME);
        value = block->GetValue();
        PasteBoardDialog::GetInstance().CancelDialog();
    }
    bool result = false;
    if (value != nullptr) {
        result = value->IsValid();
        data = std::move(*value);
    }
    std::string bundleName = GetAppBundleName(tokenId);
    NotifyObservers(bundleName, PasteboardEventStatus::PASTEBOARD_READ);
    GetPasteDataDot(data, pop, tokenId);
    pasting_.store(false);
    return result ? static_cast<int32_t>(PasteboardError::E_OK) : static_cast<int32_t>(PasteboardError::E_ERROR);
}

bool PasteboardService::GetPasteData(PasteData &data, uint32_t tokenId, bool isFocusedApp)
{
    PasteboardTrace tracer("GetPasteData inner");
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start inner.");
    auto userId = GetUserIdByToken(tokenId);
    if (userId == ERROR_USERID) {
        PasteData::sharePath = "";
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId error.");
        return false;
    }
    PasteData::sharePath = PasteData::SHARE_PATH_PREFIX + std::to_string(userId) + PasteData::SHARE_PATH_PREFIX_ACCOUNT;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Clips length %{public}d.", static_cast<uint32_t>(clips_.size()));
    bool isRemote = false;
    std::lock_guard<std::mutex> lock(clipMutex_);
    auto pastData = GetDistributedData(userId);
    if (pastData != nullptr) {
        isRemote = true;
        pastData->SetRemote(isRemote);
        clips_.insert_or_assign(userId, pastData);
    }
    auto it = clips_.find(userId);
    if (it == clips_.end()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "no data.");
        return false;
    }

    auto ret = HasPastePermission(tokenId, isFocusedApp, it->second);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "don't have paste permission.");
        return false;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetPasteData success.");

    data = *(it->second);
    data.SetRemote(isRemote);

    SetLocalPasteFlag(isRemote, tokenId, data);
    return true;
}

bool PasteboardService::HasPasteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto userId = GetUserIdByToken(tokenId);
    if (userId == ERROR_USERID) {
        return false;
    }
    std::lock_guard<std::mutex> lock(clipMutex_);
    auto it = clips_.find(userId);
    if (it == clips_.end()) {
        return HasDistributedData(userId);
    }

    return HasPastePermission(tokenId, IsFocusedApp(tokenId), it->second);
}

int32_t PasteboardService::SetPasteData(PasteData &pasteData)
{
    PasteboardTrace tracer("PasteboardService, SetPasteData");
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
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
        return static_cast<int32_t>(PasteboardError::E_ERROR);
    }
    pasteData.SetBundleName(appInfo.bundleName);
    std::string time = GetTime();
    pasteData.SetTime(time);
    pasteData.SetTokenId(tokenId);

    std::lock_guard<std::mutex> lock(clipMutex_);
    auto it = clips_.find(appInfo.userId);
    if (it != clips_.end()) {
        clips_.erase(it);
    }
    clips_.insert_or_assign(appInfo.userId, std::make_shared<PasteData>(pasteData));
    SetDistributedData(appInfo.userId, pasteData);
    NotifyObservers(appInfo.bundleName, PasteboardEventStatus::PASTEBOARD_WRITE);
    SetPasteDataDot(pasteData);
    setting_.store(false);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Clips length %{public}d.", static_cast<uint32_t>(clips_.size()));
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardService::GetUserIdByToken(uint32_t tokenId)
{
    auto appInfo = GetAppInfo(tokenId);
    PASTEBOARD_HILOGD(
        PASTEBOARD_MODULE_SERVICE, "tokenId = 0x%{public}x, userId = %{public}d.", tokenId, appInfo.userId);
    return appInfo.userId;
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
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    AddObserver(observer, observerChangedMap_);
}

void PasteboardService::RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    RemoveSingleObserver(observer, observerChangedMap_);
}

void PasteboardService::RemoveAllChangedObserver()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    RemoveAllObserver(observerChangedMap_);
}

void PasteboardService::AddPasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    if (!IsCallerUidValid()) {
        return;
    }
    AddObserver(observer, observerEventMap_);
}

void PasteboardService::RemovePasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    if (!IsCallerUidValid()) {
        return;
    }
    RemoveSingleObserver(observer, observerEventMap_);
}

void PasteboardService::RemoveAllEventObserver()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
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
    auto userId = GetUserIdByToken(IPCSkeleton::GetCallingTokenID());
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
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
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, " observers->size = %{public}u.",
        static_cast<unsigned int>(observers->size()));
}

void PasteboardService::RemoveSingleObserver(const sptr<IPasteboardChangedObserver> &observer, ObserverMap &observerMap)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "observer null.");
        return;
    }
    auto userId = GetUserIdByToken(IPCSkeleton::GetCallingTokenID());
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap.find(userId);
    if (it == observerMap.end()) {
        return;
    }
    auto observers = it->second;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "observers->size: %{public}u.",
        static_cast<unsigned int>(observers->size()));
    auto eraseNum = observers->erase(observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "listeners.size = %{public}u, eraseNum = %{public}zu",
        static_cast<unsigned int>(observers->size()), eraseNum);
}

void PasteboardService::RemoveAllObserver(ObserverMap &observerMap)
{
    auto userId = GetUserIdByToken(IPCSkeleton::GetCallingTokenID());
    if (userId == ERROR_USERID) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "userId invalid.");
        return;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    auto it = observerMap.find(userId);
    if (it == observerMap.end()) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_SERVICE, "observer empty.");
        return;
    }
    auto observers = it->second;
    auto eraseNum = observerMap.erase(userId);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "listeners.size = %{public}u, eraseNum = %{public}zu",
        static_cast<unsigned int>(observers->size()), eraseNum);
}

inline bool PasteboardService::IsCallerUidValid()
{
    pid_t callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid == EDM_UID || callingUid == ROOT_UID) {
        return true;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "callingUid error: %{public}d.", callingUid);
    return false;
}

void PasteboardService::NotifyObservers(std::string bundleName, PasteboardEventStatus status)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    std::lock_guard<std::mutex> lock(observerMutex_);
    for (auto &observers : observerChangedMap_) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "notify uid : %{public}d, changed observers size: %{public}u",
            observers.first, static_cast<unsigned int>(observers.second->size()));
        for (const auto &observer : *(observers.second)) {
            if (status != PasteboardEventStatus::PASTEBOARD_READ) {
                observer->OnPasteboardChanged();
            }
        }
    }
    for (auto &observers : observerEventMap_) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "notify uid : %{public}d, event observers size: %{public}u",
            observers.first, static_cast<unsigned int>(observers.second->size()));
        for (const auto &observer : *(observers.second)) {
            observer->OnPasteboardEvent(bundleName, static_cast<int32_t>(status));
        }
    }
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
    std::string history = std::move(info.time) + " " + std::move(info.bundleName) + " " + std::move(info.state) + " "
                          + " " + std::move(info.pop) + " " + std::move(info.remote);
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

void PasteboardService::ShareOptionToString(ShareOption shareOption, std::string &out)
{
    if (shareOption == ShareOption::InApp) {
        out = "InAPP";
    } else if (shareOption == ShareOption::LocalDevice) {
        out = "LocalDevice";
    } else {
        out = "CrossDevice";
    }
}

std::string PasteboardService::DumpData()
{
    std::vector<int32_t> ids;
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "query active user failed errCode=%{public}d", ret);
        return "";
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "id = %{public}d", ids[0]);
    std::lock_guard<std::mutex> lock(clipMutex_);
    auto it = clips_.find(ids[0]);
    std::string result;
    if (it != clips_.end() && it->second != nullptr) {
        size_t recordCounts = it->second->GetRecordCount();
        auto property = it->second->GetProperty();
        std::string shareOption;
        ShareOptionToString(property.shareOption, shareOption);
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
        result.append("}")
            .append("\n")
            .append("|source device:  ")
            .append(sourceDevice);
    } else {
        result.append("No copy data.").append("\n");
    }
    return result;
}

void PasteboardService::SetPasteDataDot(PasteData &pasteData)
{
    auto property = pasteData.GetProperty();
    HistoryInfo info{ property.setTime, property.bundleName, "set", "", "" };
    SetPasteboardHistory(info);

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetPasteData Report!");
    Reporter::GetInstance().PasteboardBehaviour().Report(
        { static_cast<int>(BehaviourPasteboardState::BPS_COPY_STATE), property.bundleName });

    int state = static_cast<int>(StatisticPasteboardState::SPS_COPY_STATE);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetPasteData GetDataSize!");
    size_t dataSize = GetDataSize(pasteData);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetPasteData timeC!");
    CalculateTimeConsuming timeC(dataSize, state);
}

void PasteboardService::GetPasteDataDot(PasteData &pasteData, const std::string &pop, uint32_t tokenId)
{
    auto property = pasteData.GetProperty();
    std::string remote;
    if (property.isRemote) {
        remote = "remote";
    }
    std::string time = GetTime();
    auto appInfo = GetAppInfo(tokenId);
    HistoryInfo info{ time, appInfo.bundleName, "get", pop, remote };
    SetPasteboardHistory(info);

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData Report!");
    int pState = StatisticPasteboardState::SPS_INVALID_STATE;
    int bState = BehaviourPasteboardState::BPS_INVALID_STATE;
    if (property.isRemote) {
        pState = static_cast<int>(StatisticPasteboardState::SPS_REMOTE_PASTE_STATE);
        bState = static_cast<int>(BehaviourPasteboardState::BPS_REMOTE_PASTE_STATE);
    } else {
        pState = static_cast<int>(StatisticPasteboardState::SPS_PASTE_STATE);
        bState = static_cast<int>(BehaviourPasteboardState::BPS_PASTE_STATE);
    };

    Reporter::GetInstance().PasteboardBehaviour().Report({ bState, appInfo.bundleName });

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData GetDataSize");
    size_t dataSize = GetDataSize(pasteData);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetPasteData timeC");
    CalculateTimeConsuming timeC(dataSize, pState);
}

std::shared_ptr<PasteData> PasteboardService::GetDistributedData(int32_t user)
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return nullptr;
    }
    ClipPlugin::GlobalEvent event;
    auto isEffective = GetDistributedEvent(clipPlugin, user, event);
    if (event.status == ClipPlugin::EVT_UNKNOWN) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "EVT_UNKNOWN.");
        return nullptr;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "same device:%{public}d, evt seq:%{public}u current seq:%{public}u.",
        event.deviceId == currentEvent_.deviceId, event.seqId, currentEvent_.seqId);
    std::vector<uint8_t> rawData = std::move(event.addition);
    SetDeviceName(event.deviceId);
    if (!isEffective) {
        currentEvent_.status = ClipPlugin::EVT_INVALID;
        currentEvent_ = std::move(event);
        Reporter::GetInstance().PasteboardFault().Report({ user, "GET_REMOTE_DATA_FAILED" });
        return nullptr;
    }

    if (event.frameNum > 0 && (clipPlugin->GetPasteData(event, rawData) != 0)) {
        Reporter::GetInstance().PasteboardFault().Report({ user, "GET_REMOTE_DATA_FAILED" });
        return nullptr;
    }

    currentEvent_ = std::move(event);
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->Decode(rawData);
    pasteData->ReplaceShareUri(user);
    return pasteData;
}

bool PasteboardService::SetDistributedData(int32_t user, PasteData &data)
{
    std::vector<uint8_t> rawData;
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return false;
    }

    if (data.GetShareOption() == CrossDevice && !data.Encode(rawData)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "encode failed.");
        return false;
    }

    uint64_t expiration =
        duration_cast<milliseconds>((system_clock::now() + minutes(EXPIRATION_INTERVAL)).time_since_epoch()).count();
    Event event;
    event.user = user;
    event.seqId = ++sequenceId_;
    event.expiration = expiration;
    event.deviceId = DMAdapter::GetInstance().GetLocalDevice();
    event.account = AccountManager::GetInstance().GetCurrentAccount();
    event.status = (data.GetShareOption() == CrossDevice) ? ClipPlugin::EVT_NORMAL : ClipPlugin::EVT_INVALID;
    currentEvent_ = event;
    clipPlugin->SetPasteData(event, rawData);
    return true;
}

bool PasteboardService::HasDistributedData(int32_t user)
{
    auto clipPlugin = GetClipPlugin();
    if (clipPlugin == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "clipPlugin null.");
        return false;
    }
    Event event;
    auto has = GetDistributedEvent(clipPlugin, user, event);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "same device:%{public}d, evt seq:%{public}u current seq:%{public}u.",
        event.deviceId == currentEvent_.deviceId, event.seqId, currentEvent_.seqId);
    return has;
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
    if (isOn) {
        return;
    }
    std::lock_guard<decltype(mutex)> lockGuard(mutex);
    clipPlugin_ = nullptr;
}

bool PasteboardService::GetDistributedEvent(std::shared_ptr<ClipPlugin> plugin, int32_t user, Event &event)
{
    auto events = plugin->GetTopEvents(1, user);
    if (events.empty()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "events empty.");
        return false;
    }

    auto &tmpEvent = events[0];
    if (tmpEvent.deviceId == DMAdapter::GetInstance().GetLocalDevice()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get local data.");
        return false;
    }
    if (tmpEvent.account != AccountManager::GetInstance().GetCurrentAccount()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "account error.");
        return false;
    }
    if (tmpEvent.deviceId == currentEvent_.deviceId && tmpEvent.seqId == currentEvent_.seqId) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get same remote data.");
        return false;
    }

    event = std::move(tmpEvent);
    uint64_t curTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return ((curTime < event.expiration) && (event.status == ClipPlugin::EVT_NORMAL));
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

std::string PasteboardService::GetDeviceName()
{
    std::lock_guard<decltype(deviceMutex_)> lockGuard(deviceMutex_);
    return fromDevice_;
}

void PasteboardService::SetDeviceName(const std::string &device)
{
    std::lock_guard<decltype(deviceMutex_)> lockGuard(deviceMutex_);
    if (device.empty() || device == DMAdapter::GetInstance().GetLocalDevice()) {
        fromDevice_ = "local";
        return;
    }
    fromDevice_ = DMAdapter::GetInstance().GetDeviceName(device);
}
} // namespace MiscServices
} // namespace OHOS
