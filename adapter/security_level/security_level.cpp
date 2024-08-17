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
#include "pasteboard_hilog.h"

#include <vector>

namespace OHOS::MiscServices {

SecurityLevel::Init(std::string deviceId)
{
    deviceId_ = std::move(deviceId);
    securityLevel_ = DATA_SEC_LEVEL1;
}

uint32_t SecurityLevel::GetDeviceSecurityLevel()
{
    if (securityLevel_ > DATA_SEC_LEVEL1) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get highest level : %{public}u", securityLevel_);
        return securityLevel_;
    }
    return GetSensitiveLevel();
}

bool SecurityLevel::InitDEVSLQueryParams(DEVSLQueryParams *params, const std::string &udid)
{
    if (params == nullptr || udid.empty()) {
        return false;
    }
    std::vector<uint8_t> vec(udid.begin(), udid.end());
    for (size_t i = 0; i < MAX_UDID_LENGTH && i < vec.size(); i++) {
        params->udid[i] = vec[i];
    }
    params->udidLen = uint32_t(udid.size());
    return true;
}

uint32_t SecurityLevel::GetSensitiveLevel()
{
    DEVSLQueryParams query;
    if (!InitDEVSLQueryParams(&query, deviceId_)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "init query params failed! udid:%{public}.6s", deviceId_.c_str());
        return DATA_SEC_LEVEL1;
    }

    uint32_t level = DATA_SEC_LEVEL1;
    int32_t result = DATASL_GetHighestSecLevel(&query, &level);
    if (result != DEVSL_SUCCESS) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "get highest level failed(%{public}.6s)! level:%{public}u, error:%{public}d",
            deviceId_.c_str(), level, result);
        return DATA_SEC_LEVEL1;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get highest level success(%{public}.6s)! level: %{public}u",
        deviceId_.c_str(), level);
    return level;
}

} // namespace OHOS::MiscServices