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

#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <chrono>

#include "file_uri.h"
#include "hiview_adapter.h"
#include "hitrace_meter.h"
#include "pasteboard_client.h"
#include "pasteboard_delay_getter_client.h"
#include "pasteboard_error.h"
#include "pasteboard_event_dfx.h"
#include "pasteboard_load_callback.h"
#include "pasteboard_observer.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "pasteboard_web_controller.h"
#include "pasteboard_utils.h"
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
constexpr const int32_t HITRACE_GETPASTEDATA = 0;
constexpr int32_t LOADSA_TIMEOUT_MS = 10000;
const std::map<int32_t, int32_t> ERROR_CODE_COVERT_TABLE = {
    {static_cast<int32_t>(PasteboardError::E_INVALID_VALUE), RadarReporter::INVALID_DATA_ERROR},
    {static_cast<int32_t>(PasteboardError::E_INVALID_OPTION), RadarReporter::OTHER_ERROR},
    {static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR), RadarReporter::SERIALIZATION_ERROR},
    {static_cast<int32_t>(PasteboardError::E_READ_PARCEL_ERROR), RadarReporter::DESERIALIZATION_ERROR},
    {static_cast<int32_t>(PasteboardError::E_SA_DIED), RadarReporter::OBTAIN_SERVER_SA_ERROR},
    {static_cast<int32_t>(PasteboardError::E_ERROR), RadarReporter::OTHER_ERROR},
    {static_cast<int32_t>(PasteboardError::E_OUT_OF_RANGE), RadarReporter::EXCEEDING_LIMIT_EXCEPTION},
    {static_cast<int32_t>(PasteboardError::E_NO_PERMISSION), RadarReporter::PERMISSION_VERIFICATION_ERROR},
    {static_cast<int32_t>(PasteboardError::E_INVALID_PARAMETERS), RadarReporter::INVALID_PARAM_ERROR},
    {static_cast<int32_t>(PasteboardError::E_TIMEOUT), RadarReporter::TIMEOUT_ERROR},
    {static_cast<int32_t>(PasteboardError::E_CANCELED), RadarReporter::CANCELED},
    {static_cast<int32_t>(PasteboardError::E_EXCEEDS_LIMIT), RadarReporter::EXCEEDING_LIMIT_EXCEPTION},
    {static_cast<int32_t>(PasteboardError::E_IS_BEGING_PROCESSED), RadarReporter::TASK_PROCESSING},
    {static_cast<int32_t>(PasteboardError::E_COPY_FORBIDDEN), RadarReporter::PROHIBIT_COPY},
    {static_cast<int32_t>(PasteboardError::E_UNKNOWN), RadarReporter::UNKNOWN_ERROR},
    {static_cast<int32_t>(PasteboardError::E_BUTT), RadarReporter::OTHER_ERROR},
    {static_cast<int32_t>(PasteboardError::E_REMOTE), RadarReporter::REMOTE_EXCEPTION},
    {static_cast<int32_t>(PasteboardError::E_INVALID_OPERATION), RadarReporter::OTHER_ERROR},
    {static_cast<int32_t>(PasteboardError::E_NO_DATA), RadarReporter::NO_DATA_ERROR},
    {static_cast<int32_t>(PasteboardError::E_INVALID_USERID), RadarReporter::INVALID_USERID_ERROR},
    {static_cast<int32_t>(PasteboardError::E_REMOTE_TASK), RadarReporter::REMOTE_TASK_ERROR},
    {static_cast<int32_t>(PasteboardError::E_INVALID_EVENT), RadarReporter::INVALID_EVENT_ERROR},
    {static_cast<int32_t>(PasteboardError::E_GET_REMOTE_DATA), RadarReporter::GET_REMOTE_DATA_ERROR},
    {ERR_INVALID_VALUE, RadarReporter::INVALID_RETURN_VALUE_ERROR},
    {ERR_INVALID_OPERATION, RadarReporter::INVALID_RETURN_VALUE_ERROR},
};
sptr<IPasteboardService> PasteboardClient::pasteboardServiceProxy_;
PasteboardClient::StaticDestoryMonitor PasteboardClient::staticDestoryMonitor_;
std::mutex PasteboardClient::instanceLock_;
std::condition_variable PasteboardClient::proxyConVar_;
PasteboardClient::PasteboardClient(){};
PasteboardClient::~PasteboardClient()
{
    auto proxyService = GetPasteboardService();
    if (proxyService != nullptr && !staticDestoryMonitor_.IsDestoryed()) {
        auto remoteObject = proxyService->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
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
    return PasteDataRecord::NewPlaintTextRecord(text);
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

void PasteboardClient::Clear()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Clear start.");
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return;
    }
    proxyService->Clear();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Clear end.");
    return;
}

