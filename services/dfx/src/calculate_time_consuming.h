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

#ifndef TIME_CONSUMING_H
#define TIME_CONSUMING_H

#include <ctime>
#include <sys/time.h>
#include "singleton.h"

namespace OHOS {
namespace MiscServices {
class CalculateTimeConsuming : public Singleton<CalculateTimeConsuming> {
public:
     CalculateTimeConsuming(const size_t calPasteboardData, const int calPasteboardState);
     ~CalculateTimeConsuming();
private:
    static uint64_t GetCurrentTimeMicros();
    int CalculateTime(uint64_t time);
    int CalculateData(size_t calPasteboardData) const;
    
    int pasteboardData_;
    int pasteboardState_;
    uint64_t lastTime_{ 0 };

    static const inline int M_BTYE = 1024;

    static const inline int TC_ZERO_KB = 0;
    static const inline int TC_HUNDRED_KB = 100;
    static const inline int TC_FIVE_HUNDRED = 500;

    static const inline int TC_ONE_MB = 1;
    static const inline int TC_FIVE_MB = 5;
    static const inline int TC_TEN_MB = 10;
    static const inline int TC_FIFTY_MB = 50;

    static const inline int FIVE_HUNDRED_MS = 500;
    static const inline int PER_FIVE_HUNDRED_MS_ZERO = 0;
    static const inline int PER_FIVE_HUNDRED_MS_ONE = 1;
    static const inline int PER_FIVE_HUNDRED_MS_TWO = 2;
    static const inline int PER_FIVE_HUNDRED_MS_THREE = 3;
    static const inline int PER_FIVE_HUNDRED_MS_FOUR = 4;
    static const inline int PER_FIVE_HUNDRED_MS_FIVE = 5;
    static const inline int PER_FIVE_HUNDRED_MS_SIX = 6;
    static const inline int PER_FIVE_HUNDRED_MS_SEVEN = 7;
    static const inline int PER_FIVE_HUNDRED_MS_EIGHT = 8;
    static const inline int PER_FIVE_HUNDRED_MS_NINE = 9;

    static const inline int64_t SEC_TO_MILLISEC = 1000;
    static const inline int64_t MICROSEC_TO_MILLISEC = 1000;
};
} // namespace MiscServices
} // namespace OHOS
#endif // TIME_CONSUMING_H
