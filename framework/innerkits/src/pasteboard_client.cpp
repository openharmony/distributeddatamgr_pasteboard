/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include <iservice_registry.h>
#include <thread>

#include "convert_utils.h"
#include "ffrt_utils.h"
#include "hitrace_meter.h"
#include "ipasteboard_client_death_observer.h"
#include "pasteboard_copy.h"
#include "pasteboard_deduplicate_memory.h"
#include "pasteboard_delay_getter_client.h"
#include "pasteboard_entry_getter_client.h"
#include "pasteboard_error.h"
#include "pasteboard_event_dfx.h"
#include "pasteboard_hilog.h"
#include "pasteboard_load_callback.h"
#include "pasteboard_progress.h"
#include "pasteboard_signal_callback.h"
#include "pasteboard_time.h"
#include "pasteboard_utils.h"
#include "pasteboard_web_controller.h"
#include "system_ability_definition.h"
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
constexpr const int32_t HITRACE_GETPASTEDATA = 0;
std::string g_progressKey;
constexpr int32_t LOADSA_TIMEOUT_MS = 4000;
constexpr int32_t PASTEBOARD_PROGRESS_UPDATE_PERCENT = 5;
constexpr int32_t UPDATE_PERCENT_WITHOUT_FILE = 10;
constexpr int32_t PASTEBOARD_PROGRESS_TWENTY_PERCENT = 20;
constexpr int32_t PASTEBOARD_PROGRESS_FINISH_PERCENT = 100;
constexpr int32_t PASTEBOARD_PROGRESS_SLEEP_TIME = 100; // ms
constexpr int32_t SLEEP_TIME_WITHOUT_FILE = 50; // ms
constexpr int32_t PASTEBOARD_PROGRESS_RETRY_TIMES = 10;
constexpr int64_t REPORT_DUPLICATE_TIMEOUT = 2 * 60 * 1000; // 2 minutes
static constexpr int32_t HAP_PULL_UP_TIME = 500; // ms
static constexpr int32_t HAP_MIN_SHOW_TIME = 300; // ms
sptr<IPasteboardService> PasteboardClient::pasteboardServiceProxy_;
PasteboardClient::StaticDestoryMonitor PasteboardClient::staticDestoryMonitor_;
std::mutex PasteboardClient::instanceLock_;
std::condition_variable PasteboardClient::proxyConVar_;
sptr<IRemoteObject> clientDeathObserverPtr_;
std::atomic<bool> PasteboardClient::remoteTask_(false);
std::atomic<bool> PasteboardClient::isPasting_(false);
std::atomic<uint64_t> PasteboardClient::progressStartTime_;

struct RadarReportIdentity {
    pid_t pid;
    int32_t errorCode;
};

bool operator==(const RadarReportIdentity &lhs, const RadarReportIdentity &rhs)
{
    return lhs.pid == rhs.pid && lhs.errorCode == rhs.errorCode;
}

PasteboardClient::PasteboardClient()
{
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "proxyService is null");
    }
}