int32_t PasteboardClient::GetPasteData(PasteData &pasteData)
{
    std::string currentId = "GetPasteData_" + std::to_string(getpid()) + "_" + std::to_string(getSequenceId_);
    ++getSequenceId_;
    RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, RadarReporter::DFX_GET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN, RadarReporter::CONCURRENT_ID, currentId);
    StartAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetPasteData", HITRACE_GETPASTEDATA);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetPasteData start.");
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, RadarReporter::DFX_CHECK_GET_SERVER, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, currentId,
            RadarReporter::ERROR_CODE, RadarReporter::OBTAIN_SERVER_SA_ERROR);
        return static_cast<int32_t>(PasteboardError::E_SA_DIED);
    }
    int32_t syncTime = 0;
    int32_t ret = proxyService->GetPasteData(pasteData, syncTime);
    int32_t bizStage = (syncTime == 0) ? RadarReporter::DFX_LOCAL_PASTE_END : RadarReporter::DFX_DISTRIBUTED_PASTE_END;
    RetainUri(pasteData);
    RebuildWebviewPasteData(pasteData);
    FinishAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetPasteData", HITRACE_GETPASTEDATA);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetPasteData end.");
    if (ret == static_cast<int32_t>(PasteboardError::E_OK)) {
        RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_SUCCESS,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, currentId,
            RadarReporter::DIS_SYNC_TIME, syncTime);
    } else {
        int32_t errorCode = RadarReporter::OTHER_ERROR;
        auto operatorIter = ERROR_CODE_COVERT_TABLE.find(ret);
        if (operatorIter != ERROR_CODE_COVERT_TABLE.end()) {
            errorCode = operatorIter->second;
        }
        RADAR_REPORT(RadarReporter::DFX_GET_PASTEBOARD, bizStage, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::CONCURRENT_ID, currentId,
            RadarReporter::DIS_SYNC_TIME, syncTime, RadarReporter::ERROR_CODE, errorCode);
    }
    return ret;
}

int32_t PasteboardClient::GetUnifiedData(UDMF::UnifiedData& unifiedData)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetUnifiedData", HITRACE_GETPASTEDATA);
    PasteData pasteData;
    int32_t ret = GetPasteData(pasteData);
    unifiedData = *(PasteboardUtils::GetInstance().Convert(pasteData));
    FinishAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetUnifiedData", HITRACE_GETPASTEDATA);
    return ret;
}

void PasteboardClient::RebuildWebviewPasteData(PasteData &pasteData)
{
    if (pasteData.GetTag() != PasteData::WEBVIEW_PASTEDATA_TAG || pasteData.GetPrimaryHtml() == nullptr) {
        return;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Rebuild webview PasteData start.");
    for (auto& item : pasteData.AllRecords()) {
        if (item->GetUri() == nullptr) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Rebuild webview one of uri is null.");
            continue;
        }
        std::shared_ptr<Uri> uri = item->GetUri();
        std::string puri = uri->ToString();
        std::string realUri = puri;
        if (puri.substr(0, PasteData::FILE_SCHEME_PREFIX.size()) == PasteData::FILE_SCHEME_PREFIX) {
            AppFileService::ModuleFileUri::FileUri fileUri(puri);
            realUri = PasteData::FILE_SCHEME_PREFIX + fileUri.GetRealPath();
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Rebuild webview uri is file uri.");
        }
        if (realUri.find(PasteData::DISTRIBUTEDFILES_TAG) != std::string::npos) {
            item->SetConvertUri(realUri);
        } else {
            item->SetUri(std::make_shared<OHOS::Uri>(realUri));
        }
    }
    auto PasteboardWebController = PasteboardWebController::GetInstance();
    auto webData = std::make_shared<PasteData>(pasteData);
    PasteboardWebController.RebuildHtml(webData);

    std::shared_ptr<std::string> primaryText = pasteData.GetPrimaryText();
    std::shared_ptr<std::string> html = webData->GetPrimaryHtml();
    std::string mimeType = MIMETYPE_TEXT_HTML;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_HTML);
    std::shared_ptr<PasteDataRecord> pasteDataRecord =
        builder.SetMimeType(mimeType).SetPlainText(primaryText).SetHtmlText(html).Build();
    webData->AddRecord(pasteDataRecord);
    std::size_t recordCnt = webData->GetRecordCount();
    if (recordCnt >= 1) {
        webData->RemoveRecordAt(recordCnt - 1);
    }
    pasteData = *webData;

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Rebuild webview PasteData end.");
}

