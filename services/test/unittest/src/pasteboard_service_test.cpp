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

#include <gtest/gtest.h>

#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_service.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
class PasteboardServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardServiceTest::SetUpTestCase(void) { }

void PasteboardServiceTest::TearDownTestCase(void) { }

void PasteboardServiceTest::SetUp(void) { }

void PasteboardServiceTest::TearDown(void) { }

/**
 * @tc.name: IncreaseChangeCountTest001
 * @tc.desc: IncreaseChangeCount should reset to 0 after reach maximum limit.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IncreaseChangeCountTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t userId = 10;
    tempPasteboard->clipChangeCount_.Compute(userId, [](auto, uint32_t &changeCount) {
        changeCount = UINT32_MAX;
        return true;
    });
    uint32_t testCount = 0;
    auto it = tempPasteboard->clipChangeCount_.Find(userId);
    if (it.first) {
        testCount = it.second;
    }
    ASSERT_EQ(testCount, UINT32_MAX);
    tempPasteboard->IncreaseChangeCount(userId);
    it = tempPasteboard->clipChangeCount_.Find(userId);
    if (it.first) {
        testCount = it.second;
    }
    ASSERT_EQ(testCount, 0);
}

/**
 * @tc.name: IncreaseChangeCountTest002
 * @tc.desc: IncreaseChangeCount should reset to 0 after switch to a new user.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IncreaseChangeCountTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    uint32_t testCount = 0;
    tempPasteboard->GetChangeCount(testCount);
    ASSERT_EQ(testCount, 0);
    tempPasteboard->currentUserId = 10;
    auto userId = tempPasteboard->GetCurrentAccountId();
    tempPasteboard->IncreaseChangeCount(userId);
    tempPasteboard->GetChangeCount(testCount);
    ASSERT_EQ(testCount, 1);
    tempPasteboard->currentUserId = 100;
    tempPasteboard->GetChangeCount(testCount);
    ASSERT_EQ(testCount, 0);
}
} // namespace OHOS::MiscServices