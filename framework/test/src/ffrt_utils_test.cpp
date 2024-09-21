/*
* Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ffrt_utils.h"
#include <gtest/gtest.h>

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;

class FFRTTimerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FFRTTimerTest::SetUpTestCase(void)
{
}

void FFRTTimerTest::TearDownTestCase(void)
{
}

void FFRTTimerTest::SetUp(void)
{
}

void FFRTTimerTest::TearDown(void)
{
}

/**
* @tc.name: SetTimerTest001
* @tc.desc: Set Timer
* @tc.type: FUNC
*/
HWTEST_F(FFRTTimerTest, SetTimerTest001, TestSize.Level0)
{
    FFRTTimer ffrtTimer;
    std::string timerId = "ffrt_test";
    int x = 4;
    FFRTTask ffrtTask = [&x] {
        x /= 2;
    };
    ffrtTimer.SetTimer(timerId, ffrtTask, 0);
    ffrt::wait();
    EXPECT_TRUE(x == 2);
}

/**
* @tc.name: SetTimerTest002
* @tc.desc: Set Timer
* @tc.type: FUNC
*/
HWTEST_F(FFRTTimerTest, SetTimerTest002, TestSize.Level0)
{
    FFRTTimer pbFfrtTimer("paste_ffrt_timer");
    std::string pbTimerId = "paste_ffrt_test1";
    int y = 8;
    FFRTTask pbFfrtTask = [&y] {
        y /= 2;
    };
    pbFfrtTimer.SetTimer(pbTimerId, pbFfrtTask, 5);
    ffrt::wait();
    uint32_t taskId = pbFfrtTimer.GetTaskId(pbTimerId);
    EXPECT_TRUE(taskId == 1);
}

/**
* @tc.name: SubmitQueueTasksTest
* @tc.desc: Submit Queue Tasks
* @tc.type: FUNC
*/
HWTEST_F(FFRTTimerTest, SubmitQueueTasksTest, TestSize.Level0)
{
    std::vector<FFRTTask> tasks;
    FFRTQueue que("QueueTasks");
    std::vector<FFRTTask>().swap(tasks);
    FFRTUtils::SubmitQueueTasks(tasks, que);
    int x = 2;
    int z = 19;
    FFRTTask task1 = [&x] {
        x <<= 3;
    };
    FFRTTask task2 = [&z] {
        z /= 2;
    };
    tasks.push_back(task1);
    tasks.push_back(task2);
    FFRTUtils::SubmitQueueTasks(tasks, que);
    EXPECT_TRUE(true);
}

/**
* @tc.name: SubmitDelayTaskTest
* @tc.desc: Submit Delay Task
* @tc.type: FUNC
*/
HWTEST_F(FFRTTimerTest, SubmitDelayTaskTest, TestSize.Level0)
{
    std::shared_ptr<FFRTQueue> queu = std::make_shared<FFRTQueue>("delayTask");
    uint32_t delayMs = 20;
    int x = 10;
    FFRTTask task0 = [&x] {
        x <<= 3;
    };
    FFRTHandle handle = FFRTUtils::SubmitDelayTask(task0, delayMs, queu);
    FFRTUtils::CancelTask(handle, queu);
    EXPECT_TRUE(true);
}

/**
* @tc.name: SubmitTimeoutTaskTest
* @tc.desc: Submit Timeout Task
* @tc.type: FUNC
*/
HWTEST_F(FFRTTimerTest, SubmitTimeoutTaskTest, TestSize.Level0)
{
    uint32_t timeoutMs = 5;
    int x = 9;
    FFRTTask task = [&x] {
        x <<= 3;
    };
    bool res = FFRTUtils::SubmitTimeoutTask(task, timeoutMs);
    EXPECT_TRUE(res);
}
} // namespace OHOS::MiscServices