PasteboardClient::~PasteboardClient()
{
    if (!staticDestoryMonitor_.IsDestoryed()) {
        auto pasteboardServiceProxy = GetPasteboardServiceProxy();
        if (pasteboardServiceProxy != nullptr) {
            auto remoteObject = pasteboardServiceProxy->AsObject();
            if (remoteObject != nullptr) {
                remoteObject->RemoveDeathRecipient(deathRecipient_);
            }
        }
    }
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateHtmlTextRecord(const std::string &htmlText)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New text record");
    return PasteDataRecord::NewHtmlRecord(htmlText);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New want record");
    return PasteDataRecord::NewWantRecord(std::move(want));
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreatePlainTextRecord(const std::string &text)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New text record");
    return PasteDataRecord::NewPlainTextRecord(text);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreatePixelMapRecord(std::shared_ptr<PixelMap> pixelMap)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New pixelMap record");
    return PasteDataRecord::NewPixelMapRecord(std::move(pixelMap));
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateUriRecord(const OHOS::Uri &uri)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New uri record");
    return PasteDataRecord::NewUriRecord(uri);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateKvRecord(
    const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New kv record");
    return PasteDataRecord::NewKvRecord(mimeType, arrayBuffer);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateMultiDelayRecord(
    std::vector<std::string> mimeTypes, const std::shared_ptr<UDMF::EntryGetter> entryGetter)
{
    return PasteDataRecord::NewMultiTypeDelayRecord(mimeTypes, entryGetter);
}

std::shared_ptr<PasteData> PasteboardClient::CreateHtmlData(const std::string &htmlText)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New htmlText data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddHtmlRecord(htmlText);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateWantData(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New want data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddWantRecord(std::move(want));
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreatePlainTextData(const std::string &text)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New plain data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddTextRecord(text);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreatePixelMapData(std::shared_ptr<PixelMap> pixelMap)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New pixelMap data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddPixelMapRecord(std::move(pixelMap));
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateUriData(const OHOS::Uri &uri)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New uri data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddUriRecord(uri);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateKvData(
    const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New Kv data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddKvRecord(mimeType, arrayBuffer);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateMultiTypeData(
    std::shared_ptr<std::map<std::string, std::shared_ptr<EntryValue>>> typeValueMap, const std::string &recordMimeType)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New multiType data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddRecord(PasteDataRecord::NewMultiTypeRecord(std::move(typeValueMap), recordMimeType));
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateMultiTypeDelayData(std::vector<std::string> mimeTypes,
    std::shared_ptr<UDMF::EntryGetter> entryGetter)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New multiTypeDelay data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddRecord(PasteDataRecord::NewMultiTypeDelayRecord(mimeTypes, entryGetter));
    return pasteData;
}

int32_t PasteboardClient::GetChangeCount(uint32_t &changeCount)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetChangeCount start.");
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        changeCount = 0;
        return static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR);
    }
    return proxyService->GetChangeCount(changeCount);
}

int32_t PasteboardClient::SubscribeEntityObserver(
    EntityType entityType, uint32_t expectedDataLength, const sptr<EntityRecognitionObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT,
        "SubscribeEntityObserver start, type is %{public}u, length is %{public}u.", static_cast<uint32_t>(entityType),
        expectedDataLength);
    if (observer == nullptr) {
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr,
        static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR),
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->SubscribeEntityObserver(entityType, expectedDataLength, observer);
}

int32_t PasteboardClient::UnsubscribeEntityObserver(
    EntityType entityType, uint32_t expectedDataLength, const sptr<EntityRecognitionObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT,
        "UnsubscribeEntityObserver start, type is %{public}u, length is %{public}u.", static_cast<uint32_t>(entityType),
        expectedDataLength);
    if (observer == nullptr) {
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr,
        static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR),
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->UnsubscribeEntityObserver(entityType, expectedDataLength, observer);
}

int32_t PasteboardClient::GetRecordValueByType(uint32_t dataId, uint32_t recordId, PasteDataEntry &value)
{
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr,
        static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR),
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->GetRecordValueByType(dataId, recordId, value);
}

void PasteboardClient::Clear()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Clear start.");
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(proxyService != nullptr, PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    proxyService->Clear();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Clear end.");
    return;
}

int32_t PasteboardClient::GetPasteData(PasteData &pasteData)
{
    static DeduplicateMemory<RadarReportIdentity> reportMemory(REPORT_DUPLICATE_TIMEOUT);
    pid_t pid = getpid();
    std::string currentPid = std::to_string(pid);
    uint32_t tmpSequenceId = getSequenceId_++;
    std::string currentId = "GetPasteData_" + currentPid + "_" + std::to_string(tmpSequenceId);
    pasteData.SetPasteId(currentId);
    RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, RadarReporter::DFX_GET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN, RadarReporter::CONCURRENT_ID, currentId,
        RadarReporter::PACKAGE_NAME, currentPid);
    StartAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetPasteData", HITRACE_GETPASTEDATA);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetPasteData start.");
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, RadarReporter::DFX_CHECK_GET_SERVER, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, currentId,
            RadarReporter::PACKAGE_NAME, currentPid, RadarReporter::ERROR_CODE,
            static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR));
        return static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR);
    }
    int32_t syncTime = 0;
    int32_t ret = proxyService->GetPasteData(pasteData, syncTime);
    int32_t bizStage = (syncTime == 0) ? RadarReporter::DFX_LOCAL_PASTE_END : RadarReporter::DFX_DISTRIBUTED_PASTE_END;
    PasteboardWebController::GetInstance().RetainUri(pasteData);
    PasteboardWebController::GetInstance().RebuildWebviewPasteData(pasteData);
    FinishAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetPasteData", HITRACE_GETPASTEDATA);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetPasteData end.");
    if (ret == static_cast<int32_t>(PasteboardError::E_OK)) {
        if (pasteData.deviceId_.empty()) {
            RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_SUCCESS,
                RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, currentId,
                RadarReporter::DIS_SYNC_TIME, syncTime, RadarReporter::PACKAGE_NAME, currentPid);
        } else {
            RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_SUCCESS,
                RadarReporter::CONCURRENT_ID, currentId, RadarReporter::DIS_SYNC_TIME, syncTime,
                RadarReporter::PACKAGE_NAME, currentPid);
        }
    } else if (ret != static_cast<int32_t>(PasteboardError::TASK_PROCESSING) &&
               !reportMemory.IsDuplicate({.pid = pid, .errorCode = ret})) {
        RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_FAILED, RadarReporter::BIZ_STATE,
            RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, currentId, RadarReporter::DIS_SYNC_TIME,
            syncTime, RadarReporter::PACKAGE_NAME, currentPid, RadarReporter::ERROR_CODE, ret);
    } else {
        RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_CANCELLED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, currentId,
            RadarReporter::DIS_SYNC_TIME, syncTime, RadarReporter::PACKAGE_NAME, currentPid,
            RadarReporter::ERROR_CODE, ret);
    }
    return ret;
}

