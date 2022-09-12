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
#include "server_uri_handler.h"

#include <fcntl.h>
#include <unistd.h>

#include "errors.h"
#include "os_account_manager.h"
#include "pasteboard_hilog_wreapper.h"
#include "remote_file_share.h"
namespace OHOS::MiscServices {
using namespace AppFileService::ModuleRemoteFileShare;
ServerUriHandler::ServerUriHandler(int32_t fd) : UriHandler(fd)
{
}
ServerUriHandler::ServerUriHandler(const std::string &uri) : UriHandler(uri)
{
}
std::string ServerUriHandler::ToUri()
{
    std::vector<int32_t> ids;
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "query active user failed errCode=%{public}d", ret);
        return uri_;
    }
    RemoteFileShare remoteFileShare;
    ret = remoteFileShare.CreateSharePath(fd_, uri_, ids[0]);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, " create share path failed, %{public}d ", ret);
        return uri_;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "share path: %{public}s", uri_.c_str());
    return uri_;
}
int32_t OHOS::MiscServices::ServerUriHandler::ToFd()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "share path: %{public}s", uri_.c_str());
    if (fd_ >= 0) {
        return fd_;
    }
    fd_ = open(uri_.c_str(), O_RDONLY);
    if (fd_ < 0) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "open uri failed, maybe its not a legal file path %{public}s",
            uri_.c_str());
    }
    return fd_;
}
} // namespace OHOS::MiscServices
