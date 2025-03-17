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

#include "pasteboard_entry_getter_proxy.h"

#include "message_parcel_warp.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_serv_ipc_interface_code.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::PasteboardServ;
PasteboardEntryGetterProxy::PasteboardEntryGetterProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IPasteboardEntryGetter>(object)
{
}

int32_t PasteboardEntryGetterProxy::MakeRequest(uint32_t recordId, PasteDataEntry &value, MessageParcel &request)
{
    if (!request.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write descriptor failed");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!request.WriteUint32(recordId)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write recordId failed");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    std::vector<uint8_t> sendEntryTLV(0);
    if (!value.Encode(sendEntryTLV)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "marshall entry value failed");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    if (!request.WriteInt64(sendEntryTLV.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write entry tlv raw data size failed");
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    MessageParcelWarp messageRequest;
    size_t tlvSize = sendEntryTLV.size();
    if (!messageRequest.WriteRawData(request, sendEntryTLV.data(), tlvSize)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write entry tlv raw data failed size:%{public}zu", tlvSize);
        return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteboardEntryGetterProxy::GetRecordValueByType(uint32_t recordId, PasteDataEntry &value)
{
    MessageParcel request;
    auto res = MakeRequest(recordId, value, request);
    if (res != static_cast<int32_t>(PasteboardError::E_OK)) {
        return res;
    }
    MessageParcel reply;
    MessageOption option;
    int result = Remote()->SendRequest(
        static_cast<int>(PasteboardEntryGetterInterfaceCode::GET_RECORD_VALUE_BY_TYPE), request, reply, option);
    if (result != ERR_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "send request failed, error:%{public}d", result);
        return result;
    }
    res = reply.ReadInt32();
    int64_t rawDataSize = reply.ReadInt64();
    if (rawDataSize <= 0 || rawDataSize > MessageParcelWarp::maxRawDataSize) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "read entry tlv raw data size failed");
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR);
    }
    MessageParcelWarp messageReply;
    const uint8_t *rawData = reinterpret_cast<const uint8_t *>(messageReply.ReadRawData(reply, rawDataSize));
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(rawData != nullptr,
        static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR),
        PASTEBOARD_MODULE_SERVICE, "read entry tlv raw data failed, size=%{public}" PRId64, rawDataSize);
    std::vector<uint8_t> recvEntryTlv(rawData, rawData + rawDataSize);
    PasteDataEntry entryValue;
    if (!entryValue.Decode(recvEntryTlv)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "unmarshall entry value failed");
        return static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR);
    }
    value = entryValue;
    return res;
}
} // namespace MiscServices
} // namespace OHOS