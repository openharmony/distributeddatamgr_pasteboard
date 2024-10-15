/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ffrtutils_fuzzer.h"

#include <vector>
#include "ffrt_utils.h"

namespace OHOS {
using namespace OHOS::MiscServices;
using namespace std;

template<class T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

bool FuzzFfrtUtilsSubmitQueueTasks(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t) + sizeof(int32_t)) {
        return true;
    }
    int pos = 0;
    int x = TypeCast<int32_t>(rawData, &pos);
    int y = TypeCast<int32_t>(rawData + pos);
    FFRTTask task = [&] {
        x /= 2;
    };
    FFRTTask task1 = [&] {
        y /= 2;
    };
    static uint32_t thresHold = 1;
    if (thresHold <= 1) {
        std::vector<FFRTTask> tasks;
        tasks.push_back(task);
        tasks.push_back(task1);
        FFRTQueue serialQueue("pasteboard_queue");
        FFRTUtils::SubmitQueueTasks(tasks, serialQueue);
        thresHold++;
    }
    return true;
}

bool FuzzFfrtUtilsSubmitDelayTask(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    static uint32_t thresHold = 1;
    if (thresHold <= 1) {
        int y = TypeCast<int32_t>(rawData);
        FFRTTask task = [&] {
            y /= 2;
        };
        uint32_t delayMs = 5; // delay 5Ms
        FFRTQueue serQueue("delaytask_queue");
        FFRTUtils::SubmitDelayTask(task, delayMs, serQueue);
        thresHold++;
    }
    return true;
}

bool FuzzUtilsSubmitDelayTask(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    int z = TypeCast<int32_t>(rawData, nullptr);
    FFRTTask task = [&] {
        z /= 2;
    };
    uint32_t delayMs = 5; // delay 5Ms
    static uint32_t thresHold = 1;
    if (thresHold <= 1) {
        shared_ptr<FFRTQueue> queue = make_shared<FFRTQueue>("delaytask_queue_ptr");
        FFRTUtils::SubmitDelayTask(task, delayMs,  queue);
        thresHold++;
    }
    return true;
}

bool FuzzUtilsSubmitTimeoutTask(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    int m = TypeCast<int32_t>(rawData, nullptr);
    FFRTTask task = [&] {
        m /= 2;
    };
    static uint32_t thresHold = 1;
    if (thresHold <= 1) {
        uint32_t timeout = 10; // timeout 10Ms
        FFRTUtils::SubmitTimeoutTask(task, timeout);
        thresHold++;
    }
    return true;
}

bool FuzzFfRTTimerSingleSetAndCancel(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    int pos = 0;
    int m = TypeCast<int32_t>(rawData, &pos);
    FFRTTask task = [&] {
        m /= 2;
    };

    string timerId(reinterpret_cast<const char*>(rawData + pos), size - pos);
    FFRTTimer ffrtTimer;
    ffrtTimer.SetTimer(timerId, task);
    ffrtTimer.CancelTimer(timerId);
    return true;
}

bool FuzzFfRTTimerMultiSetAndCancel(const uint8_t *rawData, size_t size)
{
    int n = 0;
    FFRTTask task = [&] {
        n = 2;
    };
    FFRTTask task1 = [&] {
        n = 5;
    };
    int len = size >> 1;
    string timerId1 = "TimerMultiSetAndCancel1";
    string timerId2 = "TimerMultiSetAndCancel2";
    static uint32_t thresHold = 1;
    if (thresHold <= 1) {
        FFRTTimer ffrtTimer("fuzz_test_timername");
        uint32_t delayMs = 5; // delay 5Ms
        ffrtTimer.SetTimer(timerId1, task, delayMs);
        ffrtTimer.SetTimer(timerId2, task1, delayMs);
        ffrtTimer.CancelAllTimer();
        thresHold++;
    }
    return true;
}
} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzFfrtUtilsSubmitQueueTasks(data, size);
    OHOS::FuzzFfrtUtilsSubmitDelayTask(data, size);
    OHOS::FuzzUtilsSubmitDelayTask(data, size);
    OHOS::FuzzUtilsSubmitTimeoutTask(data, size);
    OHOS::FuzzFfRTTimerSingleSetAndCancel(data, size);
    OHOS::FuzzFfRTTimerMultiSetAndCancel(data, size);
    return 0;
}
