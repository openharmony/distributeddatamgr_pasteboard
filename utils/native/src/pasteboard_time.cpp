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
static constexpr uint64_t SEC_TO_MILLISEC = 1000;
static constexpr uint64_t MICROSEC_TO_MILLISEC = 1000;
static constexpr int64_t MILLI_TO_SEC = 1000;
static constexpr int64_t NANO_TO_SEC = 1000000000;
static constexpr int64_t NANO_TO_MILLI = NANO_TO_SEC / MILLI_TO_SEC;

static int64_t GetTimeMsByClockId(clockid_t clockId)
{
    int64_t time = 0;
    struct timespec tv = { 0 };
    if (clock_gettime(clockId, &tv) < 0) {
        return -1;
    }
    return tv.tv_sec * MILLI_TO_SEC + tv.tv_nsec / NANO_TO_MILLI;
}
}

uint64_t PasteBoardTime::GetCurrentTimeMicros(void)
{
    struct timeval tv = { 0, 0 };
    gettimeofday(&tv, nullptr);
    return (static_cast<uint64_t>(tv.tv_sec) * SEC_TO_MILLISEC +
        static_cast<uint64_t>(tv.tv_usec) / MICROSEC_TO_MILLISEC);
}

int64_t PasteBoardTime::GetBootTimeMs(void)
{
    return GetTimeMsByClockId(CLOCK_BOOTTIME);
}

int64_t PasteBoardTime::GetWallTimeMs(void)
{
    return GetTimeMsByClockId(CLOCK_REALTIME);
}
} // namespace MiscServices
} // namespace OHOS
