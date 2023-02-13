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
#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_COPY_URI_HANDLER_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_COPY_URI_HANDLER_H
#include "uri_handler.h"
namespace OHOS::MiscServices {
class CopyUriHandler : public UriHandler {
public:
    CopyUriHandler();
    std::string ToUri(int32_t fd) override;
private:
    static constexpr int32_t FILE_EXIST = 17;
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_COPY_URI_HANDLER_H
