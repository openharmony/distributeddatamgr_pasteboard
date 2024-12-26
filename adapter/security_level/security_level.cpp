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
#include "security_level.h"
#include <vector>

#include "device/dm_adapter.h"
#include "pasteboard_hilog.h"

namespace OHOS::MiscServices {
uint32_t SecurityLevel::GetDeviceSecurityLevel()
{
    if (securityLevel_ > DATA_SEC_LEVEL0) {
        return securityLevel_;
    }
    return GetSensitiveLevel();
}

bool SecurityLevel::InitDEVSLQueryParams(DEVSLQueryParams *params, const std::string &udid)
{
    if (params == nullptr || udid.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "params check failed, params is null %{public}d", params == nullptr);
        return false;
    }
    std::vector<uint8_t> vec(udid.begin(), udid.end());
    for (size_t i = 0; i < MAX_UDID_LENGTH && i < vec.size(); i++) {
        params->udid[i] = vec[i];
    }
    params->udidLen = static_cast<uint32_t>(MAX_UDID_LENGTH);
    return true;
}

uint32_t SecurityLevel::GetSensitiveLevel()
{
    auto &udid = DMAdapter::GetInstance().GetLocalDeviceUdid();
    DEVSLQueryParams query;
    if (!InitDEVSLQueryParams(&query, udid)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "init query params failed! udid:%{public}.6s", udid.c_str());
        return DATA_SEC_LEVEL0;
    }

    uint32_t level = DATA_SEC_LEVEL0;
    int32_t result = DATASL_GetHighestSecLevel(&query, &level);
    if (result != DEVSL_SUCCESS) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "get highest level failed(%{public}.6s)! level:%{public}u, error:%{public}d", udid.c_str(), level, result);
        return DATA_SEC_LEVEL0;
    }
    securityLevel_ = level;
    PASTEBOARD_HILOGI(
        PASTEBOARD_MODULE_SERVICE, "get highest level success(%{public}.6s)! level: %{public}u", udid.c_str(), level);
    return level;
}
} // namespace OHOS::MiscServices