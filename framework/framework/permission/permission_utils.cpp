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

#include "permission/permission_utils.h"

#include "accesstoken_kit.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
bool PermissionUtils::IsPermissionGranted(const std::string &perm, uint32_t tokenId)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "check permission, perm=%{public}s", perm.c_str());
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(tokenId, perm);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret == Security::AccessToken::PermissionState::PERMISSION_GRANTED,
        false, PASTEBOARD_MODULE_SERVICE, "permission denied, perm=%{public}s", perm.c_str());
    return true;
}
} // namespace MiscServices
} // namespace OHOS