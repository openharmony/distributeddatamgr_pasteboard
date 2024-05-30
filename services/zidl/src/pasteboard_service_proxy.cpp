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

int32_t PasteboardServiceProxy::SetPasteData(PasteData &pasteData, const sptr<IPasteboardDelayGetter> delayGetter)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return ERR_INVALID_VALUE;
    }
    std::vector<uint8_t> pasteDataTlv(0);
    bool ret = pasteData.Encode(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to encode pastedata in TLV");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteInt32(pasteDataTlv.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write raw size");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteRawData(pasteDataTlv.data(), pasteDataTlv.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write raw data");
        return ERR_INVALID_VALUE;
    }
    CopyUriHandler copyHandler;
    if (!pasteData.WriteUriFd(data, copyHandler)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write record uri fd");
        return ERR_INVALID_VALUE;
    }
    if (pasteData.IsDelayData() &&
        !data.WriteRemoteObject(delayGetter->AsObject())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed to write delay getter");
        return ERR_INVALID_VALUE;
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::SET_PASTE_DATA, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return ERR_INVALID_OPERATION;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return reply.ReadInt32();
}

int32_t PasteboardServiceProxy::GetPasteData(PasteData &pasteData)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return ERR_INVALID_VALUE;
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::GET_PASTE_DATA, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return ERR_INVALID_OPERATION;
    }
    int32_t rawDataSize = reply.ReadInt32();
    if (rawDataSize <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to get raw size");
        return ERR_INVALID_VALUE;
    }
    auto *rawData = (uint8_t *)reply.ReadRawData(rawDataSize);
    if (rawData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to get raw data");
        return ERR_INVALID_VALUE;
    }
    std::vector<uint8_t> pasteDataTlv(rawData, rawData + rawDataSize);
    bool ret = pasteData.Decode(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to decode pastedata in TLV");
        return ERR_INVALID_VALUE;
    }
    PasteUriHandler pasteHandler;
    if (!pasteData.ReadUriFd(reply, pasteHandler)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write record uri fd");
        return ERR_INVALID_VALUE;
    }

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return reply.ReadInt32();
}

void PasteboardServiceProxy::AddPasteboardChangedObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    ProcessObserver(PasteboardServiceInterfaceCode::ADD_CHANGED_OBSERVER, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    ProcessObserver(PasteboardServiceInterfaceCode::DELETE_CHANGED_OBSERVER, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}
void PasteboardServiceProxy::RemoveAllChangedObserver()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    int32_t result =
        Remote()->SendRequest(PasteboardServiceInterfaceCode::DELETE_ALL_CHANGED_OBSERVER, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::AddPasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    ProcessObserver(PasteboardServiceInterfaceCode::ADD_EVENT_OBSERVER, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::RemovePasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    ProcessObserver(PasteboardServiceInterfaceCode::DELETE_EVENT_OBSERVER, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}
void PasteboardServiceProxy::RemoveAllEventObserver()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    int32_t result =
       Remote()->SendRequest(PasteboardServiceInterfaceCode::DELETE_ALL_EVENT_OBSERVER, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::ProcessObserver(uint32_t code, const sptr<IPasteboardChangedObserver> &observer)
{
    if (observer == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "observer nullptr");
        return;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    if (!data.WriteRemoteObject(observer->AsObject())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
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
        return ERR_INVALID_VALUE;
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::GET_DATA_SOURCE, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return ERR_INVALID_OPERATION;
    }
    bundleName = reply.ReadString();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return reply.ReadInt32();
}

bool PasteboardServiceProxy::HasDataType(const std::string &mimeType)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteString(mimeType)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write string");
        return ERR_INVALID_VALUE;
    }
    int32_t result = Remote()->SendRequest(PasteboardServiceInterfaceCode::HAS_DATA_TYPE, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return ERR_INVALID_OPERATION;
    }

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return reply.ReadBool();
}

int32_t PasteboardServiceProxy::SetGlobalShareOption(const std::map<uint32_t, ShareOption> &globalShareOptions)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "WriteInterfaceToken failed.");
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteUint32(static_cast<uint32_t>(globalShareOptions.size()))) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write size failed.");
        return ERR_INVALID_VALUE;
    }
    for (const auto &[tokenId, shareOption] : globalShareOptions) {
        if (!data.WriteUint32(tokenId)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write tokenId failed.");
            return ERR_INVALID_VALUE;
        }
        if (!data.WriteInt32(static_cast<int32_t>(shareOption))) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write shareOption failed.");
            return ERR_INVALID_VALUE;
        }
    }
    int32_t result = Remote()->SendRequest(
        PasteboardServiceInterfaceCode::SET_GLOBAL_SHARE_OPTION, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "SendRequest failed, error code: %{public}d.", result);
        return ERR_INVALID_OPERATION;
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
        return ERR_INVALID_VALUE;
    }
    if (!data.WriteUInt32Vector(tokenIds)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write tokenIds failed.");
        return ERR_INVALID_VALUE;
    }
    int32_t result = Remote()->SendRequest(
        PasteboardServiceInterfaceCode::REMOVE_GLOBAL_SHARE_OPTION, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "SendRequest failed, error code: %{public}d.", result);
        return ERR_INVALID_OPERATION;
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
        return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
    }
    if (!data.WriteInt32(shareOptions)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Write share options failed.");
        return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
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
        return static_cast<int32_t>(PasteboardError::E_WRITE_PARCEL_ERROR);
    }
    auto result = Remote()->SendRequest(PasteboardServiceInterfaceCode::REMOVE_APP_SHARE_OPTIONS, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Send request failed, error code: %{public}d.", result);
        return result;
    }
    return reply.ReadInt32();
}
} // namespace MiscServices
} // namespace OHOS