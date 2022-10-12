/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#include "calculate_time_consuming.h"

#include <sys/time.h>

#include "dfx_types.h"
#include "pasteboard_hilog.h"
#include "reporter.h"
#include "statistic_reporter.h"

namespace OHOS {
namespace MiscServices {
uint64_t CalculateTimeConsuming::lastTime_ = 0;
void CalculateTimeConsuming::SetBeginTime()
{
    lastTime_ = GetCurrentTimeMicros();
}
CalculateTimeConsuming::CalculateTimeConsuming(const size_t calPasteboardData, const int calPasteboardState)
    : pasteboardState_(calPasteboardState)
{
    pasteboardData_ = CalculateData(calPasteboardData);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "CalculateTimeConsuming()");
}

CalculateTimeConsuming::~CalculateTimeConsuming()
{
    uint64_t delta = GetCurrentTimeMicros() - lastTime_;
    int calculateTime = CalculateTime(delta);
    Reporter::GetInstance().TimeConsumingStatistic().Report({ pasteboardState_, pasteboardData_, calculateTime });
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "~CalculateTimeConsuming()");
}

int CalculateTimeConsuming::CalculateData(size_t calPasteboardData) const
{
    constexpr int M_BTYE = 1024;
    constexpr int TC_ZERO_KB = 0;
    constexpr int TC_HUNDRED_KB = 100;
    constexpr int TC_FIVE_HUNDRED = 500;
    constexpr int TC_ONE_MB = 1;
    constexpr int TC_FIVE_MB = 5;
    constexpr int TC_TEN_MB = 10;
    constexpr int TC_FIFTY_MB = 50;

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "CalculateData() enter");
    if (calPasteboardData % M_BTYE == 0) {
        if (calPasteboardData >= TC_ZERO_KB && calPasteboardData < TC_HUNDRED_KB) {
            return static_cast<int>(DataRange::DR_ZERO_TO_HUNDRED_KB);
        } else if (calPasteboardData >= TC_HUNDRED_KB && calPasteboardData < TC_FIVE_HUNDRED) {
            return static_cast<int>(DataRange::DR_HUNDRED_TO_FIVE_HUNDREDS_KB);
        } else {
            return static_cast<int>(DataRange::DR_FIVE_HUNDREDS_TO_THOUSAND_KB);
        }
    } else {
        size_t dataSize = calPasteboardData % M_BTYE;
        if (dataSize >= TC_ONE_MB && dataSize < TC_FIVE_MB) {
            return static_cast<int>(DataRange::DR_ONE_TO_FIVE_MB);
        } else if (dataSize >= TC_FIVE_MB && dataSize < TC_TEN_MB) {
            return static_cast<int>(DataRange::DR_FIVE_TO_TEN_MB);
        } else if (dataSize >= TC_TEN_MB && dataSize < TC_FIFTY_MB) {
            return static_cast<int>(DataRange::DR_TEN_TO_FIFTY_MB);
        } else {
            return static_cast<int>(DataRange::DR_OVER_FIFTY_MB);
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "CalculateData()  end");
}

int CalculateTimeConsuming::CalculateTime(uint64_t time)
{
    constexpr int FIVE_HUNDRED_MS = 500;
    uint64_t timeCosuming = time % FIVE_HUNDRED_MS;
    switch (timeCosuming) {
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_ZERO):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_ONE);
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_ONE):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_TWO);
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_TWO):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_THREE);
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_THREE):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_FOUR);
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_FOUR):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_FIVE);
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_FIVE):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_SIX);
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_SIX):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_SEVEN);
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_SEVEN):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_EIGHT);
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_EIGHT):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_NINE);
        case static_cast<uint64_t>(TimeLevel::PER_FIVE_HUNDRED_MS_NINE):
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_TEN);
        default:
            return static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_ELEVEN);
    }
}

uint64_t CalculateTimeConsuming::GetCurrentTimeMicros()
{
    constexpr int64_t SEC_TO_MILLISEC = 1000;
    constexpr int64_t MICROSEC_TO_MILLISEC = 1000;

    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "GetCurrentTimeMicros()  start");
    struct timeval tv = { 0, 0 };
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * SEC_TO_MILLISEC + tv.tv_usec / MICROSEC_TO_MILLISEC);
}
} // namespace MiscServices
} // namespace OHOS
