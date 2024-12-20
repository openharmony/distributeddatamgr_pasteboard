/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is: distributed on an "AS is:"BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pasteboard_service_proxy.h"

#include "copy_uri_handler.h"
#include "iremote_broker.h"
#include "paste_uri_handler.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_serv_ipc_interface_code.h"

using namespace OHOS::Security::PasteboardServ;
namespace OHOS {
namespace MiscServices {
PasteboardServiceProxy::PasteboardServiceProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IPasteboardService>(object)
{
}

void PasteboardServiceProxy::Clear()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }

    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::CLEAR_ALL, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
}

int32_t PasteboardServiceProxy::GetRecordValueByType(uint32_t dataId, uint32_t recordId, PasteDataEntry &value)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT,
            "fail to write descriptor, dataId:%{public}d or recordId:%{public}d", dataId, recordId);
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteUint32(dataId) || !data.WriteUint32(recordId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT,
            "fail to write dataId:%{public}d or recordId:%{public}d", dataId, recordId);
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    std::vector<uint8_t> sendTLV(0);
    if (!value.Marshalling(sendTLV)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fail encode entry value");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteInt32(sendTLV.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fail write data size");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteRawData(sendTLV.data(), sendTLV.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fail write raw data");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::GET_RECORD_VALUE, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is:%{public}d", result);
        return result;
    }
    int32_t res = reply.ReadInt32();
    int32_t rawDataSize = reply.ReadInt32();
    if (rawDataSize <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fail to get raw data size");
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR) ;
    }
    const uint8_t *rawData = reinterpret_cast<const uint8_t *>(reply.ReadRawData(rawDataSize));
    if (rawData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fail to get raw data");
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR);
    }
    std::vector<uint8_t> receiveTlv(rawData, rawData + rawDataSize);
    PasteDataEntry entryValue;
    if (!entryValue.Unmarshalling(receiveTlv)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fail to decode paste data entry");
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR);
    }
    value = std::move(entryValue);
    return res;
}

bool PasteboardServiceProxy::HasPasteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return false;
    }

    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::HAS_PASTE_DATA, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return false;
    }
    auto has = reply.ReadBool();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return has;
}

int32_t PasteboardServiceProxy::SetPasteData(PasteData &pasteData, const sptr<IPasteboardDelayGetter> delayGetter,
    const sptr<IPasteboardEntryGetter> entryGetter)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (pasteData.IsDelayData() && delayGetter == nullptr) {
        pasteData.SetDelayData(false);
    }
    if (pasteData.IsDelayRecord() && entryGetter == nullptr) {
        pasteData.SetDelayRecord(false);
    }
    std::vector<uint8_t> pasteDataTlv(0);
    bool ret = pasteData.Encode(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to encode pastedata in TLV");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteInt32(pasteDataTlv.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write raw size");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteRawData(pasteDataTlv.data(), pasteDataTlv.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write raw data");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    CopyUriHandler copyHandler;
    if (!pasteData.WriteUriFd(data, copyHandler)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write record uri fd");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (pasteData.IsDelayData() && !data.WriteRemoteObject(delayGetter->AsObject())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed to write delay getter");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (pasteData.IsDelayRecord() && !data.WriteRemoteObject(entryGetter->AsObject())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed to write entry getter");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::SET_PASTE_DATA, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return result;
    }
    return reply.ReadInt32();
}

__attribute__ ((no_sanitize("cfi"))) int32_t PasteboardServiceProxy::GetPasteData(PasteData &pasteData,
    int32_t &syncTime)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteString(pasteData.GetPasteId())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write pasteId");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::GET_PASTE_DATA, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return result;
    }
    pasteData.SetPasteId("");
    int32_t rawDataSize = reply.ReadInt32();
    if (rawDataSize <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to get raw size");
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR);
    }
    auto *rawData = (uint8_t *)reply.ReadRawData(rawDataSize);
    if (rawData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to get raw data");
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR);
    }
    std::vector<uint8_t> pasteDataTlv(rawData, rawData + rawDataSize);
    bool ret = pasteData.Decode(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to decode pastedata in TLV");
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR);
    }
    PasteUriHandler pasteHandler;
    if (!pasteData.ReadUriFd(reply, pasteHandler)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write record uri fd");
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR);
    }
    syncTime = reply.ReadInt32();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return reply.ReadInt32();
}

