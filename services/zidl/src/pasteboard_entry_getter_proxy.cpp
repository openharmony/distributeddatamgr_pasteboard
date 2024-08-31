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

#include "message_option.h"
#include "message_parcel.h"
#include "pasteboard_entry_getter_proxy.h"
#include "pasteboard_hilog.h"
#include "pasteboard_serv_ipc_interface_code.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::PasteboardServ;
PasteboardEntryGetterProxy::PasteboardEntryGetterProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IPasteboardEntryGetter>(object)
{
}

int32_t PasteboardEntryGetterProxy::MakeRequest(uint32_t recordId, PasteDataEntry& value, MessageParcel& request)
{
    if (!request.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write descriptor failed");
        return ERR_INVALID_VALUE;
    }
    if (!request.WriteUint32(recordId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write recordId failed");
        return ERR_INVALID_VALUE;
    }
    std::vector<uint8_t> sendEntryTLV(0);
    if (!value.Marshalling(sendEntryTLV)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "marshall entry value failed");
        return ERR_INVALID_VALUE;
    }
    if (!request.WriteInt32(sendEntryTLV.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write entry tlv raw data size failed");
        return ERR_INVALID_VALUE;
    }
    if (!request.WriteRawData(sendEntryTLV.data(), sendEntryTLV.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write entry tlv raw data failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t PasteboardEntryGetterProxy::GetRecordValueByType(uint32_t recordId, PasteDataEntry& value)
{
    MessageParcel request;
    auto res = MakeRequest(recordId, value, request);
    if (res != ERR_OK) {
        return res;
    }
    MessageParcel reply;
    MessageOption option;
    int result = Remote()->SendRequest(
        static_cast<int>(PasteboardEntryGetterInterfaceCode::GET_RECORD_VALUE_BY_TYPE), request, reply, option);
    if (result != ERR_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "send request failed, error:%{public}d", result);
        return ERR_INVALID_OPERATION;
    }
    res = reply.ReadInt32();
    int32_t rawDataSize = reply.ReadInt32();
    if (rawDataSize <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "read entry tlv raw data size failed");
        return ERR_INVALID_VALUE;
    }
    const uint8_t *rawData = reinterpret_cast<const uint8_t *>(reply.ReadRawData(rawDataSize));
    if (rawData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "read entry tlv raw data failed");
        return ERR_INVALID_VALUE;
    }
    std::vector<uint8_t> recvEntryTlv(rawData, rawData + rawDataSize);
    PasteDataEntry entryValue;
    if (!entryValue.Unmarshalling(recvEntryTlv)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "unmarshall entry value failed");
        return ERR_INVALID_VALUE;
    }
    value = entryValue;
    return res;
}
} // namespace MiscServices
} // namespace OHOS