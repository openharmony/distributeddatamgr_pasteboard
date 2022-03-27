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
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "pasteboard_common.h"
#include "pasteboard_service.h"


namespace OHOS {
namespace MiscServices {
namespace {
const std::int32_t INIT_INTERVAL = 10000L;
const std::string PASTEBOARD_SERVICE_NAME = "PasteboardService";
const bool G_REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<PasteboardService>::GetInstance().get());
}

PasteboardService::PasteboardService()
    : SystemAbility(PASTEBOARD_SERVICE_ID, true),
      state_(ServiceRunningState::STATE_NOT_START)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "PasteboardService Start.");
}

PasteboardService::~PasteboardService() {}

int32_t PasteboardService::Init()
{
    if (!Publish(DelayedSingleton<PasteboardService>::GetInstance().get())) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OnStart register to system ability manager failed.");
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
    if (Init() != ERR_OK) {
        auto callback = [=]() { Init(); };
        serviceHandler_->PostTask(callback, INIT_INTERVAL);
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Init failed. Try again 10s later.");
        return;
    }

    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "Start PasteboardService success.");
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
    std::lock_guard<std::mutex> lock(clipMutex_);
    auto it = clips_.find(userId);
    if (it != clips_.end()) {
        clips_.erase(it);
        NotifyObservers();
    }
}

bool PasteboardService::GetPasteData(PasteData& data)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    auto userId = GetUserId();
    std::lock_guard<std::mutex> lock(clipMutex_);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Clips length %{public}d.", static_cast<uint32_t>(clips_.size()));
    auto it = clips_.find(userId);
    if (it != clips_.end()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "find end.");
        data = *(it->second);
        return true;
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "not found end.");
        return false;
    }
}

bool PasteboardService::HasPasteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    auto userId = GetUserId();
    std::lock_guard<std::mutex> lock(clipMutex_);
    return clips_.find(userId) != clips_.end();
}

void PasteboardService::SetPasteData(PasteData& pasteData)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start.");
    auto userId = GetUserId();
    std::lock_guard<std::mutex> lock(clipMutex_);
    auto it = clips_.find(userId);
    if (it != clips_.end()) {
        clips_.erase(it);
    }
    clips_.insert(std::make_pair(userId, std::make_shared<PasteData>(pasteData)));
    NotifyObservers();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Clips length %{public}d.", static_cast<uint32_t>(clips_.size()));
}

int32_t PasteboardService::GetUserId()
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "uid: %{public}d.", uid);
    return uid ;
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
    auto it = observerMap_.find(userId);
    std::shared_ptr<std::set<const sptr<IPasteboardChangedObserver>, classcomp>> observers;
    if (it != observerMap_.end()) {
        observers = it->second;
    } else {
        observers = std::make_shared<std::set<const sptr<IPasteboardChangedObserver>, classcomp>>();
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
        for (const auto &observer: *(observers.second)) {
            observer->OnPasteboardChanged();
        }
    }
}
} // MiscServices
} // OHOS