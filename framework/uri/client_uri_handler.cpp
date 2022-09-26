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
#include "client_uri_handler.h"

#include <fcntl.h>
#include <unistd.h>

#include "pasteboard_hilog_wreapper.h"
#include "remote_uri.h"
namespace OHOS::MiscServices {
ClientUriHandler::ClientUriHandler(int32_t fd) : UriHandler(fd)
{
}
ClientUriHandler::ClientUriHandler(const std::string &uri) : UriHandler(uri)
{
}
std::string ClientUriHandler::ToUri()
{
    if (fd_ < 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fd not available");
        return "";
    }
    std::string result;
    int ret = DistributedFS::ModuleRemoteUri::RemoteUri::ConvertUri(fd_, result);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "convert uri from fd failed: %{public}d", ret);
        return result;
    }
    return result;
}
int32_t OHOS::MiscServices::ClientUriHandler::ToFd()
{
    if (fd_ >= 0) {
        return fd_;
    }
    fd_ = open(uri_.c_str(), O_RDONLY);
    if (fd_ < 0) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "open uri failed, maybe its not a legal file path %{public}s",
            uri_.c_str());
    }
    return fd_;
}
} // namespace OHOS::MiscServices
