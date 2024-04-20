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
#include "pasteboard_delay_getter_proxy.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "paste_uri_handler.h"

namespace OHOS {
namespace MiscServices {
PasteboardDelayGetterProxy::PasteboardDelayGetterProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IPasteboardDelayGetter>(object)
{
}

void PasteboardDelayGetterProxy::GetPasteData(const std::string &type, PasteData &data)
{
    MessageParcel request;
    MessageParcel reply;
    MessageOption option;
    if (!request.WriteInterfaceToken(GetDescriptor())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write descriptor failed");
        return;
    }
    if (!request.WriteString(type)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "write type failed");
        return;
    }
    int result = Remote()->SendRequest(TransId::TRANS_GET_PASTE_DATA, request, reply, option);
    if (result != ERR_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "send request fail, error:%{public}d", result);
        return;
    }
    int32_t rawDataSize = reply.ReadInt32();
    if (rawDataSize <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fail to get data size");
        return;
    }
    const uint8_t *rawData = reinterpret_cast<const uint8_t *>(reply.ReadRawData(rawDataSize));
    if (rawData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fail to get raw data");
        return;
    }
    std::vector<uint8_t> pasteDataTlv(rawData, rawData + rawDataSize);
    bool ret = data.Decode(pasteDataTlv);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fail to decode paste data");
        return;
    }
    PasteUriHandler pasteHandler;
    if (!data.ReadUriFd(reply, pasteHandler)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "failed to write record uri fd");
        return;
    }
}

void PasteboardDelayGetterProxy::GetUnifiedData(const std::string &type, UDMF::UnifiedData &data)
{
}
} // namespace MiscServices
} // namespace OHOS