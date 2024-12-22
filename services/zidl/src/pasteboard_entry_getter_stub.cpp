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

#include "ipc_skeleton.h"
#include "pasteboard_entry_getter_stub.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_serv_ipc_interface_code.h"

#define MAX_RAWDATA_SIZE (128 * 1024 * 1024)

namespace OHOS {
namespace MiscServices {
using namespace OHOS::Security::PasteboardServ;
PasteboardEntryGetterStub::PasteboardEntryGetterStub()
{
    memberFuncMap_[static_cast<uint32_t>(PasteboardEntryGetterInterfaceCode::GET_RECORD_VALUE_BY_TYPE)] =
        &PasteboardEntryGetterStub::OnGetRecordValueByType;
}

PasteboardEntryGetterStub::~PasteboardEntryGetterStub()
{
    memberFuncMap_.clear();
}

int PasteboardEntryGetterStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    PASTEBOARD_HILOGI(
        PASTEBOARD_MODULE_CLIENT, "code:%{public}u, callingPid:%{public}d", code, IPCSkeleton::GetCallingPid());
    std::u16string localDescriptor = PasteboardEntryGetterStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (remoteDescriptor != localDescriptor) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "remote descriptor is not equal to local descriptor");
        return -1;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t PasteboardEntryGetterStub::OnGetRecordValueByType(MessageParcel &data, MessageParcel &reply)
{
    uint32_t recordId = data.ReadUint32();
    int32_t rawDataSize = data.ReadInt32();
    if (rawDataSize <= 0 || rawDataSize > MAX_RAWDATA_SIZE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "invalid raw data size");
        return ERR_INVALID_VALUE;
    }
    const uint8_t *rawData = reinterpret_cast<const uint8_t *>(data.ReadRawData(rawDataSize));
    if (rawData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "read entry tlv raw data fail");
        return ERR_INVALID_VALUE;
    }
    std::vector<uint8_t> recvEntryTlv(rawData, rawData + rawDataSize);
    PasteDataEntry entryValue;
    bool ret = entryValue.Unmarshalling(recvEntryTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "unmarshall entry value failed");
        return ERR_INVALID_VALUE;
    }
    auto result = GetRecordValueByType(recordId, entryValue);
    if (!reply.WriteInt32(result)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Failed to write result:%{public}d", result);
        return ERR_INVALID_VALUE;
    }
    std::vector<uint8_t> sendEntryTLV(0);
    ret = entryValue.Marshalling(sendEntryTLV);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "marshall entry value failed");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteInt32(sendEntryTLV.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write entry tlv raw data size failed");
        return ERR_INVALID_VALUE;
    }
    if (!reply.WriteRawData(sendEntryTLV.data(), sendEntryTLV.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "write entry tlv raw data failed");
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}
} // namespace MiscServices
} // namespace OHOS