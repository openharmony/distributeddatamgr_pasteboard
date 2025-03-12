/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <iservice_registry.h>

#include "ipasteboard_client_death_observer.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_service_loader.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace MiscServices {

constexpr int32_t LOADSA_TIMEOUT_MS = 4000;
sptr<IPasteboardService> PasteboardServiceLoader::pasteboardServiceProxy_;
std::condition_variable PasteboardServiceLoader::proxyConVar_;
PasteboardServiceLoader::StaticDestroyMonitor PasteboardServiceLoader::staticDestroyMonitor_;
sptr<IRemoteObject> clientDeathObserverPtr_;
std::mutex PasteboardServiceLoader::instanceLock_;

PasteboardServiceLoader::PasteboardServiceLoader()
{
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "proxyService is null");
    }
}

PasteboardServiceLoader::~PasteboardServiceLoader()
{
    if (!staticDestroyMonitor_.IsDestoryed()) {
        auto pasteboardServiceProxy = GetPasteboardServiceProxy();
        if (pasteboardServiceProxy != nullptr) {
            auto remoteObject = pasteboardServiceProxy->AsObject();
            if (remoteObject != nullptr) {
                remoteObject->RemoveDeathRecipient(deathRecipient_);
            }
        }
    }
}

PasteboardServiceLoader &PasteboardServiceLoader::GetInstance()
{
    static PasteboardServiceLoader serviceLoader;
    return serviceLoader;
}

sptr<IPasteboardService> PasteboardServiceLoader::GetPasteboardService()
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

sptr<IPasteboardService> PasteboardServiceLoader::GetPasteboardServiceProxy()
{
    std::lock_guard<std::mutex> lock(instanceLock_);
    return pasteboardServiceProxy_;
}

void PasteboardServiceLoader::SetPasteboardServiceProxy(const sptr<IRemoteObject> &remoteObject)
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

int32_t PasteboardServiceLoader::GetRecordValueByType(uint32_t dataId, uint32_t recordId, PasteDataEntry &value)
{
    auto proxyService = GetPasteboardService();
    if (proxyService == nullptr) {
        return static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR);
    }
    return proxyService->GetRecordValueByType(dataId, recordId, value);
}

void PasteboardServiceLoader::LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject)
{
    std::lock_guard<std::mutex> lock(instanceLock_);
    SetPasteboardServiceProxy(remoteObject);
    proxyConVar_.notify_all();
}

void PasteboardServiceLoader::LoadSystemAbilityFail()
{
    std::lock_guard<std::mutex> lock(instanceLock_);
    pasteboardServiceProxy_ = nullptr;
    proxyConVar_.notify_all();
}

void PasteboardServiceLoader::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "OnRemoteSaDied start.");
    std::lock_guard<std::mutex> lock(instanceLock_);
    pasteboardServiceProxy_ = nullptr;
}

PasteboardSaDeathRecipient::PasteboardSaDeathRecipient() {}

void PasteboardSaDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &object)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "PasteboardSaDeathRecipient on remote systemAbility died.");
    PasteboardServiceLoader::GetInstance().OnRemoteSaDied(object);
}
} // namespace MiscServices
} // namespace OHOS