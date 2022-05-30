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
#include "pasteboard_common.h"

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

void PasteboardServiceProxy::AddPasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
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
    int32_t result = Remote()->SendRequest(ADD_OBSERVER, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

void PasteboardServiceProxy::RemovePasteboardChangedObserver(const sptr<IPasteboardChangedObserver>& observer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
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
    int32_t result = Remote()->SendRequest(DELETE_OBSERVER, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
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
    int32_t result = Remote()->SendRequest(DELETE_ALL_OBSERVER, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
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

void PasteboardServiceProxy::SetPasteData(PasteData& pasteData)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data, reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return;
    }
    if (!data.WriteParcelable(&pasteData)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable subscribeInfo");
        return;
    }
    int32_t result = Remote()->SendRequest(SET_PASTE_DATA, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
}

bool PasteboardServiceProxy::GetPasteData(PasteData& pasteData)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "start.");
    MessageParcel data, reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to write parcelable");
        return false;
    }
    int32_t result = Remote()->SendRequest(GET_PASTE_DATA, data, reply, option);
    if (result != ERR_NONE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed, error code is: %{public}d", result);
        return false;
    }
    std::unique_ptr<PasteData> pasteInfo(reply.ReadParcelable<PasteData>());
    if (pasteInfo == nullptr) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "nullptr end.");
        return false;
    }
    pasteData = *pasteInfo;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end.");
    return true;
}
} // namespace MiscServices
} // namespace OHOS