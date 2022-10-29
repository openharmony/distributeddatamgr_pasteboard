/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "paste_uri_handler.h"

#include "pasteboard_hilog.h"
#include "remote_uri.h"
namespace OHOS::MiscServices {
PasteUriHandler::PasteUriHandler()
{
    isPaste_ = true;
}
std::string PasteUriHandler::ToUri(int32_t fd)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "begin");
    if (fd < 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fd not available");
        return "";
    }
    std::string result;
    int ret = DistributedFS::ModuleRemoteUri::RemoteUri::ConvertUri(fd, result); // transfer fd ownership
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "convert uri from fd failed: %{public}d", ret);
        return result;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "end, %{public}s", result.c_str());
    return result;
}
} // namespace OHOS::MiscServices
