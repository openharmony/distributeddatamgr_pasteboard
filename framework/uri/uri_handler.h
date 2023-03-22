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
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace OHOS::MiscServices {
class UriHandler {
public:
    virtual bool IsFile(const std::string &uri) const;
    virtual std::string ToUri(int32_t fd) = 0;
    virtual int32_t ToFd(const std::string &uri, bool isClient);
    virtual void ReleaseFd(int32_t fd);
    bool IsPaste() const;
    static bool GetRealPath(const std::string &inOriPath, std::string &outRealPath);

protected:
    static constexpr int32_t INVALID_FD = -1;
    std::string uri_;
    bool isPaste_ = false;
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_URI_HANDLER_H