void PasteboardServiceProxy::SubscribeObserver(PasteboardObserverType type,
    const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    ProcessObserver(PasteboardServiceInterfaceCode::SUBSCRIBE_OBSERVER, type, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::UnsubscribeObserver(PasteboardObserverType type,
    const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    ProcessObserver(PasteboardServiceInterfaceCode::UNSUBSCRIBE_OBSERVER, type, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}
void PasteboardServiceProxy::UnsubscribeAllObserver(PasteboardObserverType type)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    int32_t result =
        Remote()->SendRequest(PasteboardServiceInterfaceCode::UNSUBSCRIBE_ALL_OBSERVER, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::ProcessObserver(uint32_t code, PasteboardObserverType type,
    const sptr<IPasteboardChangedObserver> &observer)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "observer nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write descriptor to parcelable");
        return;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(type))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write type to parcelable");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write observer to parcelable");
        return;
    }
    int32_t result = Remote()->SendRequest(code, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
}

bool PasteboardServiceProxy::IsRemoteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return false;
    }

    int32_t ret = Remote()->SendRequest(PasteboardServiceInterfaceCode::IS_REMOTE_DATA, data, reply, option);
    if (ret != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", ret);
        return false;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return reply.ReadBool();
}

int32_t PasteboardServiceProxy::GetDataSource(std::string &bundleName)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::GET_DATA_SOURCE, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return result;
    }
    bundleName = reply.ReadString();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return reply.ReadInt32();
}

std::vector<std::string> PasteboardServiceProxy::GetMimeTypes()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return {};
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::GET_MIME_TYPES, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return {};
    }
    uint32_t size = 0;
    if (!reply.ReadUint32(size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to read size of mime types");
        return {};
    }
    std::vector<std::string> mimeTypes;
    for (uint32_t i = 0; i < size; i++) {
        std::string type;
        if (!reply.ReadString(type)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to read mime type");
            return {};
        }
        mimeTypes.push_back(type);
    }
    return mimeTypes;
}

bool PasteboardServiceProxy::HasDataType(const std::string &mimeType)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteString(mimeType)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write string");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::HAS_DATA_TYPE, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return result;
    }

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return reply.ReadBool();
}

std::set<Pattern> PasteboardServiceProxy::DetectPatterns(const std::set<Pattern> &patternsToCheck)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return {};
    }
    if (!data.WriteUint32(static_cast<uint32_t>(patternsToCheck.size()))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write size of patterns to check");
        return {};
    }
    for (const auto &pattern : patternsToCheck) {
        if (!data.WriteUint32(static_cast<uint32_t>(pattern))) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write pattern to check");
            return {};
        }
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::DETECT_PATTERNS, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return {};
    }
    uint32_t size = 0;
    if (!reply.ReadUint32(size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to read size of existed patterns");
        return {};
    }
    std::set<Pattern> existedPatterns;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t pattern;
        if (!reply.ReadUint32(pattern)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to read existed pattern");
            return {};
        }
        existedPatterns.insert(static_cast<Pattern>(pattern));
    }
    return existedPatterns;
}

int32_t PasteboardServiceProxy::SetGlobalShareOption(const std::map<uint32_t, ShareOption> &globalShareOptions)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteInterfaceToken failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteUint32(static_cast<uint32_t>(globalShareOptions.size()))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write size failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    for (const auto &[tokenId, shareOption] : globalShareOptions) {
        if (!data.WriteUint32(tokenId)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write tokenId failed.");
            return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
        }
        if (!data.WriteInt32(static_cast<int32_t>(shareOption))) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write shareOption failed.");
            return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
        }
    }
    int32_t result = Remote()->SendRequest(
        PasteboardServiceInterfaceCode::SET_GLOBAL_SHARE_OPTION, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "SendRequest failed, error code: %{public}d.", result);
        return result;
    }
    return reply.ReadInt32();
}

