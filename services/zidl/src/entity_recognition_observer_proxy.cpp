/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "entity_recognition_observer_proxy.h"
#include "errors.h"
#include "message_option.h"
#include "message_parcel.h"
#include "pasteboard_hilog.h"
#include "pasteboard_serv_ipc_interface_code.h"

using namespace OHOS::Security::PasteboardServ;
namespace OHOS {
namespace MiscServices {
EntityRecognitionObserverProxy::EntityRecognitionObserverProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<IEntityRecognitionObserver>(object)
{
}

void EntityRecognitionObserverProxy::OnRecognitionEvent(EntityType entityType, std::string &entity)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    PASTEBOARD_CHECK_AND_RETURN_LOGE(
        data.WriteInterfaceToken(GetDescriptor()), PASTEBOARD_MODULE_SERVICE, "write descriptor failed!");
    PASTEBOARD_CHECK_AND_RETURN_LOGE(data.WriteString(entity), PASTEBOARD_MODULE_SERVICE, "Write entity failed");
    uint32_t type = static_cast<uint32_t>(entityType);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(data.WriteUint32(type), PASTEBOARD_MODULE_SERVICE, "Write type failed");
    int32_t ret =
        Remote()->SendRequest(static_cast<int>(EntityObserverInterfaceCode::ON_RECOGNITION_EVENT), data, reply, option);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(
        ret == ERR_OK, PASTEBOARD_MODULE_SERVICE, "SendRequest is failed, error code: %{public}d", ret);
    return;
}
} // namespace MiscServices
} // namespace OHOS
