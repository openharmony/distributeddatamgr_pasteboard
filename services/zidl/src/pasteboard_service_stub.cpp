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

#include "hiview_adapter.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_serv_ipc_interface_code.h"

#define MAX_RAWDATA_SIZE (128 * 1024 * 1024)

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
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::DETECT_PATTERNS)] =
        &PasteboardServiceStub::OnDetectPatterns;
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
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_RECORD_VALUE)] =
        &PasteboardServiceStub::OnGetRecordValueByType;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_MIME_TYPES)] =
        &PasteboardServiceStub::OnGetMimeTypes;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_REMOTE_DEVICE_NAME)] =
        &PasteboardServiceStub::OnGetRemoteDeviceName;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::SHOW_PROGRESS)] =
        &PasteboardServiceStub::OnShowProgress;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::GET_CHANGE_COUNT)] =
        &PasteboardServiceStub::OnGetChangeCount;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::SUBSCRIBE_ENTITY_OBSERVER)] =
        &PasteboardServiceStub::OnSubscribeEntityObserver;
    memberFuncMap_[static_cast<uint32_t>(PasteboardServiceInterfaceCode::UNSUBSCRIBE_ENTITY_OBSERVER)] =
        &PasteboardServiceStub::OnUnsubscribeEntityObserver;
}

int32_t PasteboardServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescriptor = PasteboardServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "end##descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    pid_t pid = IPCSkeleton::GetCallingPid();
    pid_t uid = IPCSkeleton::GetCallingUid();
    if (code != static_cast<uint32_t>(PasteboardServiceInterfaceCode::HAS_PASTE_DATA)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "pid:%{public}d, uid:%{public}d, cmd:%{public}u", pid, uid, code);
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
    Clear();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnGetRecordValueByType(MessageParcel &data, MessageParcel &reply)
{
    uint32_t dataId = 0;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(data.ReadUint32(dataId), ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE,
        "read uint32 failed");
    uint32_t recordId = 0;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(data.ReadUint32(recordId), ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE,
        "read uint32 failed");
    PasteDataEntry entryValue;
    int32_t rawDataSize = data.ReadInt32();
    if (rawDataSize <= 0 || rawDataSize > MAX_RAWDATA_SIZE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "invalid raw data size");
        return ERR_INVALID_VALUE;
    }
    const uint8_t *rawData = reinterpret_cast<const uint8_t *>(data.ReadRawData(rawDataSize));
    if (rawData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "fail to get raw data");
        return ERR_INVALID_VALUE;
    }
    std::vector<uint8_t> receiveTlv(rawData, rawData + rawDataSize);
    bool ret = entryValue.Unmarshalling(receiveTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "fail to decode paste data entry");
        return ERR_INVALID_VALUE;
    }
    auto result = GetRecordValueByType(dataId, recordId, entryValue);
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write result:%{public}d", result);
        return ERR_INVALID_VALUE;
    }
    std::vector<uint8_t> entryValueTLV(0);
    ret = entryValue.Marshalling(entryValueTLV);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "fail encode entry value");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(entryValueTLV.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "fail write data size");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteRawData(entryValueTLV.data(), entryValueTLV.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "fail write raw data");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnGetPasteData(MessageParcel &data, MessageParcel &reply)
{
    std::string pasteId = data.ReadString();
    PasteData pasteData{};
    pasteData.SetPasteId(pasteId);
    int32_t syncTime = 0;
    auto result = GetPasteData(pasteData, syncTime);
    HiViewAdapter::ReportUseBehaviour(pasteData, HiViewAdapter::PASTE_STATE, result);
    std::vector<uint8_t> pasteDataTlv(0);
    bool ret = pasteData.Marshalling(pasteDataTlv);
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

std::shared_ptr<PasteData> PasteboardServiceStub::UnmarshalPasteData(MessageParcel &data, MessageParcel &reply)
{
    int32_t rawDataSize = data.ReadInt32();
    if (rawDataSize <= 0 || rawDataSize > MAX_RAWDATA_SIZE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Invalid raw data size");
        return nullptr;
    }
    auto *rawData = (uint8_t *)data.ReadRawData(rawDataSize);
    if (rawData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to get raw data");
        return nullptr;
    }
    std::vector<uint8_t> pasteDataTlv(rawData, rawData + rawDataSize);
    auto pasteData = std::make_shared<PasteData>();
    bool ret = pasteData->Unmarshalling(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to decode pastedata in TLV");
        return nullptr;
    }
    return pasteData;
}

int32_t PasteboardServiceStub::OnSetPasteData(MessageParcel &data, MessageParcel &reply)
{
    auto pasteData = UnmarshalPasteData(data, reply);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(pasteData != nullptr, ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE,
        "Failed to Unmarshal PasteData");
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    if (pasteData->IsDelayData()) {
        sptr<IRemoteObject> obj = data.ReadRemoteObject();
        if (obj == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "delay getter is nullptr");
            return ERR_INVALID_VALUE;
        }
        delayGetter = iface_cast<IPasteboardDelayGetter>(obj);
        if (delayGetter == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "delay getter is nullptr");
            return ERR_INVALID_VALUE;
        }
    }
    sptr<IPasteboardEntryGetter> entryGetter = nullptr;
    if (pasteData->IsDelayRecord()) {
        sptr<IRemoteObject> obj = data.ReadRemoteObject();
        if (obj == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "entry getter is nullptr");
            return ERR_INVALID_VALUE;
        }
        entryGetter = iface_cast<IPasteboardEntryGetter>(obj);
        if (entryGetter == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "entry getter is nullptr");
            return ERR_INVALID_VALUE;
        }
    }
    auto result = SetPasteData(*pasteData, delayGetter, entryGetter);
    HiViewAdapter::ReportUseBehaviour(*pasteData, HiViewAdapter::COPY_STATE, result);
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write SetPasteData result");
        return ERR_INVALID_VALUE;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, " end, ret is %{public}d.", result);
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnSubscribeObserver(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = 0;
    sptr<IPasteboardChangedObserver> callback;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(IsObserverValid(data, type, callback), ERR_INVALID_VALUE,
        PASTEBOARD_MODULE_SERVICE, "Observer invalid");

    SubscribeObserver(static_cast<PasteboardObserverType>(type), callback);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}
