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
#include "uri_handler.h"

#include "pasteboard_hilog.h"
namespace OHOS::MiscServices {
constexpr const int32_t ERROR_FD = -1;
bool UriHandler::IsFile(const std::string &uri) const
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "uri:%{public}s", uri.c_str());
    if (uri.empty()) {
        return false;
    }
    struct stat fileInfo {};
    if (stat(uri.c_str(), &fileInfo) == 0 && (fileInfo.st_mode & S_IFREG)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "valid uri");
        return true;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "not file");
    return false;
}
int32_t UriHandler::ToFd(const std::string &uri)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "uri: %{public}s", uri.c_str());
    std::string fileRealPath;
    if (!GetRealPath(uri, fileRealPath)) {
        return ERROR_FD;
    }
    int32_t fd = open(fileRealPath.c_str(), O_RDONLY);
    if (fd < 0) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "open file failed, maybe its not a legal file path %{public}s",
            fileRealPath.c_str());
    }
    return fd;
}

bool UriHandler::GetRealPath(const std::string &inOriPath, std::string &outRealPath)
{
    char realPath[PATH_MAX + 1] = {0x00};
    if (inOriPath.size() > PATH_MAX || realpath(inOriPath.c_str(), realPath) == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get real path failed, len = %{public}zu, errno = %{public}d.",
            inOriPath.size(), errno);
        return false;
    }
    outRealPath = std::string(realPath);
    return true;
}

void UriHandler::ReleaseFd(int32_t fd)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "close fd: %{public}d", fd);
    if (fd >= 0) {
        close(fd);
    }
}
} // namespace OHOS::MiscServices