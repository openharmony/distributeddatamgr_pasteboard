/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "copy_uri_handler.h"
#include "errors.h"
#include "hiview_adapter.h"
#include "paste_data.h"
#include "paste_uri_handler.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_observer_proxy.h"
#include "pasteboard_serv_ipc_interface_code.h"

using namespace OHOS::Security::PasteboardServ;
namespace OHOS {
namespace MiscServices {
PasteboardServiceStub::PasteboardServiceStub()
{
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_PASTE_DATA)] =
        &PasteboardServiceStub::OnGetPasteData;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::HAS_PASTE_DATA)] =
        &PasteboardServiceStub::OnHasPasteData;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::SET_PASTE_DATA)] =
        &PasteboardServiceStub::OnSetPasteData;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::CLEAR_ALL)] = &PasteboardServiceStub::OnClear;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::ADD_CHANGED_OBSERVER)] =
        &PasteboardServiceStub::OnAddPasteboardChangedObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::DELETE_CHANGED_OBSERVER)] =
        &PasteboardServiceStub::OnRemovePasteboardChangedObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::DELETE_ALL_CHANGED_OBSERVER)] =
        &PasteboardServiceStub::OnRemoveAllChangedObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::ADD_EVENT_OBSERVER)] =
        &PasteboardServiceStub::OnAddPasteboardEventObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::DELETE_EVENT_OBSERVER)] =
        &PasteboardServiceStub::OnRemovePasteboardEventObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::DELETE_ALL_EVENT_OBSERVER)] =
        &PasteboardServiceStub::OnRemoveAllEventObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::IS_REMOTE_DATA)] =
            &PasteboardServiceStub::OnIsRemoteData;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_DATA_SOURCE)] =
            &PasteboardServiceStub::OnGetDataSource;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::HAS_DATA_TYPE)] =
            &PasteboardServiceStub::OnHasDataType;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::SET_GLOBAL_SHARE_OPTION)] =
            &PasteboardServiceStub::OnSetGlobalShareOption;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::REMOVE_GLOBAL_SHARE_OPTION)] =
            &PasteboardServiceStub::OnRemoveGlobalShareOption;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_GLOBAL_SHARE_OPTION)] =
            &PasteboardServiceStub::OnGetGlobalShareOption;
}

int32_t PasteboardServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
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
    PASTEBOARD_HILOGI(
        PASTEBOARD_MODULE_SERVICE, "CallingPid = %{public}d, CallingUid = %{public}d, code = %{public}u", p, p1, code);
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
    HiViewAdapter::ReportUseBehaviour(pasteData, HiViewAdapter::PASTE_STATE, result);
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
    auto pasteData = std::make_shared<PasteData>();
    bool ret = pasteData->Decode(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to decode pastedata in TLV");
        return ERR_INVALID_VALUE;
    }
    CopyUriHandler copyUriHandler;
    if (!pasteData->ReadUriFd(data, copyUriHandler)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to read uri fd");
        return ERR_INVALID_VALUE;
    }
    int32_t result = SavePasteData(pasteData);
    HiViewAdapter::ReportUseBehaviour(*pasteData, HiViewAdapter::COPY_STATE, result);
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

int32_t PasteboardServiceStub::OnIsRemoteData(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    auto result = IsRemoteData();
    reply.WriteBool(result);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnGetDataSource(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    std::string bundleName;
    auto ret = GetDataSource(bundleName);
    if (bundleName.empty() || bundleName.length() > MAX_BUNDLE_NAME_LENGTH) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to get bundleName");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteString(bundleName)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to writeName result");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(ret)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to OnGetResourceApp result");
        return ERR_INVALID_VALUE;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnHasDataType(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    std::string mimeType = data.ReadString();
    auto ret = HasDataType(mimeType);
    reply.WriteBool(ret);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnSetGlobalShareOption(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnSetGlobalShareOption start.");
    int32_t size = 0;
    if (!data.ReadInt32(size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read size failed.");
        return static_cast<int32_t>(PasteboardError::E_READ_PARCEL_ERROR);
    }
    size_t readAbleSize = data.GetReadableBytes();
    std::map<uint32_t, ShareOption> globalShareOption;
    if (static_cast<size_t>(size) > readAbleSize || static_cast<size_t>(size) > globalShareOption.max_size()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read oversize failed.");
        return static_cast<int32_t>(PasteboardError::E_INVALID_VALUE);
    }
    for (int32_t i = 0; i < size; i++) {
        uint32_t key;
        if (!data.ReadUint32(key)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read tokenId failed.");
            return static_cast<int32_t>(PasteboardError::E_READ_PARCEL_ERROR);
        }
        int32_t value;
        if (!data.ReadInt32(value)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read shareOption failed.");
            return static_cast<int32_t>(PasteboardError::E_READ_PARCEL_ERROR);
        }
        globalShareOption[key] = static_cast<ShareOption>(value);
    }
    int32_t result = SetGlobalShareOption(globalShareOption);
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write result failed.");
        return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnSetGlobalShareOption end.");
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardServiceStub::OnRemoveGlobalShareOption(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnRemoveGlobalShareOption start.");
    std::vector<uint32_t> tokenId;
    if (!data.ReadUInt32Vector(&tokenId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read tokenId failed.");
        return static_cast<int32_t>(PasteboardError::E_READ_PARCEL_ERROR);
    }
    int32_t result = RemoveGlobalShareOption(tokenId);
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write result failed.");
        return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnRemoveGlobalShareOption end.");
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardServiceStub::OnGetGlobalShareOption(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnGetGlobalShareOption start.");
    std::vector<uint32_t> tokenId;
    if (!data.ReadUInt32Vector(&tokenId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read tokenId failed.");
        return static_cast<int32_t>(PasteboardError::E_READ_PARCEL_ERROR);
    }
    std::map<uint32_t, ShareOption> globalShareOption = GetGlobalShareOption(tokenId);
    if (!reply.WriteInt32(static_cast<int32_t>(globalShareOption.size()))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write size failed.");
        return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
    }
    for (const auto &entry : globalShareOption) {
        if (!reply.WriteUint32(entry.first)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write tokenId failed.");
            return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
        }
        if (!reply.WriteInt32(static_cast<int32_t>(entry.second))) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write shareOption failed.");
            return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "OnGetGlobalShareOption end.");
    return static_cast<int32_t>(PasteboardError::E_OK);
}

PasteboardServiceStub::~PasteboardServiceStub()
{
    memberFuncMap_.clear();
}
} // namespace MiscServices
} // namespace OHOS