void PasteboardClient::RetainUri(PasteData &pasteData)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "RetainUri start.");
    if (!pasteData.IsLocalPaste()) {
        return;
    }
    // clear convert uri
    for (size_t i = 0; i < pasteData.GetRecordCount(); ++i) {
        auto record = pasteData.GetRecordAt(i);
        if (record != nullptr) {
            record->SetConvertUri("");
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "RetainUri end.");
}

bool PasteboardClient::HasPasteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "HasPasteData start.");
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return false;
    }
    return proxyService->HasPasteData();
}

int32_t PasteboardClient::SetPasteData(PasteData &pasteData, std::shared_ptr<PasteboardDelayGetter> delayGetter)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "SetPasteData start.");
    RADAR_REPORT(RadarReporter::DFX_SET_PASTEBOARD, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_BEGIN);
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        RADAR_REPORT(RadarReporter::DFX_SET_PASTEBOARD, RadarReporter::DFX_CHECK_SET_SERVER, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::OBTAIN_SERVER_SA_ERROR);
        return static_cast<int32_t>(PasteboardError::E_SA_DIED);
    }
    sptr<PasteboardDelayGetterClient> delayGetterAgent;
    if (delayGetter != nullptr) {
        pasteData.SetDelayData(true);
        delayGetterAgent = new (std::nothrow) PasteboardDelayGetterClient(delayGetter);
    }
    std::shared_ptr<std::string> html = pasteData.GetPrimaryHtml();
    if (pasteData.GetTag() != PasteData::WEBVIEW_PASTEDATA_TAG || html == nullptr) {
        auto noHtmlRet = proxyService->SetPasteData(pasteData, delayGetterAgent);
        return noHtmlRet;
    }
    auto webData = SplitWebviewPasteData(pasteData);
    if (webData == nullptr) {
        return static_cast<int32_t>(PasteboardError::E_INVALID_VALUE);
    }
    auto ret = proxyService->SetPasteData(*webData, delayGetterAgent);
    if (ret == static_cast<int32_t>(PasteboardError::E_OK)) {
        RADAR_REPORT(RadarReporter::DFX_SET_PASTEBOARD, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END);
    } else {
        int32_t errorCode = RadarReporter::OTHER_ERROR;
        auto operatorIter = ERROR_CODE_COVERT_TABLE.find(ret);
        if (operatorIter != ERROR_CODE_COVERT_TABLE.end()) {
            errorCode = operatorIter->second;
        }
        RADAR_REPORT(RadarReporter::DFX_SET_PASTEBOARD, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_SUCCESS,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE, errorCode);
    }
    return ret;
}

int32_t PasteboardClient::SetUnifiedData(const UDMF::UnifiedData &unifiedData,
    std::shared_ptr<PasteboardDelayGetter> delayGetter)
{
    auto pasteData = PasteboardUtils::GetInstance().Convert(unifiedData);
    return SetPasteData(*pasteData, delayGetter);
}

std::shared_ptr<PasteData> PasteboardClient::SplitWebviewPasteData(PasteData &pasteData)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "SplitWebviewPasteData start.");
    std::shared_ptr<std::string> html = pasteData.GetPrimaryHtml();
    std::shared_ptr<std::string> primaryText = pasteData.GetPrimaryText();
    auto PasteboardWebController = PasteboardWebController::GetInstance();
    std::shared_ptr<PasteData> webPasteData = PasteboardWebController.SplitHtml(html);
    webPasteData->SetProperty(pasteData.GetProperty());
    std::string mimeType = MIMETYPE_TEXT_HTML;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_HTML);
    std::shared_ptr<PasteDataRecord> pasteDataRecord =
        builder.SetMimeType(mimeType).SetPlainText(primaryText).SetHtmlText(html).Build();
    webPasteData->AddRecord(pasteDataRecord);
    std::size_t recordCnt = webPasteData->GetRecordCount();
    if (recordCnt >= 1) {
        webPasteData->RemoveRecordAt(recordCnt - 1);
    }
    webPasteData->SetTag(PasteData::WEBVIEW_PASTEDATA_TAG);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "SplitWebviewPasteData end.");
    return webPasteData;
}

void PasteboardClient::Subscribe(PasteboardObserverType type, sptr<PasteboardObserver> callback)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    if (callback == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "input nullptr.");
        return;
    }
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return;
    }
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
    if (proxyService == nullptr) {
        return;
    }
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
    if (proxyService == nullptr) {
        return static_cast<int32_t>(PasteboardError::E_SA_DIED);
    }
    return proxyService->SetGlobalShareOption(globalShareOptions);
}

