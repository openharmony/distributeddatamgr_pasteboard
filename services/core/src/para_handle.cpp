/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "para_handle.h"

#include "pasteboard_hilog.h"
namespace OHOS {
namespace MiscServices {
constexpr int32_t HANDLE_OK = 0;
ParaHandle::ParaHandle()
{
}

ParaHandle &ParaHandle::GetInstance()
{
    static ParaHandle instance;
    return instance;
}

void ParaHandle::Init()
{
    auto status = GetEnabledStatus();
    if (!status.empty()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "local device param already been set");
        return;
    }

    auto errNo = SetParameter(DISTRIBUTED_PASTEBOARD_ENABLED_KEY, DISTRIBUTED_PASTEBOARD_ENABLED_DEFAULT_VALUE);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "SetParameter, errNo = %{public}d.", errNo);
}

void ParaHandle::WatchEnabledStatus(ParameterChgPtr ptr) const
{
    auto errNo = WatchParameter(DISTRIBUTED_PASTEBOARD_ENABLED_KEY, ptr, nullptr);
    if (errNo != HANDLE_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "local device param watch failed, %{public}d", errNo);
    }
}

std::string ParaHandle::GetEnabledStatus() const
{
    char value[CONFIG_LEN] = { 0 };
    auto errNo = GetParameter(DISTRIBUTED_PASTEBOARD_ENABLED_KEY, "", value, CONFIG_LEN);
    if (errNo > HANDLE_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetParameter success, value = %{public}s.", value);
        return value;
    }
    return "";
}
} // namespace MiscServices
} // namespace OHOS
