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
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::SUBSCRIBE_OBSERVER)] =
        &PasteboardServiceStub::OnSubscribeObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::UNSUBSCRIBE_OBSERVER)] =
        &PasteboardServiceStub::OnUnsubscribeObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::UNSUBSCRIBE_ALL_OBSERVER)] =
        &PasteboardServiceStub::OnUnsubscribeAllObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::IS_REMOTE_DATA)] =
            &PasteboardServiceStub::OnIsRemoteData;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_DATA_SOURCE)] =
            &PasteboardServiceStub::OnGetDataSource;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::HAS_DATA_TYPE)] =
            &PasteboardServiceStub::OnHasDataType;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::EXISTED_PATTERNS)] =
            &PasteboardServiceStub::OnExistedPatterns;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::SET_GLOBAL_SHARE_OPTION)] =
            &PasteboardServiceStub::OnSetGlobalShareOption;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::REMOVE_GLOBAL_SHARE_OPTION)] =
            &PasteboardServiceStub::OnRemoveGlobalShareOption;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_GLOBAL_SHARE_OPTION)] =
            &PasteboardServiceStub::OnGetGlobalShareOption;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::SET_APP_SHARE_OPTIONS)] =
            &PasteboardServiceStub::OnSetAppShareOptions;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::REMOVE_APP_SHARE_OPTIONS)] =
            &PasteboardServiceStub::OnRemoveAppShareOptions;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::PASTE_START)] =
            &PasteboardServiceStub::OnPasteStart;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::PASTE_COMPLETE)] =
            &PasteboardServiceStub::OnPasteComplete;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::REGISTER_CLIENT_DEATH_OBSERVER)] =
            &PasteboardServiceStub::OnRegisterClientDeathObserver;
}

int32_t PasteboardServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescripter = PasteboardServiceStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "end##descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    pid_t p = IPCSkeleton::GetCallingPid();
    pid_t p1 = IPCSkeleton::GetCallingUid();
    if (code != static_cast<uint32_t>(PasteboardServiceInterfaceCode::HAS_PASTE_DATA)) {
        PASTEBOARD_HILOGI(
            PASTEBOARD_MODULE_SERVICE, "pid:%{public}d, uid:%{public}d, cmd:%{public}u", p, p1, code);
    }
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
    int32_t syncTime = 0;
    auto result = GetPasteData(pasteData, syncTime);
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
    if (!reply.WriteInt32(syncTime)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to GetPasteData syncTime");
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
    auto result = HasPasteData();
    reply.WriteBool(result);
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
    int32_t result = 0;
    if (pasteData->IsDelayData()) {
        sptr<IRemoteObject> obj = data.ReadRemoteObject();
        if (obj == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "delayGetter is nullptr");
            return ERR_INVALID_VALUE;
        }
        auto delayGetter = iface_cast<IPasteboardDelayGetter>(obj);
        if (delayGetter == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "delayGetter is nullptr");
            return ERR_INVALID_VALUE;
        }
        result = SavePasteData(pasteData, delayGetter);
    } else {
        result = SavePasteData(pasteData);
    }
    HiViewAdapter::ReportUseBehaviour(*pasteData, HiViewAdapter::COPY_STATE, result);
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write SetPasteData result");
        return ERR_INVALID_VALUE;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, " end.");
    return ERR_OK;
}
int32_t PasteboardServiceStub::OnSubscribeObserver(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    uint32_t type = 0;
    sptr<IPasteboardChangedObserver> callback;
    if (!IsObserverValid(data, type, callback)) {
        return ERR_INVALID_VALUE;
    }

    SubscribeObserver(static_cast<PasteboardObserverType>(type), callback);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}