void PasteboardClient::GetProgressByProgressInfo(std::shared_ptr<GetDataParams> params)
{
    PASTEBOARD_CHECK_AND_RETURN_LOGE(params != nullptr, PASTEBOARD_MODULE_CLIENT, "params is null!");
    if (params->info == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "params->info is null!");
        return;
    }
    std::unique_lock<std::mutex> lock(instanceLock_);
    std::string progressKey = g_progressKey;
    lock.unlock();
    std::string currentValue = std::to_string(params->info->percentage);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "pasteboard progress percent = %{public}s", currentValue.c_str());
    PasteBoardProgress::GetInstance().UpdateValue(progressKey, currentValue);
}

int32_t PasteboardClient::SetProgressWithoutFile(std::string &progressKey, std::shared_ptr<GetDataParams> params)
{
    int progressValue = PASTEBOARD_PROGRESS_TWENTY_PERCENT;
    while (progressValue < PASTEBOARD_PROGRESS_FINISH_PERCENT && !remoteTask_.load()) {
        uint64_t currentTimeMicros = PasteBoardTime::GetCurrentTimeMicros();
        if (currentTimeMicros >= progressStartTime_) {
            uint64_t duration = currentTimeMicros - progressStartTime_;
            if (duration >= (HAP_PULL_UP_TIME + HAP_MIN_SHOW_TIME) || duration < HAP_PULL_UP_TIME) {
                UpdateProgress(params, PASTEBOARD_PROGRESS_FINISH_PERCENT);
                break;
            }
        }
        if (ProgressSignalClient::GetInstance().CheckCancelIfNeed()) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "progress cancel success!");
            return static_cast<int32_t>(PasteboardError::E_OK);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_WITHOUT_FILE));
        progressValue += UPDATE_PERCENT_WITHOUT_FILE;
        UpdateProgress(params, progressValue);
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardClient::ProgressSmoothToTwentyPercent(PasteData &pasteData, std::string &progressKey,
    std::shared_ptr<GetDataParams> params)
{
    if (pasteData.GetRecordCount() <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "no pasteData, progress no need to twenty");
        return;
    }
    int progressValue = 0;
    bool hasUri = (pasteData.GetPrimaryUri() != nullptr);
    while (progressValue < PASTEBOARD_PROGRESS_TWENTY_PERCENT && !remoteTask_.load()) {
        uint64_t currentTimeMicros = PasteBoardTime::GetCurrentTimeMicros();
        if (currentTimeMicros >= progressStartTime_) {
            uint64_t duration = currentTimeMicros - progressStartTime_;
            if (duration >= (HAP_PULL_UP_TIME + HAP_MIN_SHOW_TIME) || duration < HAP_PULL_UP_TIME) {
                UpdateProgress(
                    params, hasUri ? PASTEBOARD_PROGRESS_TWENTY_PERCENT : PASTEBOARD_PROGRESS_FINISH_PERCENT);
                break;
            }
        }
        if (ProgressSignalClient::GetInstance().CheckCancelIfNeed()) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(PASTEBOARD_PROGRESS_SLEEP_TIME));
        progressValue += PASTEBOARD_PROGRESS_UPDATE_PERCENT;
        UpdateProgress(params, progressValue);
    }
}