int32_t PasteboardServiceStub::OnUnsubscribeObserver(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = 0;
    sptr<IPasteboardChangedObserver> callback;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(IsObserverValid(data, type, callback), ERR_INVALID_VALUE,
        PASTEBOARD_MODULE_SERVICE, "Observer invalid");
    UnsubscribeObserver(static_cast<PasteboardObserverType>(type), callback);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnUnsubscribeAllObserver(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = 0;
    if (!data.ReadUint32(type)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read type failed.");
        return ERR_INVALID_VALUE;
    }
    UnsubscribeAllObserver(static_cast<PasteboardObserverType>(type));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

bool PasteboardServiceStub::IsObserverValid(
    MessageParcel &data, uint32_t &type, sptr<IPasteboardChangedObserver> &callback)
{
    if (!data.ReadUint32(type)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read type failed.");
        return false;
    }
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(obj != nullptr, false, PASTEBOARD_MODULE_SERVICE, "obj nullptr");
    callback = iface_cast<IPasteboardChangedObserver>(obj);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(callback != nullptr, false, PASTEBOARD_MODULE_SERVICE, "callback nullptr");
    return true;
}

int32_t PasteboardServiceStub::OnIsRemoteData(MessageParcel &data, MessageParcel &reply)
{
    auto result = IsRemoteData();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(reply.WriteBool(result), ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE,
        "Failed to write result");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnGetDataSource(MessageParcel &data, MessageParcel &reply)
{
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
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end, ret is %{public}d.", ret);
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnGetMimeTypes(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> mimeTypes = GetMimeTypes();
    if (!reply.WriteUint32(static_cast<uint32_t>(mimeTypes.size()))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write size failed.");
        return ERR_INVALID_VALUE;
    }
    for (const std::string &type : mimeTypes) {
        if (!reply.WriteString(type)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write mime type failed.");
            return ERR_INVALID_VALUE;
        }
    }
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnGetChangeCount(MessageParcel &data, MessageParcel &reply)
{
    uint32_t changeCount = 0;
    int32_t ret = GetChangeCount(changeCount);
    if (!reply.WriteUint32(changeCount)) {
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnSubscribeEntityObserver(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = 0;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        data.ReadUint32(type), ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE, "Failed to read data");
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(type < static_cast<uint32_t>(EntityType::MAX), ERR_INVALID_VALUE,
        PASTEBOARD_MODULE_SERVICE, "Failed to read data");
    EntityType entityType = static_cast<EntityType>(type);
    uint32_t expectedDataLength = 0;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        data.ReadUint32(expectedDataLength), ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE, "Failed to read data");
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    sptr<IEntityRecognitionObserver> observer = iface_cast<IEntityRecognitionObserver>(callback);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        observer != nullptr, ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE, "Failed to read data");
    int32_t ret = SubscribeEntityObserver(entityType, expectedDataLength, observer);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(reply.WriteInt32(ret),
        static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR), PASTEBOARD_MODULE_SERVICE, "Write result failed");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnUnsubscribeEntityObserver(MessageParcel &data, MessageParcel &reply)
{
    uint32_t type = 0;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        data.ReadUint32(type), ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE, "Failed to read data");
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(type < static_cast<uint32_t>(EntityType::MAX), ERR_INVALID_VALUE,
        PASTEBOARD_MODULE_SERVICE, "Failed to read data");
    EntityType entityType = static_cast<EntityType>(type);
    uint32_t expectedDataLength = 0;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        data.ReadUint32(expectedDataLength), ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE, "Failed to read data");
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    sptr<IEntityRecognitionObserver> observer = iface_cast<IEntityRecognitionObserver>(callback);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        observer != nullptr, ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE, "Failed to read data");
    int32_t ret = UnsubscribeEntityObserver(entityType, expectedDataLength, observer);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(reply.WriteInt32(ret),
        static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR), PASTEBOARD_MODULE_SERVICE, "Write result failed");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnGetRemoteDeviceName(MessageParcel &data, MessageParcel &reply)
{
    std::string name;
    bool isRemote;
    auto ret = GetRemoteDeviceName(name, isRemote);
    if (name.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to get remote device name");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteString(name) || !reply.WriteBool(isRemote)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write name result");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(ret)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write result");
        return ERR_INVALID_VALUE;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end, ret is %{public}d.", ret);
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnShowProgress(MessageParcel &data, MessageParcel &reply)
{
    std::string progressKey = data.ReadString();
    sptr<IRemoteObject> callback = data.ReadRemoteObject();
    if (callback == nullptr) {
        return ERR_INVALID_VALUE;
    }
    ShowProgress(progressKey, callback);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnHasDataType(MessageParcel &data, MessageParcel &reply)
{
    std::string mimeType = data.ReadString();
    auto ret = HasDataType(mimeType);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(reply.WriteBool(ret), ERR_INVALID_VALUE, PASTEBOARD_MODULE_SERVICE,
        "Failed to write ret");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnDetectPatterns(MessageParcel &data, MessageParcel &reply)
{
    uint32_t size = 0;
    if (!data.ReadUint32(size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read size failed.");
        return ERR_INVALID_VALUE;
    }
    size_t readAbleSize = data.GetReadableBytes();
    if (size > readAbleSize || size > static_cast<uint32_t>(Pattern::COUNT)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read oversize failed.");
        return ERR_INVALID_VALUE;
    }
    std::set<Pattern> patternsToCheck;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t pattern;
        if (!data.ReadUint32(pattern)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read pattern failed.");
            return ERR_INVALID_VALUE;
        }
        patternsToCheck.insert(static_cast<Pattern>(pattern));
    }
    std::set<Pattern> existedPatterns = DetectPatterns(patternsToCheck);
    if (!reply.WriteUint32(static_cast<uint32_t>(existedPatterns.size()))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write size failed.");
        return ERR_INVALID_VALUE;
    }
    for (const auto &pattern : existedPatterns) {
        if (!reply.WriteUint32(static_cast<uint32_t>(pattern))) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write pattern failed.");
            return ERR_INVALID_VALUE;
        }
    }
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
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR);
    }
    auto result = SetAppShareOptions(static_cast<ShareOption>(shareOptions));
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write result failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnRemoveAppShareOptions(MessageParcel &data, MessageParcel &reply)
{
    auto result = RemoveAppShareOptions();
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Write result failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    return ERR_OK;
}

PasteboardServiceStub::~PasteboardServiceStub()
{
    memberFuncMap_.clear();
}

int32_t PasteboardServiceStub::OnPasteStart(MessageParcel &data, MessageParcel &reply)
{
    std::string pasteId = data.ReadString();
    PasteStart(pasteId);
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnPasteComplete(MessageParcel &data, MessageParcel &reply)
{
    std::string deviceId = data.ReadString();
    std::string pasteId = data.ReadString();
    PasteComplete(deviceId, pasteId);
    return ERR_OK;
}

int32_t PasteboardServiceStub::OnRegisterClientDeathObserver(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> pasteboardClientDeathObserverProxy = data.ReadRemoteObject();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(pasteboardClientDeathObserverProxy != nullptr, ERR_INVALID_VALUE,
        PASTEBOARD_MODULE_SERVICE, "Read remote object failed");
    int32_t status = RegisterClientDeathObserver(std::move(pasteboardClientDeathObserverProxy));
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(reply.WriteInt32(static_cast<int32_t>(status)), ERR_INVALID_VALUE,
        PASTEBOARD_MODULE_SERVICE, "Write status failed");
    return ERR_OK;
}
} // namespace MiscServices
} // namespace OHOS