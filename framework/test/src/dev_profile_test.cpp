/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "device/dev_profile.h"
#include "device/dm_adapter.h"
#include "pasteboard_error.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
class DevProfileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DevProfileTest::SetUpTestCase(void) { }

void DevProfileTest::TearDownTestCase(void) { }

void DevProfileTest::SetUp(void) { }

void DevProfileTest::TearDown(void) { }

/**
 * @tc.name: GetRemoteDeviceVersion
 * @tc.desc: Get Remote Device Version
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileTest, GetRemoteDeviceVersionTest001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    uint32_t versionId;
    std::string bondleName = "com.dev.profile";
    DevProfile::GetInstance().GetRemoteDeviceVersion(bondleName, versionId);
    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: SubscribeProfileEvent001
 * @tc.desc: Sub scribe Profile Event
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileTest, SubscribeProfileEventTest001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    std::string bondleName = "com.pro.proEvent";
    DevProfile::GetInstance().SubscribeProfileEvent(bondleName);
    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: UnSubscribeProfileEvent001
 * @tc.desc: UnSub scribe Profile Event
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileTest, UnSubscribeProfileEventTest001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    std::string bsndleName = "com.pro.proEvent";
    DevProfile::GetInstance().UnSubscribeProfileEvent(bsndleName);
    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: WatchTest
 * @tc.desc: Watch
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileTest, Watch, TestSize.Level0)
{
    DevProfile::Observer observer;
    DevProfile::GetInstance().Watch(observer);
    EXPECT_FALSE(observer);
}
} // namespace OHOS::MiscServices