void PasteboardClient::UpdateProgress(std::shared_ptr<GetDataParams> params, int progressValue)
{
    if (params == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "params is null!");
        return;
    }
    if (params->info != nullptr) {
        params->info->percentage = progressValue;
    }
    if (params->listener.ProgressNotify != nullptr) {
        params->listener.ProgressNotify(params);
    }
}

void PasteboardClient::OnProgressAbnormal(int32_t result)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "The progress is reported abnormal.");
    remoteTask_.store(true);
}

int32_t PasteboardClient::GetPasteDataFromService(PasteData &pasteData,
    PasteDataFromServiceInfo &pasteDataFromServiceInfo, std::string progressKey, std::shared_ptr<GetDataParams> params)
{
    static DeduplicateMemory<RadarReportIdentity> reportMemory(REPORT_DUPLICATE_TIMEOUT);
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, RadarReporter::DFX_CHECK_GET_SERVER,
            RadarReporter::DFX_FAILED, RadarReporter::BIZ_STATE, RadarReporter::DFX_END,
            RadarReporter::CONCURRENT_ID, pasteDataFromServiceInfo.currentId,
            RadarReporter::PACKAGE_NAME, pasteDataFromServiceInfo.currentId, RadarReporter::ERROR_CODE,
            static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR));
        return static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR);
    }
    int32_t syncTime = 0;
    int32_t ret = proxyService->GetPasteData(pasteData, syncTime);
    ProgressSmoothToTwentyPercent(pasteData, progressKey, params);
    int32_t bizStage = (syncTime == 0) ? RadarReporter::DFX_LOCAL_PASTE_END : RadarReporter::DFX_DISTRIBUTED_PASTE_END;
    PasteboardWebController::GetInstance().RetainUri(pasteData);
    if (ret == static_cast<int32_t>(PasteboardError::E_OK)) {
        if (pasteData.deviceId_.empty()) {
            RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_SUCCESS,
                RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID,
                pasteDataFromServiceInfo.currentId, RadarReporter::DIS_SYNC_TIME, syncTime,
                RadarReporter::PACKAGE_NAME, pasteDataFromServiceInfo.currentPid);
        } else {
            RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_SUCCESS,
                RadarReporter::CONCURRENT_ID, pasteDataFromServiceInfo.currentId,
                RadarReporter::DIS_SYNC_TIME, syncTime, RadarReporter::PACKAGE_NAME,
                pasteDataFromServiceInfo.currentId);
        }
    } else if (ret != static_cast<int32_t>(PasteboardError::TASK_PROCESSING) &&
               !reportMemory.IsDuplicate({.pid = pasteDataFromServiceInfo.pid, .errorCode = ret})) {
        RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID,
            pasteDataFromServiceInfo.currentId, RadarReporter::DIS_SYNC_TIME, syncTime,
            RadarReporter::PACKAGE_NAME, pasteDataFromServiceInfo.currentPid, RadarReporter::ERROR_CODE, ret);
    } else {
        RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_CANCELLED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID,
            pasteDataFromServiceInfo.currentId, RadarReporter::DIS_SYNC_TIME, syncTime,
            RadarReporter::PACKAGE_NAME, pasteDataFromServiceInfo.currentPid, RadarReporter::ERROR_CODE, ret);
    }
    return ret;
}

void PasteboardClient::ProgressRadarReport(PasteData &pasteData, PasteDataFromServiceInfo &pasteDataFromServiceInfo)
{
    pasteDataFromServiceInfo.pid = getpid();
    pasteDataFromServiceInfo.currentPid = std::to_string(pasteDataFromServiceInfo.pid);
    uint32_t tmpSequenceId = getSequenceId_++;
    pasteDataFromServiceInfo.currentId = "GetDataWithProgress_" + pasteDataFromServiceInfo.currentPid
        + "_" + std::to_string(tmpSequenceId);
    pasteData.SetPasteId(pasteDataFromServiceInfo.currentId);
    RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, RadarReporter::DFX_GET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN, RadarReporter::CONCURRENT_ID,
        pasteDataFromServiceInfo.currentId, RadarReporter::PACKAGE_NAME,
        pasteDataFromServiceInfo.currentPid);
}

