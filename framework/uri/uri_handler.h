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
#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_URI_HANDLER_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_URI_HANDLER_H
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "pasteboard_hilog_wreapper.h"
#include "tlv_object.h"
namespace OHOS::MiscServices {
class UriHandler : public TLVObject {
public:
    explicit UriHandler(int32_t fd) : fd_(fd)
    {
    }
    explicit UriHandler(const std::string &uri) : fd_(INVALID_FD), uri_(uri)
    {
    }
    virtual ~UriHandler()
    {
        if (fd_ >= 0) {
            close(fd_);
        }
    }
    bool IsFile()
    {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "uri:%{public}s", uri_.c_str());
        if (uri_.empty()) {
            return false;
        }
        struct stat fileInfo {};
        if (stat(uri_.c_str(), &fileInfo) == 0 && (fileInfo.st_mode & S_IFREG)) {
            return true;
        }
        return false;
    }
    virtual std::string ToUri() = 0;
    virtual int32_t ToFd() = 0;

protected:
    static constexpr int32_t INVALID_FD = -1;

    std::int32_t fd_;
    std::string uri_;
};
} // namespace OHOS::MiscServices
#endif //DISTRIBUTEDDATAMGR_PASTEBOARD_URI_HANDLER_H