int32_t PasteboardServiceStub::OnUnsubscribeObserver(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    uint32_t type = 0;
    sptr<IPasteboardChangedObserver> callback;
    if (!IsObserverValid(data, type, callback)) {
        return ERR_INVALID_VALUE;
    }
    UnsubscribeObserver(static_cast<PasteboardObserverType>(type), callback);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnUnsubscribeAllObserver(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    uint32_t type = 0;
    if (!data.ReadUint32(type)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read type failed.");
        return ERR_INVALID_VALUE;
    }
    UnsubscribeAllObserver(static_cast<PasteboardObserverType>(type));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

bool PasteboardServiceStub::IsObserverValid(MessageParcel &data, uint32_t &type,
    sptr<IPasteboardChangedObserver> &callback)
{
    if (!data.ReadUint32(type)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read type failed.");
        return false;
    }
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

int32_t PasteboardServiceStub::OnExistedPatterns(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "start.");
    uint32_t size = 0;
    if (!data.ReadUint32(size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read size failed.");
        return ERR_INVALID_VALUE;
    }
    size_t readAbleSize = data.GetReadableBytes();
    if (size > readAbleSize || size > static_cast<uint32_t>(Pattern::PatternCount_)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read oversize failed.");
        return ERR_INVALID_VALUE;
    }
    std::unordered_set<Pattern> patternsToCheck;
    for (uint32_t i = 0; i < size; i++) {
        int32_t pattern;
        if (!data.ReadInt32(pattern)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read pattern failed.");
            return ERR_INVALID_VALUE;
        }
        patternsToCheck.insert(static_cast<Pattern>(pattern));
    }
    std::unordered_set<Pattern> existedPatterns = ExistedPatterns(patternsToCheck);
    if (!reply.WriteUint32(static_cast<uint32_t>(existedPatterns.size()))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write size failed.");
        return ERR_INVALID_VALUE;
    }
    for (const auto &pattern : existedPatterns) {
        if (!reply.WriteInt32(static_cast<int32_t>(pattern))) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write pattern failed.");
            return ERR_INVALID_VALUE;
        }
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnSetGlobalShareOption(MessageParcel &data, MessageParcel &reply)
{
    uint32_t size = 0;
    if (!data.ReadUint32(size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read size failed.");
        return ERR_INVALID_VALUE;
    }
    size_t readAbleSize = data.GetReadableBytes();
    if (size > readAbleSize || size > MAX_SET_GLOBAL_SHARE_OPTION_SIZE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read oversize failed.");
        return ERR_INVALID_VALUE;
    }
    std::map<uint32_t, ShareOption> globalShareOptions;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t tokenId;
        if (!data.ReadUint32(tokenId)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read tokenId failed.");
            return ERR_INVALID_VALUE;
        }
        int32_t shareOption;
        if (!data.ReadInt32(shareOption)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read shareOption failed.");
            return ERR_INVALID_VALUE;
        }
        globalShareOptions[tokenId] = static_cast<ShareOption>(shareOption);
    }
    int32_t result = SetGlobalShareOption(globalShareOptions);
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write result failed.");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnRemoveGlobalShareOption(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint32_t> tokenIds;
    if (!data.ReadUInt32Vector(&tokenIds)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read tokenIds failed.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = RemoveGlobalShareOption(tokenIds);
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write result failed.");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnGetGlobalShareOption(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint32_t> tokenIds;
    if (!data.ReadUInt32Vector(&tokenIds)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read tokenIds failed.");
        return ERR_INVALID_VALUE;
    }
    std::map<uint32_t, ShareOption> globalShareOptions = GetGlobalShareOption(tokenIds);
    if (!reply.WriteUint32(static_cast<uint32_t>(globalShareOptions.size()))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write size failed.");
        return ERR_INVALID_VALUE;
    }
    for (const auto &[tokenId, shareOption] : globalShareOptions) {
        if (!reply.WriteUint32(tokenId)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write tokenId failed.");
            return ERR_INVALID_VALUE;
        }
        if (!reply.WriteInt32(static_cast<int32_t>(shareOption))) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write shareOption failed.");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnSetAppShareOptions(MessageParcel &data, MessageParcel &reply)
{
    int32_t shareOptions;
    if (!data.ReadInt32(shareOptions)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read share options failed.");
        return static_cast<int32_t>(PasteboardError::E_READ_PARCEL_ERROR);
    }
    auto result = SetAppShareOptions(static_cast<ShareOption>(shareOptions));
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write result failed.");
        return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
    }
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnRemoveAppShareOptions(MessageParcel &data, MessageParcel &reply)
{
    auto result = RemoveAppShareOptions();
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write result failed.");
        return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
    }
    return ERR_OK;
}

PasteboardServiceStub::~PasteboardServiceStub()
{
    memberFuncMap_.clear();
}

int32_t PasteboardServiceStub::OnPasteStart(MessageParcel &data, MessageParcel &reply)
{
    int32_t pasteId = data.ReadInt32();
    PasteStart(pasteId);
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnPasteComplete(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    int32_t pasteId = data.ReadInt32();
    PasteComplete(deviceId, pasteId);
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnRegisterClientDeathObserver(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> pasteboardClientDeathObserverProxy = data.ReadRemoteObject();
    if (pasteboardClientDeathObserverProxy == nullptr) {
        return ERR_INVALID_VALUE;
    }
    int32_t status = RegisterClientDeathObserver(std::move(pasteboardClientDeathObserverProxy));
    if (!reply.WriteInt32(static_cast<int32_t>(status))) {
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
} // namespace MiscServices
} // namespace OHOS