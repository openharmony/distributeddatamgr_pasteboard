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
    std::string bundleName = "com.dev.profile";
    DevProfile::GetInstance().GetRemoteDeviceVersion(bundleName, versionId);
    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: GetEnabledStatusTest001
 * @tc.desc: GetEnabledStatus should not return E_OK when query invalid networkId
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileTest, GetEnabledStatusTest001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    std::string enabledStatus = "";
    std::string networkId = "test.dev.profile";
    int32_t ret = DevProfile::GetInstance().GetEnabledStatus(networkId, enabledStatus);
    EXPECT_NE(ret, static_cast<int32_t>(PasteboardError::E_OK));
#else
    EXPECT_EQ(ret, static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR));
#endif
}

/**
 * @tc.name: GetEnabledStatusTest002
 * @tc.desc: GetEnabledStatus should not return NO_TRUST_DEVICE_ERROR when query valid networkId
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileTest, GetEnabledStatusTest002, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    std::string enabledStatus = "";
    auto networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    int32_t ret = DevProfile::GetInstance().GetEnabledStatus(networkId, enabledStatus);
    EXPECT_NE(ret, static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR));
#else
    EXPECT_EQ(ret, static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR));
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
    std::string bundleName = "com.pro.proEvent";
    DevProfile::GetInstance().SubscribeProfileEvent(bundleName);
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