/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "entity_recognition_observer_stub.h"

#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_serv_ipc_interface_code.h"

using namespace OHOS::Security::PasteboardServ;
namespace OHOS {
namespace MiscServices {
EntityRecognitionObserverStub::EntityRecognitionObserverStub()
{
    memberFuncMap_[static_cast<uint32_t>(EntityObserverInterfaceCode::ON_RECOGNITION_EVENT)] =
        &EntityRecognitionObserverStub::OnRecognitionEventStub;
}

int32_t EntityRecognitionObserverStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start##code = %{public}u", code);
    std::u16string myDescriptor = EntityRecognitionObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "end##descriptor checked fail");
        return static_cast<int32_t>(PasteboardError::CHECK_DESCRIPTOR_ERROR);
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "end##ret = %{public}d", ret);
    return ret;
}

int32_t EntityRecognitionObserverStub::OnRecognitionEventStub(MessageParcel &data, MessageParcel &reply)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "start.");
    std::string entity = data.ReadString();
    uint32_t type = 0;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        data.ReadUint32(type), ERR_INVALID_VALUE, PASTEBOARD_MODULE_CLIENT, "Failed to read type");
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(type < static_cast<uint32_t>(EntityType::MAX), ERR_INVALID_VALUE,
        PASTEBOARD_MODULE_CLIENT, "type invalid, type=%{public}u", type);
    EntityType entityType = static_cast<EntityType>(type);
    OnRecognitionEvent(entityType, entity);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "end.");
    return ERR_OK;
}

EntityRecognitionObserverStub::~EntityRecognitionObserverStub()
{
    memberFuncMap_.clear();
}
} // namespace MiscServices
} // namespace OHOS