int32_t PasteboardClient::ProgressAfterTwentyPercent(PasteData &pasteData, std::shared_ptr<GetDataParams> params,
    std::string progressKey)
{
    if (pasteData.GetRecordCount() <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "no pasteData, no need progress");
        return static_cast<int32_t>(PasteboardError::NO_DATA_ERROR);
    }
    int32_t ret = 0;
    bool hasUri = (pasteData.GetPrimaryUri() != nullptr);
    if (hasUri) {
        ret = PasteBoardCopyFile::GetInstance().CopyPasteData(pasteData, params);
    } else {
        ret = SetProgressWithoutFile(progressKey, params);
    }
    return ret;
}

int32_t PasteboardClient::CheckProgressParam(std::shared_ptr<GetDataParams> params)
{
    if (params == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Invalid param!");
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    if (isPasting_) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "task copying!");
        return static_cast<int32_t>(PasteboardError::TASK_PROCESSING);
    }
    ProgressSignalClient::GetInstance().Init();
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardClient::GetDataWithProgress(PasteData &pasteData, std::shared_ptr<GetDataParams> params)
{
    int32_t ret = CheckProgressParam(params);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        return ret;
    }
    progressStartTime_ = PasteBoardTime::GetCurrentTimeMicros();
    isPasting_.store(true);
    std::string progressKey;
    std::string keyDefaultValue = "0";
    std::shared_ptr<FFRTTimer> ffrtTimer;
    ffrtTimer = std::make_shared<FFRTTimer>("pasteboard_progress");
    if (params->progressIndicator != NONE_PROGRESS_INDICATOR) {
        PasteBoardProgress::GetInstance().InsertValue(progressKey, keyDefaultValue); // 0%
        std::unique_lock<std::mutex> lock(instanceLock_);
        g_progressKey = progressKey;
        lock.unlock();
        params->listener.ProgressNotify = GetProgressByProgressInfo;
        if (ffrtTimer != nullptr) {
            FFRTTask task = [this, progressKey] {
                ShowProgress(progressKey);
            };
            ffrtTimer->SetTimer(progressKey, task, HAP_PULL_UP_TIME);
        }
    }
    PasteDataFromServiceInfo pasteDataFromServiceInfo;
    ProgressRadarReport(pasteData, pasteDataFromServiceInfo);
    StartAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetDataWithProgress", HITRACE_GETPASTEDATA);
    ret = GetPasteDataFromService(pasteData, pasteDataFromServiceInfo, progressKey, params);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "GetPasteDataFromService is failed: ret=%{public}d.", ret);
        remoteTask_.store(false);
        isPasting_.store(false);
        return ret;
    }
    FinishAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetDataWithProgress", HITRACE_GETPASTEDATA);
    ret = ProgressAfterTwentyPercent(pasteData, params, progressKey);
    PasteboardWebController::GetInstance().RebuildWebviewPasteData(pasteData);
    if (ffrtTimer != nullptr) {
        ffrtTimer->CancelTimer(progressKey);
    }
    if (remoteTask_.load()) {
        ret = static_cast<int32_t>(PasteboardError::PROGRESS_ABNORMAL);
        remoteTask_.store(false);
    }
    isPasting_.store(false);
    return ret;
}

int32_t PasteboardClient::GetUnifiedDataWithProgress(UDMF::UnifiedData &unifiedData,
    std::shared_ptr<GetDataParams> params)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetUdsdData", HITRACE_GETPASTEDATA);
    PasteData pasteData;
    int32_t ret = GetDataWithProgress(pasteData, params);
    unifiedData = *(ConvertUtils::Convert(pasteData));
    FinishAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetUdsdData", HITRACE_GETPASTEDATA);
    return ret;
}

int32_t PasteboardClient::GetUnifiedData(UDMF::UnifiedData &unifiedData)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetUnifiedData", HITRACE_GETPASTEDATA);
    PasteData pasteData;
    int32_t ret = GetPasteData(pasteData);
    unifiedData = *(PasteboardUtils::GetInstance().Convert(pasteData));
    FinishAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetUnifiedData", HITRACE_GETPASTEDATA);
    return ret;
}

