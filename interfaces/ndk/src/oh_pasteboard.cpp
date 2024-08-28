/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#define LOG_TAG "Pasteboard_Capi"

#include "oh_pasteboard.h"
#include <string>
#include <memory>
#include <thread>
#include <map>
#include "udmf.h"
#include "oh_pasteboard_err_code.h"
#include "oh_pasteboard_observer_impl.h"
#include "pasteboard_client.h"
#include "pasteboard_hilog.h"
#include "pasteboard_error.h"
#include "udmf_capi_common.h"
#include "i_pasteboard_observer.h"

using namespace OHOS::MiscServices;
static bool IsPasteboardValid(OH_Pasteboard* pasteboard)
{
    return pasteboard != nullptr && pasteboard->cid == PASTEBOARD_STRUCT_ID;
}

static bool IsSubscriberValid(OH_PasteboardObserver* observer)
{
    return observer != nullptr && observer->cid == SUBSCRIBER_STRUCT_ID;
}

OH_PasteboardObserver* OH_PasteboardObserver_Create()
{
    OH_PasteboardObserver* observer = new(std::nothrow) OH_PasteboardObserver();
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CAPI, "allocate memory fail.");
        return nullptr;
    }
    return observer;
}

int OH_PasteboardObserver_Destroy(OH_PasteboardObserver* observer)
{
    if (!IsSubscriberValid(observer)) {
        return ERR_INVALID_PARAMETER;
    }
    if (observer->finalize != nullptr) {
        (observer->finalize)(observer->context);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CAPI, "context finalized");
    }
    delete observer;
    return ERR_OK;
}

int OH_PasteboardObserver_SetData(OH_PasteboardObserver* observer, void* context,
    const Pasteboard_Notify callback, const Pasteboard_Finalize finalize)
{
    if (observer == nullptr || callback == nullptr) {
        return ERR_INVALID_PARAMETER;
    }
    observer->callback = callback;
    if (context != nullptr && finalize == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CAPI, "finalize is null");
        return ERR_INVALID_PARAMETER;
    }
    observer->context = context;
    observer->finalize = finalize;
    return ERR_OK;
}

OH_Pasteboard* OH_Pasteboard_Create()
{
    OH_Pasteboard* pasteboard = new (std::nothrow) OH_Pasteboard();
    if (pasteboard == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CAPI, "allocate memory fail.");
        return nullptr;
    }
    return pasteboard;
}

void OH_Pasteboard_Destroy(OH_Pasteboard* pasteboard)
{
    if (!IsPasteboardValid(pasteboard)) {
        return;
    }
    std::lock_guard<std::mutex> lock(pasteboard->mutex);
    for (auto iter : pasteboard->observers_) {
        if (iter.second != nullptr) {
            PasteboardClient::GetInstance()->Unsubscribe(
                static_cast<PasteboardObserverType>(iter.second->GetType()), iter.second);
        }
    }
    pasteboard->observers_.clear();
    delete pasteboard;
}

int OH_Pasteboard_Subscribe(OH_Pasteboard* pasteboard, int type, const OH_PasteboardObserver* observer)
{
    if (!IsPasteboardValid(pasteboard) || observer == nullptr || type < NOTIFY_LOCAL_DATA_CHANGE
        || type > NOTIFY_REMOTE_DATA_CHANGE) {
        return ERR_INVALID_PARAMETER;
    }
    std::lock_guard<std::mutex> lock(pasteboard->mutex);
    auto iter = pasteboard->observers_.find(observer);
    if (iter != pasteboard->observers_.end()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CAPI, "observer exist.");
        return ERR_OK;
    }
    OHOS::sptr<PasteboardObserverCapiImpl> observerBox = new (std::nothrow) PasteboardObserverCapiImpl();
    if (observerBox == nullptr) {
        return ERR_ALLOCATE_MEMORY_FAIL;
    }
    observerBox->SetInnerObserver(observer);
    observerBox->SetType(static_cast<Pasteboard_NotifyType>(type));
    pasteboard->observers_[observer] = observerBox;
    PasteboardClient::GetInstance()->Subscribe(static_cast<PasteboardObserverType>(type), observerBox);
    return ERR_OK;
}