int32_t PasteboardServiceProxy::RemoveGlobalShareOption(const std::vector<uint32_t> &tokenIds)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteInterfaceToken failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteUInt32Vector(tokenIds)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write tokenIds failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    int32_t result = Remote()->SendRequest(
        PasteboardServiceInterfaceCode::REMOVE_GLOBAL_SHARE_OPTION, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "SendRequest failed, error code: %{public}d.", result);
        return result;
    }
    return reply.ReadInt32();
}

std::map<uint32_t, ShareOption> PasteboardServiceProxy::GetGlobalShareOption(const std::vector<uint32_t> &tokenIds)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteInterfaceToken failed.");
        return {};
    }
    if (!data.WriteUInt32Vector(tokenIds)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write tokenIds failed.");
        return {};
    }
    int32_t result = Remote()->SendRequest(
        PasteboardServiceInterfaceCode::GET_GLOBAL_SHARE_OPTION, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "SendRequest failed, error code: %{public}d.", result);
        return {};
    }
    uint32_t size = 0;
    if (!reply.ReadUint32(size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read size failed.");
        return {};
    }
    size_t readAbleSize = reply.GetReadableBytes();
    if (size > readAbleSize || size > MAX_GET_GLOBAL_SHARE_OPTION_SIZE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read oversize failed.");
        return {};
    }
    std::map<uint32_t, ShareOption> globalShareOptions;
    for (uint32_t i = 0; i < size; i++) {
        uint32_t tokenId;
        if (!reply.ReadUint32(tokenId)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read tokenId failed.");
            return {};
        }
        int32_t shareOption;
        if (!reply.ReadInt32(shareOption)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Read shareOption failed.");
            return {};
        }
        globalShareOptions[tokenId] = static_cast<ShareOption>(shareOption);
    }
    return globalShareOptions;
}

int32_t PasteboardServiceProxy::SetAppShareOptions(const ShareOption &shareOptions)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write interface token failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteInt32(shareOptions)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write share options failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    auto result = Remote()->SendRequest(PasteboardServiceInterfaceCode::SET_APP_SHARE_OPTIONS, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Send request failed, error code: %{public}d.", result);
        return result;
    }
    return reply.ReadInt32();
}

int32_t PasteboardServiceProxy::RemoveAppShareOptions()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write interface token failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    auto result = Remote()->SendRequest(PasteboardServiceInterfaceCode::REMOVE_APP_SHARE_OPTIONS, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Send request failed, error code: %{public}d.", result);
        return result;
    }
    return reply.ReadInt32();
}

void PasteboardServiceProxy::PasteStart(const std::string &pasteId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    if (!data.WriteString(pasteId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write pasteId");
        return;
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::PASTE_START, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
}

void PasteboardServiceProxy::PasteComplete(const std::string &deviceId, const std::string &pasteId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    if (!data.WriteString(deviceId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write string");
        return;
    }
    if (!data.WriteString(pasteId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write pasteId");
        return;
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::PASTE_COMPLETE, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
}

int32_t PasteboardServiceProxy::RegisterClientDeathObserver(sptr<IRemoteObject> observer)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "observer is nullptr");
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write interface token failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!data.WriteRemoteObject(observer)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "remote observer failed.");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    auto result = Remote()->SendRequest(
        PasteboardServiceInterfaceCode::REGISTER_CLIENT_DEATH_OBSERVER, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Send request failed, error code: %{public}d.", result);
        return result;
    }
    return reply.ReadInt32();
}

} // namespace MiscServices
} // namespace OHOS