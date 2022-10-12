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

#ifndef TIME_CONSUMING_H
#define TIME_CONSUMING_H

#include <cstdint>
#include <ctime>

#include "singleton.h"

namespace OHOS {
namespace MiscServices {
enum class TimeLevel : std::uint64_t {
    PER_FIVE_HUNDRED_MS_ZERO = 0,
    PER_FIVE_HUNDRED_MS_ONE,
    PER_FIVE_HUNDRED_MS_TWO,
    PER_FIVE_HUNDRED_MS_THREE,
    PER_FIVE_HUNDRED_MS_FOUR,
    PER_FIVE_HUNDRED_MS_FIVE,
    PER_FIVE_HUNDRED_MS_SIX,
    PER_FIVE_HUNDRED_MS_SEVEN,
    PER_FIVE_HUNDRED_MS_EIGHT,
    PER_FIVE_HUNDRED_MS_NINE,
};

class CalculateTimeConsuming : public Singleton<CalculateTimeConsuming> {
public:
    CalculateTimeConsuming(const size_t calPasteboardData, const int calPasteboardState);
    ~CalculateTimeConsuming();
    static void SetBeginTime();
private:
    static uint64_t GetCurrentTimeMicros();
    int CalculateTime(uint64_t time);
    int CalculateData(size_t calPasteboardData) const;
    
    int pasteboardData_;
    int pasteboardState_;
    static uint64_t lastTime_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // TIME_CONSUMING_H
