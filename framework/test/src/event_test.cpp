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

#include <gtest/gtest.h>

#include "eventcenter/event.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
class EventTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void EventTest::SetUpTestCase(void) {}

void EventTest::TearDownTestCase(void) {}

void EventTest::SetUp(void) {}

void EventTest::TearDown(void) {}

/**
 * @tc.name: GetEventIdTest
 * @tc.desc: Test GetEventId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EventTest, GetEventIdTest, TestSize.Level0)
{
    int32_t TestEventId = 123;
    Event event(TestEventId);

    int32_t result = event.GetEventId();

    EXPECT_EQ(result, TestEventId);
}

/**
 * @tc.name: EqualsTest
 * @tc.desc: Test Equals
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(EventTest, EqualsTest, TestSize.Level0)
{
    int32_t TestEventId = 123;
    Event event1(TestEventId);
    Event event2(TestEventId);

    bool result = event1.Equals(event2);

    EXPECT_EQ(result, false);
}
} // namespace OHOS::MiscServices