int OH_Pasteboard_Unsubscribe(OH_Pasteboard* pasteboard, int type, const OH_PasteboardObserver* observer)
{
    if (!IsPasteboardValid(pasteboard) || observer == nullptr  || type < NOTIFY_LOCAL_DATA_CHANGE
        || type > NOTIFY_REMOTE_DATA_CHANGE) {
        return ERR_INVALID_PARAMETER;
    }
    std::lock_guard<std::mutex> lock(pasteboard->mutex);
    auto iter = pasteboard->observers_.find(observer);
    if (iter == pasteboard->observers_.end()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CAPI, "couldn't find this observer");
        return ERR_OBSERVER_NOT_EXIST;
    }
    PasteboardClient::GetInstance()->Unsubscribe(static_cast<PasteboardObserverType>(type), iter->second);
    pasteboard->observers_.erase(iter);
    return ERR_OK;
}

bool OH_Pasteboard_IsRemoteData(OH_Pasteboard* pasteboard)
{
    if (!IsPasteboardValid(pasteboard)) {
        return ERR_INVALID_PARAMETER;
    }
    return PasteboardClient::GetInstance()->IsRemoteData();
}

int OH_Pasteboard_GetDataSource(OH_Pasteboard* pasteboard, char* source, unsigned int len)
{
    if (!IsPasteboardValid(pasteboard) || source == nullptr || len == 0) {
        return ERR_INVALID_PARAMETER;
    }
    std::string bundleName;
    auto ret = PasteboardClient::GetInstance()->GetDataSource(bundleName);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(
            PASTEBOARD_MODULE_CAPI, "client getDataSource return invalid, result is %{public}d", ret);
        return ERR_CLIENT_FAIL;
    }
    if (strcpy_s(source, len, bundleName.c_str()) != EOK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CAPI, "copy string fail");
        return ERR_FAIL;
    }
    return ERR_OK;
}

bool OH_Pasteboard_HasType(OH_Pasteboard* pasteboard, const char* type)
{
    if (!IsPasteboardValid(pasteboard) || type == nullptr) {
        return ERR_INVALID_PARAMETER;
    }
    return PasteboardClient::GetInstance()->HasDataType(std::string(type));
}

bool OH_Pasteboard_HasData(OH_Pasteboard* pasteboard)
{
    if (!IsPasteboardValid(pasteboard)) {
        return ERR_INVALID_PARAMETER;
    }
    return PasteboardClient::GetInstance()->HasPasteData();
}

OH_UdmfData* OH_Pasteboard_GetData(OH_Pasteboard* pasteboard, int* status)
{
    if (!IsPasteboardValid(pasteboard) || status == nullptr) {
        return nullptr;
    }
    auto unifiedData = std::make_shared<OHOS::UDMF::UnifiedData>();
    auto ret = PasteboardClient::GetInstance()->GetUdsdData(*unifiedData);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(
            PASTEBOARD_MODULE_CAPI, "client OH_Pasteboard_GetData return invalid, result is %{public}d", ret);
        *status = ERR_CLIENT_FAIL;
        return nullptr;
    }
    OH_UdmfData* data = OH_UdmfData_Create();
    data->unifiedData_ = std::move(unifiedData);
    *status = ERR_OK;
    return data;
}

int OH_Pasteboard_SetData(OH_Pasteboard* pasteboard, OH_UdmfData* data)
{
    if (!IsPasteboardValid(pasteboard) || data == nullptr) {
        return ERR_INVALID_PARAMETER;
    }
    auto ret = PasteboardClient::GetInstance()->SetUdsdData(*(data->unifiedData_));
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(
            PASTEBOARD_MODULE_CAPI, "client OH_Pasteboard_SetData return invalid, result is %{public}d", ret);
        return ERR_CLIENT_FAIL;
    }
    return ERR_OK;
}

int OH_Pasteboard_ClearData(OH_Pasteboard* pasteboard)
{
    if (!IsPasteboardValid(pasteboard)) {
        return ERR_INVALID_PARAMETER;
    }
    PasteboardClient::GetInstance()->Clear();
    return ERR_OK;
}