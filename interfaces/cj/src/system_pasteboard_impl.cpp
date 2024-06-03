/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "system_pasteboard_impl.h"
#include "pasteboard_client.h"
 
using namespace OHOS::MiscServices;
 
namespace OHOS {
namespace MiscServicesCj {
 
static sptr<SystemPasteboardImpl> g_systemPasteboard_instance = nullptr;

OHOS::FFI::RuntimeType *SystemPasteboardImpl::GetClassType()
{
    static OHOS::FFI::RuntimeType runtimeType =
        OHOS::FFI::RuntimeType::Create<OHOS::FFI::FFIData>("SystemPasteboardImpl");
    return &runtimeType;
}
 
SystemPasteboardImpl::SystemPasteboardImpl()
{
    value_ = nullptr;
    pasteData_ = nullptr;
}

int NewInstance(sptr<SystemPasteboardImpl> &instance)
{
    if (g_systemPasteboard_instance != nullptr) {
        instance = g_systemPasteboard_instance;
        return 0;
    }
    g_systemPasteboard_instance = FFI::FFIData::Create<SystemPasteboardImpl>();
    instance = g_systemPasteboard_instance;
    return 0;
}

int32_t SystemPasteboardImpl::GetSystemPasteboardImpl(int64_t &id)
{
    sptr<SystemPasteboardImpl> instence = nullptr;
    int status = NewInstance(instence);
    if (status != 0) {
        LOGE("[SystemPasteboardImpl] CJgetSystemPasteboard create instance failed");
        return status;
    }
    id = instence->GetID();
    return 0;
}

int32_t SystemPasteboardImpl::SetData(sptr<PasteDataImpl> dataImpl, std::shared_ptr<MiscServices::PasteData> data)
{
    if (data == nullptr) {
        LOGE("[SystemPasteboardImpl] SetData data nullptr");
        return PASTEBOARD_INVALID_PARAMETERS;
    }
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*data);
    int32_t res = PASTEBOARD_SUCCESS;
    if (ret == static_cast<int>(PasteboardError::E_OK)) {
        value_ = dataImpl;
        pasteData_ = data;
        LOGI("[SystemPasteboardImpl] SetData OK");
    } else if (ret == static_cast<int>(PasteboardError::E_COPY_FORBIDDEN)) {
        res = PASTEBOARD_COPY_FORBIDDEN;
        LOGE("[SystemPasteboardImpl] SetData ERR E_COPY_FORBIDDEN");
    } else if (ret == static_cast<int>(PasteboardError::E_IS_BEGING_PROCESSED)) {
        LOGE("[SystemPasteboardImpl] SetData ERR E_IS_BEGING_PROCESSED");
        res = PASTEBOARD_IS_BEGING_PROCESSED;
    }

    return res;
}

sptr<PasteDataImpl> SystemPasteboardImpl::GetSystemPasteboardPasteDataImpl()
{
    if (value_ == nullptr) {
        return nullptr;
    }

    return value_;
}

int32_t SystemPasteboardImpl::GetData(MiscServices::PasteData &pasteData)
{
    int32_t ret = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    return ret;
}

bool SystemPasteboardImpl::HasData()
{
    bool res = PasteboardClient::GetInstance()->HasPasteData();
    return res;
}

void SystemPasteboardImpl::ClearData()
{
    PasteboardClient::GetInstance()->Clear();
    value_ = nullptr;
    pasteData_ = nullptr;
}

bool SystemPasteboardImpl::IsRemoteData()
{
    bool res = PasteboardClient::GetInstance()->IsRemoteData();
    return res;
}

bool SystemPasteboardImpl::HasDataType(std::string mimeType)
{
    bool res = PasteboardClient::GetInstance()->HasDataType(mimeType);
    return res;
}

std::string SystemPasteboardImpl::GetDataSource()
{
    std::string res;
    PasteboardClient::GetInstance()->GetDataSource(res);
    return res;
}

}
}