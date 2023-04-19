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

#include "pasteboard_service_stub.h"

#include "errors.h"
#include "paste_data.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_observer_proxy.h"
#include "paste_uri_handler.h"
#include "copy_uri_handler.h"

namespace OHOS {
namespace MiscServices {
PasteboardServiceStub::PasteboardServiceStub()
{
    memberFuncMap_[static_cast<uint32_t>(GET_PASTE_DATA)] = &PasteboardServiceStub::OnGetPasteData;
    memberFuncMap_[static_cast<uint32_t>(HAS_PASTE_DATA)] = &PasteboardServiceStub::OnHasPasteData;
    memberFuncMap_[static_cast<uint32_t>(SET_PASTE_DATA)] = &PasteboardServiceStub::OnSetPasteData;
    memberFuncMap_[static_cast<uint32_t>(CLEAR_ALL)] = &PasteboardServiceStub::OnClear;
    memberFuncMap_[static_cast<uint32_t>(ADD_CHANGED_OBSERVER)] =
        &PasteboardServiceStub::OnAddPasteboardChangedObserver;
    memberFuncMap_[static_cast<uint32_t>(DELETE_CHANGED_OBSERVER)] =
        &PasteboardServiceStub::OnRemovePasteboardChangedObserver;
    memberFuncMap_[static_cast<uint32_t>(DELETE_ALL_CHANGED_OBSERVER)] =
        &PasteboardServiceStub::OnRemoveAllChangedObserver;
    memberFuncMap_[static_cast<uint32_t>(ADD_EVENT_OBSERVER)] =
        &PasteboardServiceStub::OnAddPasteboardEventObserver;
    memberFuncMap_[static_cast<uint32_t>(DELETE_EVENT_OBSERVER)] =
        &PasteboardServiceStub::OnRemovePasteboardEventObserver;
    memberFuncMap_[static_cast<uint32_t>(DELETE_ALL_EVENT_OBSERVER)] =
        &PasteboardServiceStub::OnRemoveAllEventObserver;
}

int32_t PasteboardServiceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start##code = %{public}u", code);
    std::u16string myDescripter = PasteboardServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "end##descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    pid_t p = IPCSkeleton::GetCallingPid();
    pid_t p1 = IPCSkeleton::GetCallingUid();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "CallingPid = %{public}d, CallingUid = %{public}d, code = %{public}u",
        p, p1, code);
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end##ret = %{public}d", ret);
    return ret;
}
int32_t PasteboardServiceStub::OnClear(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    Clear();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnGetPasteData(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, " start.");
    PasteData pasteData{};
    auto result = GetPasteData(pasteData);
    std::vector<uint8_t> pasteDataTlv(0);
    bool ret = pasteData.Encode(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to encode pastedata in TLV");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(pasteDataTlv.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write raw size");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteRawData(pasteDataTlv.data(), pasteDataTlv.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write raw data");
        return ERR_INVALID_VALUE;
    }
    PasteUriHandler pasteUriHandler;
    if (!pasteData.WriteUriFd(reply, pasteUriHandler, false)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write uri fd");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to GetPasteData result");
        return ERR_INVALID_VALUE;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, " end.");
    return ERR_OK;
}
int32_t PasteboardServiceStub::OnHasPasteData(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, " start.");
    auto result = HasPasteData();
    reply.WriteBool(result);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, " end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnSetPasteData(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, " start.");
    int32_t rawDataSize = data.ReadInt32();
    if (rawDataSize <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to read raw size");
        return ERR_INVALID_VALUE;
    }
    auto *rawData = (uint8_t *)data.ReadRawData(rawDataSize);
    if (rawData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to get raw data");
        return ERR_INVALID_VALUE;
    }
    std::vector<uint8_t> pasteDataTlv(rawData, rawData + rawDataSize);
    PasteData pasteData;
    bool ret = pasteData.Decode(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to decode pastedata in TLV");
        return ERR_INVALID_VALUE;
    }
    CopyUriHandler copyUriHandler;
    if (!pasteData.ReadUriFd(data, copyUriHandler)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to read uri fd");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SetPasteData(pasteData);
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write SetPasteData result");
        return ERR_INVALID_VALUE;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, " end.");
    return ERR_OK;
}
int32_t PasteboardServiceStub::OnAddPasteboardChangedObserver(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    sptr<IPasteboardChangedObserver> callback;
    if (!IsObserverValid(data, callback)) {
        return ERR_INVALID_VALUE;
    }

    AddPasteboardChangedObserver(callback);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}
int32_t PasteboardServiceStub::OnRemovePasteboardChangedObserver(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    sptr<IPasteboardChangedObserver> callback;
    if (!IsObserverValid(data, callback)) {
        return ERR_INVALID_VALUE;
    }
    RemovePasteboardChangedObserver(callback);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnRemoveAllChangedObserver(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    RemoveAllChangedObserver();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnAddPasteboardEventObserver(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    sptr<IPasteboardChangedObserver> callback;
    if (!IsObserverValid(data, callback)) {
        return ERR_INVALID_VALUE;
    }

    AddPasteboardEventObserver(callback);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnRemovePasteboardEventObserver(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    sptr<IPasteboardChangedObserver> callback;
    if (!IsObserverValid(data, callback)) {
        return ERR_INVALID_VALUE;
    }
    RemovePasteboardEventObserver(callback);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnRemoveAllEventObserver(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    RemoveAllEventObserver();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

inline bool PasteboardServiceStub::IsObserverValid(MessageParcel &data, sptr<IPasteboardChangedObserver> &callback)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    if (obj == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "obj nullptr");
        return false;
    }
    callback = iface_cast<IPasteboardChangedObserver>(obj);
    if (callback == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "callback nullptr");
        return false;
    }
    return true;
}

PasteboardServiceStub::~PasteboardServiceStub()
{
    memberFuncMap_.clear();
}
} // namespace MiscServices
} // namespace OHOS
