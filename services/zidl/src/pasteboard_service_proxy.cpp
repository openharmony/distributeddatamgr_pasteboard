/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "iremote_broker.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "paste_uri_handler.h"
#include "copy_uri_handler.h"

namespace OHOS {
namespace MiscServices {
PasteboardServiceProxy::PasteboardServiceProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IPasteboardService>(object)
{
}

void PasteboardServiceProxy::Clear()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data, reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }

    int32_t result = Remote()->SendRequest(CLEAR_ALL, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
}

bool PasteboardServiceProxy::HasPasteData()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data, reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return false;
    }

    int32_t result = Remote()->SendRequest(HAS_PASTE_DATA, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return false;
    }
    auto has = reply.ReadBool();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return has;
}

int32_t PasteboardServiceProxy::SetPasteData(PasteData &pasteData)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data, reply;
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

    int32_t result = Remote()->SendRequest(SET_PASTE_DATA, data, reply, option);
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
    MessageParcel data, reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return ERR_INVALID_VALUE;
    }
    int32_t result = Remote()->SendRequest(GET_PASTE_DATA, data, reply, option);
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
    ProcessObserver(ADD_CHANGED_OBSERVER, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    ProcessObserver(DELETE_CHANGED_OBSERVER, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}
void PasteboardServiceProxy::RemoveAllChangedObserver()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data, reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    int32_t result = Remote()->SendRequest(DELETE_ALL_CHANGED_OBSERVER, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::AddPasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    ProcessObserver(ADD_EVENT_OBSERVER, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::RemovePasteboardEventObserver(const sptr<IPasteboardChangedObserver> &observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    ProcessObserver(DELETE_EVENT_OBSERVER, observer);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}
void PasteboardServiceProxy::RemoveAllEventObserver()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data, reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    int32_t result = Remote()->SendRequest(DELETE_ALL_EVENT_OBSERVER, data, reply, option);
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
    MessageParcel data, reply;
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

} // namespace MiscServices
} // namespace OHOS