int32_t PasteboardClient::GetUdsdData(UDMF::UnifiedData &unifiedData)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetUdsdData", HITRACE_GETPASTEDATA);
    PasteData pasteData;
    int32_t ret = GetPasteData(pasteData);
    unifiedData = *(ConvertUtils::Convert(pasteData));
    FinishAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetUdsdData", HITRACE_GETPASTEDATA);
    return ret;
}

bool PasteboardClient::HasPasteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "HasPasteData start.");
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr, false,
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->HasPasteData();
}

int32_t PasteboardClient::SetPasteData(PasteData &pasteData, std::shared_ptr<PasteboardDelayGetter> delayGetter,
    std::map<uint32_t, std::shared_ptr<UDMF::EntryGetter>> entryGetters)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "SetPasteData start.");
    RADAR_REPORT(RadarReporter::DFX_SET_PASTEBOARD, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN);
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        RADAR_REPORT(RadarReporter::DFX_SET_PASTEBOARD, RadarReporter::DFX_CHECK_SET_SERVER, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR));
        return static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR);
    }
    sptr<PasteboardDelayGetterClient> delayGetterAgent;
    if (delayGetter != nullptr) {
        pasteData.SetDelayData(true);
        delayGetterAgent = new (std::nothrow) PasteboardDelayGetterClient(delayGetter);
    }
    sptr<PasteboardEntryGetterClient> entryGetterAgent;
    if (!(entryGetters.empty())) {
        pasteData.SetDelayRecord(true);
        entryGetterAgent = new (std::nothrow) PasteboardEntryGetterClient(entryGetters);
    }

    auto ret = proxyService->SetPasteData(pasteData, delayGetterAgent, entryGetterAgent);
    if (ret == static_cast<int32_t>(PasteboardError::E_OK)) {
        RADAR_REPORT(RadarReporter::DFX_SET_PASTEBOARD, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END);
    } else {
        RADAR_REPORT(RadarReporter::DFX_SET_PASTEBOARD, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE, ret);
    }
    return ret;
}

int32_t PasteboardClient::SetUnifiedData(
    const UDMF::UnifiedData &unifiedData, std::shared_ptr<PasteboardDelayGetter> delayGetter)
{
    auto pasteData = PasteboardUtils::GetInstance().Convert(unifiedData);
    return SetPasteData(*pasteData, delayGetter);
}

int32_t PasteboardClient::SetUdsdData(const UDMF::UnifiedData &unifiedData)
{
    auto pasteData = ConvertUtils::Convert(unifiedData);
    std::map<uint32_t, std::shared_ptr<UDMF::EntryGetter>> entryGetters;
    for (auto record : unifiedData.GetRecords()) {
        if (record != nullptr && record->GetEntryGetter() != nullptr) {
            entryGetters.emplace(record->GetRecordId(), record->GetEntryGetter());
        }
    }
    return SetPasteData(*pasteData, nullptr, entryGetters);
}

void PasteboardClient::Subscribe(PasteboardObserverType type, sptr<PasteboardObserver> callback)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    if (callback == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "input nullptr.");
        return;
    }
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(proxyService != nullptr, PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    proxyService->SubscribeObserver(type, callback);
}

void PasteboardClient::AddPasteboardChangedObserver(sptr<PasteboardObserver> callback)
{
    Subscribe(PasteboardObserverType::OBSERVER_LOCAL, callback);
}

void PasteboardClient::AddPasteboardEventObserver(sptr<PasteboardObserver> callback)
{
    Subscribe(PasteboardObserverType::OBSERVER_EVENT, callback);
}

void PasteboardClient::Unsubscribe(PasteboardObserverType type, sptr<PasteboardObserver> callback)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(proxyService != nullptr, PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    if (callback == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "remove all.");
        proxyService->UnsubscribeAllObserver(type);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
        return;
    }
    proxyService->UnsubscribeObserver(type, callback);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardClient::RemovePasteboardChangedObserver(sptr<PasteboardObserver> callback)
{
    Unsubscribe(PasteboardObserverType::OBSERVER_LOCAL, callback);
}

void PasteboardClient::RemovePasteboardEventObserver(sptr<PasteboardObserver> callback)
{
    Unsubscribe(PasteboardObserverType::OBSERVER_EVENT, callback);
}

int32_t PasteboardClient::SetGlobalShareOption(const std::map<uint32_t, ShareOption> &globalShareOptions)
{
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr,
        static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR),
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->SetGlobalShareOption(globalShareOptions);
}

