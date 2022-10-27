/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef PASTE_BOARD_PARA_HANDLE_H
#define PASTE_BOARD_PARA_HANDLE_H
#include <string>

#include "parameter.h"

namespace OHOS {
namespace MiscServices {
class ParaHandle {
public:
    static constexpr const char *DISTRIBUTED_PASTEBOARD_ENABLED_KEY = "persist.pasteboard.distributedPasteboardEnabled";
    static ParaHandle &GetInstance();
    std::string GetEnabledStatus() const;
    void Init();
    void WatchEnabledStatus(ParameterChgPtr ptr) const;

private:
    static constexpr const char *DISTRIBUTED_PASTEBOARD_ENABLED_DEFAULT_VALUE = "true";
    static constexpr int CONFIG_LEN = 10;
    ParaHandle();
    virtual ~ParaHandle() = default;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_PARA_HANDLE_H