int32_t PasteboardClient::RemoveGlobalShareOption(const std::vector<uint32_t> &tokenIds)
{
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return static_cast<int32_t>(PasteboardError::E_SA_DIED);
    }
    return proxyService->RemoveGlobalShareOption(tokenIds);
}

std::map<uint32_t, ShareOption> PasteboardClient::GetGlobalShareOption(const std::vector<uint32_t> &tokenIds)
{
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return {};
    }
    return proxyService->GetGlobalShareOption(tokenIds);
}

int32_t PasteboardClient::SetAppShareOptions(const ShareOption &shareOptions)
{
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return static_cast<int32_t>(PasteboardError::E_SA_DIED);
    }
    return proxyService->SetAppShareOptions(shareOptions);
}

int32_t PasteboardClient::RemoveAppShareOptions()
{
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return static_cast<int32_t>(PasteboardError::E_SA_DIED);
    }
    return proxyService->RemoveAppShareOptions();
}

bool PasteboardClient::IsRemoteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "IsRemoteData start.");
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return false;
    }
    auto ret = proxyService->IsRemoteData();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "IsRemoteData end.");
    return ret;
}

int32_t PasteboardClient::GetDataSource(std::string &bundleName)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetDataSource start.");
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return static_cast<int32_t>(PasteboardError::E_SA_DIED);
    }
    int32_t ret = proxyService->GetDataSource(bundleName);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "GetDataSource end.");
    return ret;
}

bool PasteboardClient::HasDataType(const std::string &mimeType)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "HasDataType start.");
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return false;
    }
    if (mimeType.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "parameter is invalid");
        return false;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "type is %{public}s", mimeType.c_str());
    bool ret = proxyService->HasDataType(mimeType);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "HasDataType end.");
    return ret;
}

sptr<IPasteboardService> PasteboardClient::GetPasteboardService()
{
    std::unique_lock<std::mutex> lock(instanceLock_);
    if (pasteboardServiceProxy_ != nullptr) {
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
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Get PasteboardServiceProxy succeed.");
        if (deathRecipient_ == nullptr) {
            deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new PasteboardSaDeathRecipient());
        }
        remoteObject->AddDeathRecipient(deathRecipient_);
        pasteboardServiceProxy_ = iface_cast<IPasteboardService>(remoteObject);
        return pasteboardServiceProxy_;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "remoteObject is null.");
    sptr<PasteboardLoadCallback> loadCallback = new PasteboardLoadCallback();
    if (loadCallback == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "loadCallback is nullptr.");
        return nullptr;
    }
    int32_t ret = samgrProxy->LoadSystemAbility(PASTEBOARD_SERVICE_ID, loadCallback);
    if (ret != ERR_OK) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Failed to load systemAbility.");
        return nullptr;
    }
    auto waitStatus = proxyConVar_.wait_for(lock, std::chrono::milliseconds(LOADSA_TIMEOUT_MS),
        [this]() { return pasteboardServiceProxy_ != nullptr; });
    if (!waitStatus) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Load systemAbility timeout.");
        return nullptr;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Getting PasteboardServiceProxy succeeded.");
    return pasteboardServiceProxy_;
}

void PasteboardClient::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(instanceLock_);
    if (deathRecipient_ == nullptr) {
        deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new PasteboardSaDeathRecipient());
    }
    if (remoteObject != nullptr) {
        remoteObject->AddDeathRecipient(deathRecipient_);
        pasteboardServiceProxy_ = iface_cast<IPasteboardService>(remoteObject);
    }
    proxyConVar_.notify_one();
}

void PasteboardClient::LoadSystemAbilityFail()
{
    std::lock_guard<std::mutex> lock(instanceLock_);
    pasteboardServiceProxy_ = nullptr;
    proxyConVar_.notify_one();
}

void PasteboardClient::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "OnRemoteSaDied start.");
    std::lock_guard<std::mutex> lock(instanceLock_);
    pasteboardServiceProxy_ = nullptr;
}

void PasteboardClient::PasteStart()
{
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return false;
    }
    proxyService->PasteStart();
}

void PasteboardClient::PasteComplete(std::string deviceId)
{
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return false;
    }
    proxyService->PasteComplete(deviceId);
}

PasteboardSaDeathRecipient::PasteboardSaDeathRecipient()
{
}

void PasteboardSaDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "PasteboardSaDeathRecipient on remote systemAbility died.");
    PasteboardClient::GetInstance()->OnRemoteSaDied(object);
}
} // namespace MiscServices
} // namespace OHOS
