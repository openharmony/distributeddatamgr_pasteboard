/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "common_utils.h"

namespace OHOS {
namespace MiscServices {
std::string CommonUtils::GetAnonymousString(const std::string& str)
{
    const int32_t ANONYMOUS_LEN_LIMIT = 8;
    const int32_t TWO_TIMES = 2;
    if (str.length() <= ANONYMOUS_LEN_LIMIT) {
        return str;
    }
    int32_t printLen = ANONYMOUS_LEN_LIMIT / TWO_TIMES;
    return str.substr(0, printLen) + "**" + str.substr(str.length() - printLen);
}
} // namespace MiscServices
} // namespace OHOS