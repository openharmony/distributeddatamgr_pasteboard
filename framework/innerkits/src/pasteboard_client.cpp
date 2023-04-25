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
#include "pasteboard_client.h"

#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>

#include "hitrace_meter.h"

#include "pasteboard_observer.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "pasteboard_observer.h"
#include "pasteboard_error.h"
#include "pasteboard_client.h"

using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
constexpr const int32_t HITRACE_GETPASTEDATA = 0;
sptr<IPasteboardService> PasteboardClient::pasteboardServiceProxy_;
PasteboardClient::StaticDestoryMonitor PasteboardClient::staticDestoryMonitor_;
std::mutex PasteboardClient::instanceLock_;
PasteboardClient::PasteboardClient(){};
PasteboardClient::~PasteboardClient()
{
    if (pasteboardServiceProxy_ != nullptr && !staticDestoryMonitor_.IsDestoryed()) {
        auto remoteObject = pasteboardServiceProxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateHtmlTextRecord(const std::string &htmlText)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New text record");
    return PasteDataRecord::NewHtmlRecord(htmlText);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New want record");
    return PasteDataRecord::NewWantRecord(std::move(want));
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreatePlainTextRecord(const std::string &text)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New text record");
    return PasteDataRecord::NewPlaintTextRecord(text);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreatePixelMapRecord(std::shared_ptr<PixelMap> pixelMap)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New pixelMap record");
    return PasteDataRecord::NewPixelMapRecord(std::move(pixelMap));
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateUriRecord(const OHOS::Uri &uri)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New uri record");
    return PasteDataRecord::NewUriRecord(uri);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateKvRecord(
    const std::string &mimeType, const std::vector<uint8_t>& arrayBuffer)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New kv record");
    return PasteDataRecord::NewKvRecord(mimeType, arrayBuffer);
}

std::shared_ptr<PasteData> PasteboardClient::CreateHtmlData(const std::string &htmlText)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New htmlText data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddHtmlRecord(htmlText);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateWantData(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New want data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddWantRecord(std::move(want));
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreatePlainTextData(const std::string &text)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New plain data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddTextRecord(text);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreatePixelMapData(std::shared_ptr<PixelMap> pixelMap)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New pixelMap data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddPixelMapRecord(std::move(pixelMap));
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateUriData(const OHOS::Uri &uri)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New uri data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddUriRecord(uri);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateKvData(
    const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "New Kv data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddKvRecord(mimeType, arrayBuffer);
    return pasteData;
}

void PasteboardClient::Clear()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Clear start.");
    if (!IsServiceAvailable()) {
        return;
    }
    pasteboardServiceProxy_->Clear();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "end.");
    return;
}

int32_t PasteboardClient::GetPasteData(PasteData& pasteData)
{
    StartAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetPasteData", HITRACE_GETPASTEDATA);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "GetPasteData start.");
    if (!IsServiceAvailable()) {
        return static_cast<int32_t>(PasteboardError::E_SA_DIED);
    }
    int32_t ret = pasteboardServiceProxy_->GetPasteData(pasteData);
    RetainUri(pasteData);
    FinishAsyncTrace(HITRACE_TAG_MISC, "PasteboardClient::GetPasteData", HITRACE_GETPASTEDATA);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "end.");
    return ret;
}
void PasteboardClient::RetainUri(PasteData &pasteData)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start");
    if (!pasteData.IsLocalPaste()) {
        return;
    }
    // clear convert uri
    for (size_t i = 0; i < pasteData.GetRecordCount(); ++i) {
        auto record = pasteData.GetRecordAt(0);
        if (record != nullptr) {
            record->SetConvertUri("");
        }
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "end");
}

bool PasteboardClient::HasPasteData()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "HasPasteData start.");
    if (!IsServiceAvailable()) {
        return false;
    }
    return pasteboardServiceProxy_->HasPasteData();
}

int32_t PasteboardClient::SetPasteData(PasteData &pasteData)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "SetPasteData start.");
    if (!IsServiceAvailable()) {
        return static_cast<int32_t>(PasteboardError::E_SA_DIED);
    }
    return pasteboardServiceProxy_->SetPasteData(pasteData);
}

void PasteboardClient::AddPasteboardChangedObserver(sptr<PasteboardObserver> callback)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start.");
    if (callback == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "input nullptr.");
        return;
    }
    if (!IsServiceAvailable()) {
        return;
    }
    pasteboardServiceProxy_->AddPasteboardChangedObserver(callback);
    return;
}

void PasteboardClient::AddPasteboardEventObserver(sptr<PasteboardObserver> callback)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start.");
    if (callback == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "input nullptr.");
        return;
    }
    if (!IsServiceAvailable()) {
        return;
    }
    pasteboardServiceProxy_->AddPasteboardEventObserver(callback);
}

void PasteboardClient::RemovePasteboardChangedObserver(sptr<PasteboardObserver> callback)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start.");
    if (!IsServiceAvailable()) {
        return;
    }
    if (callback == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "remove all.");
        pasteboardServiceProxy_->RemoveAllChangedObserver();
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
        return;
    }
    pasteboardServiceProxy_->RemovePasteboardChangedObserver(callback);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return;
}

void PasteboardClient::RemovePasteboardEventObserver(sptr<PasteboardObserver> callback)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    if (!IsServiceAvailable()) {
        return;
    }
    if (callback == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "remove all.");
        pasteboardServiceProxy_->RemoveAllEventObserver();
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
        return;
    }
    pasteboardServiceProxy_->RemovePasteboardEventObserver(callback);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

inline bool PasteboardClient::IsServiceAvailable() {
    if (pasteboardServiceProxy_ == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "Redo ConnectService");
        ConnectService();
    }

    if (pasteboardServiceProxy_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Service proxy null.");
        return false;
    }
    return true;
}

void PasteboardClient::ConnectService()
{
    std::lock_guard<std::mutex> lock(instanceLock_);
    if (pasteboardServiceProxy_ != nullptr) {
        return ;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start.");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Getting SystemAbilityManager failed.");
        pasteboardServiceProxy_ = nullptr;
        return;
    }
    sptr<IRemoteObject> remoteObject = sam->CheckSystemAbility(PASTEBOARD_SERVICE_ID);
    if (remoteObject == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "GetSystemAbility failed!");
        return;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new PasteboardSaDeathRecipient());
    if (deathRecipient_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Getting deathRecipient_ failed.");
        return;
    }
    if ((remoteObject->IsProxyObject()) && (!remoteObject->AddDeathRecipient(deathRecipient_))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Add death recipient to paste service failed.");
        return;
    }

    pasteboardServiceProxy_ = iface_cast<IPasteboardService>(remoteObject);
    if (pasteboardServiceProxy_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Get PasteboardServiceProxy from SA failed.");
        return ;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Getting PasteboardServiceProxy succeeded.");
}

void PasteboardClient::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start.");
    ConnectService();
}

PasteboardSaDeathRecipient::PasteboardSaDeathRecipient()
{
}

void PasteboardSaDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "PasteboardSaDeathRecipient on remote systemAbility died.");
    PasteboardClient::GetInstance()->OnRemoteSaDied(object);
}
} // MiscServices
} // OHOS