int32_t PasteboardClient::RemoveGlobalShareOption(const std::vector<uint32_t> &tokenIds)
{
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr,
        static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR),
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->RemoveGlobalShareOption(tokenIds);
}

std::map<uint32_t, ShareOption> PasteboardClient::GetGlobalShareOption(const std::vector<uint32_t> &tokenIds)
{
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr, {},
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->GetGlobalShareOption(tokenIds);
}

int32_t PasteboardClient::SetAppShareOptions(const ShareOption &shareOptions)
{
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr,
        static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR),
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->SetAppShareOptions(shareOptions);
}

int32_t PasteboardClient::RemoveAppShareOptions()
{
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr,
        static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR),
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->RemoveAppShareOptions();
}

bool PasteboardClient::IsRemoteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "IsRemoteData start.");
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr, false,
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    auto ret = proxyService->IsRemoteData();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "IsRemoteData end.");
    return ret;
}

int32_t PasteboardClient::GetDataSource(std::string &bundleName)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetDataSource start.");
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr,
        static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR),
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    int32_t ret = proxyService->GetDataSource(bundleName);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetDataSource end.");
    return ret;
}

std::vector<std::string> PasteboardClient::GetMimeTypes()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetMimeTypes start.");
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr, {},
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->GetMimeTypes();
}

bool PasteboardClient::HasDataType(const std::string &mimeType)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "HasDataType start.");
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr, false,
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(!mimeType.empty(), false, PASTEBOARD_MODULE_CLIENT, "parameter is invalid");
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "type is %{public}s", mimeType.c_str());
    bool ret = proxyService->HasDataType(mimeType);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "HasDataType end.");
    return ret;
}

std::set<Pattern> PasteboardClient::DetectPatterns(const std::set<Pattern> &patternsToCheck)
{
    if (!PatternDetection::IsValid(patternsToCheck)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Invalid number in Pattern set!");
        return {};
    }

    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr, {},
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->DetectPatterns(patternsToCheck);
}

sptr<IPasteboardService> PasteboardClient::GetPasteboardService()
{
    std::unique_lock<std::mutex> lock(instanceLock_);
    if (pasteboardServiceProxy_ != nullptr) {
        return pasteboardServiceProxy_;
    }
    if (constructing_) {
        auto waitStatus = proxyConVar_.wait_for(lock, std::chrono::milliseconds(LOADSA_TIMEOUT_MS), [this]() {
            return pasteboardServiceProxy_ != nullptr;
        });
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGI(waitStatus, nullptr, PASTEBOARD_MODULE_CLIENT, "Load SA timeout");
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Getting PasteboardServiceProxy succeeded.");
        return pasteboardServiceProxy_;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "GetPasteboardService start.");
    sptr<ISystemAbilityManager> samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Get SystemAbilityManager failed.");
        pasteboardServiceProxy_ = nullptr;
        return nullptr;
    }
    sptr<IRemoteObject> remoteObject = samgrProxy->CheckSystemAbility(PASTEBOARD_SERVICE_ID);
    if (remoteObject != nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Get PasteboardServiceProxy succeed.");
        SetPasteboardServiceProxy(remoteObject);
        return pasteboardServiceProxy_;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "remoteObject is null.");
    sptr<PasteboardLoadCallback> loadCallback = new PasteboardLoadCallback();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGI(loadCallback != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "loadCb is null");
    int32_t ret = samgrProxy->LoadSystemAbility(PASTEBOARD_SERVICE_ID, loadCallback);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGI(ret == ERR_OK, nullptr, PASTEBOARD_MODULE_CLIENT, "Failed to load SA");
    constructing_ = true;
    auto waitStatus = proxyConVar_.wait_for(lock, std::chrono::milliseconds(LOADSA_TIMEOUT_MS), [this]() {
        return pasteboardServiceProxy_ != nullptr;
    });
    constructing_ = false;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGI(waitStatus, nullptr, PASTEBOARD_MODULE_CLIENT, "Load SA timeout");
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Getting PasteboardServiceProxy succeeded.");
    return pasteboardServiceProxy_;
}

