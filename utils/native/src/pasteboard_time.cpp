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

#include "pasteboard_time.h"

#include <sys/time.h>

namespace OHOS {
namespace MiscServices {
namespace {
constexpr uint64_t SEC_TO_MILLISEC = 1000;
constexpr uint64_t MICROSEC_TO_MILLISEC = 1000;
}
uint64_t PasteBoardTime::GetCurrentTimeMicros(void)
{
    struct timeval tv = { 0, 0 };
    gettimeofday(&tv, nullptr);
    return (static_cast<uint64_t>(tv.tv_sec) * SEC_TO_MILLISEC +
        static_cast<uint64_t>(tv.tv_usec) / MICROSEC_TO_MILLISEC);
}
} // namespace MiscServices
} // namespace OHOS