void PasteboardClient::SetPasteboardServiceProxy(const sptr<IRemoteObject> &remoteObject)
{
    PASTEBOARD_CHECK_AND_RETURN_LOGE(remoteObject != nullptr, PASTEBOARD_MODULE_CLIENT, "remoteObject is null");
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new PasteboardSaDeathRecipient());
    }
    remoteObject->AddDeathRecipient(deathRecipient_);
    pasteboardServiceProxy_ = iface_cast<IPasteboardService>(remoteObject);
    if (clientDeathObserverPtr_ == nullptr) {
        clientDeathObserverPtr_ = new (std::nothrow) PasteboardClientDeathObserverStub();
    }
    PASTEBOARD_CHECK_AND_RETURN_LOGE(
        clientDeathObserverPtr_ != nullptr, PASTEBOARD_MODULE_CLIENT, "clientDeathObserverPtr_ is null");
    auto ret = pasteboardServiceProxy_->RegisterClientDeathObserver(clientDeathObserverPtr_);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(ret == ERR_OK, PASTEBOARD_MODULE_CLIENT, "RegisterClientDeathObserver failed");
}

sptr<IPasteboardService> PasteboardClient::GetPasteboardServiceProxy()
{
    std::lock_guard<std::mutex> lock(instanceLock_);
    return pasteboardServiceProxy_;
}

void PasteboardClient::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(instanceLock_);
    SetPasteboardServiceProxy(remoteObject);
    proxyConVar_.notify_all();
}

void PasteboardClient::LoadSystemAbilityFail()
{
    std::lock_guard<std::mutex> lock(instanceLock_);
    pasteboardServiceProxy_ = nullptr;
    proxyConVar_.notify_all();
}

void PasteboardClient::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "OnRemoteSaDied start.");
    std::lock_guard<std::mutex> lock(instanceLock_);
    pasteboardServiceProxy_ = nullptr;
}

void PasteboardClient::PasteStart(const std::string &pasteId)
{
    RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, RadarReporter::DFX_DISTRIBUTED_FILE_START,
        RadarReporter::DFX_SUCCESS, RadarReporter::CONCURRENT_ID, pasteId);
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(proxyService != nullptr, PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    proxyService->PasteStart(pasteId);
}

void PasteboardClient::PasteComplete(const std::string &deviceId, const std::string &pasteId)
{
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(proxyService != nullptr, PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    proxyService->PasteComplete(deviceId, pasteId);
}

int32_t PasteboardClient::GetRemoteDeviceName(std::string &deviceName, bool &isRemote)
{
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(proxyService != nullptr,
        static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR),
        PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    return proxyService->GetRemoteDeviceName(deviceName, isRemote);
}

int32_t PasteboardClient::HandleSignalValue(const std::string &signalValue)
{
    int32_t progressStatusValue = 0;
    std::shared_ptr<ProgressReportLintener> progressReport = std::make_shared<ProgressReportLintener>();
    progressReport->OnProgressFail = OnProgressAbnormal;
    try {
        progressStatusValue = std::stoi(signalValue);
    } catch (const std::invalid_argument &e) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT,
            "progressStatusValue invalid = %{public}s", e.what());
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    } catch (const std::out_of_range &e) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT,
            "progressStatusValue out of range = %{public}s", e.what());
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    if (progressStatusValue == NORMAL_PASTE) {
        return static_cast<int32_t>(PasteboardError::E_OK);
    }
    if (progressStatusValue == CANCEL_PASTE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "progress cancel paste");
        ProgressSignalClient::GetInstance().Cancel();
    } else if (progressStatusValue == PASTE_TIME_OUT) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "pasteboard progress time out");
        progressReport->OnProgressFail(static_cast<int32_t>(PasteboardError::PROGRESS_PASTE_TIME_OUT));
    } else {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "pasteboard progress invalid status");
        progressReport->OnProgressFail(static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR));
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardClient::ShowProgress(const std::string &progressKey)
{
    auto proxyService = GetPasteboardService();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(proxyService != nullptr, PASTEBOARD_MODULE_CLIENT, "proxyService is nullptr");
    sptr<PasteboardSignalCallback> callback = new PasteboardSignalCallback();
    proxyService->ShowProgress(progressKey, callback);
}

PasteboardSaDeathRecipient::PasteboardSaDeathRecipient() {}

void PasteboardSaDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "PasteboardSaDeathRecipient on remote systemAbility died.");
    PasteboardClient::GetInstance()->OnRemoteSaDied(object);
}
} // namespace MiscServices
} // namespace OHOS
