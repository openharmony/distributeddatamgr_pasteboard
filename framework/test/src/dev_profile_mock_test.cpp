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

#include "dev_profile_mock_test.h"

#include "device/dev_profile.h"
#include "device/dm_adapter.h"
#include "pasteboard_error.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace MiscServices {

void DevProfileMockTest::SetUpTestCase(void)
{
    DistributedDeviceProfile::PasteDistributedDeviceProfileClient::pasteDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
}

void DevProfileMockTest::TearDownTestCase(void)
{
    DistributedDeviceProfile::PasteDistributedDeviceProfileClient::pasteDistributedDeviceProfileClient = nullptr;
    distributedDeviceProfileClientMock_ = nullptr;
}

void DevProfileMockTest::SetUp(void) { }

void DevProfileMockTest::TearDown(void) { }

/**
 * @tc.name: GetEnabledStatusTest001
 * @tc.desc: GetEnabledStatus should return E_OK when query valid networkId
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileMockTest, GetEnabledStatusTest001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(DistributedDeviceProfile::DP_SUCCESS));
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string enabledStatus = "";
    auto networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    int32_t ret = DevProfile::GetInstance().GetEnabledStatus(networkId, enabledStatus);
    ASSERT_EQ(static_cast<int32_t>(PasteboardError::E_OK), ret);
    ret = DevProfile::GetInstance().GetEnabledStatus(networkId, enabledStatus);
    ASSERT_EQ(static_cast<int32_t>(PasteboardError::E_OK), ret);
#else
    EXPECT_EQ(ret, static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR));
#endif
}

/**
 * @tc.name: GetEnabledStatusTest002
 * @tc.desc: GetEnabledStatus should return E_OK when query valid networkId
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileMockTest, GetEnabledStatusTest002, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR)));
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string enabledStatus = "";
    auto networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    int32_t ret = DevProfile::GetInstance().GetEnabledStatus(networkId, enabledStatus);
    ASSERT_EQ(static_cast<int32_t>(PasteboardError::DP_LOAD_SERVICE_ERROR), ret);
#else
    EXPECT_EQ(static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR), ret);
#endif
}

/**
 * @tc.name: PutCharacteristicProfile001
 * @tc.desc: PutCharacteristicProfile
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileMockTest, PutCharacteristicProfile001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutCharacteristicProfile(testing::_))
        .WillRepeatedly(testing::Return(DistributedDeviceProfile::DP_SUCCESS));
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string enabledStatus = "";
    DevProfile::GetInstance().PutEnabledStatus(enabledStatus);
    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: PutCharacteristicProfile002
 * @tc.desc: PutCharacteristicProfile
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileMockTest, PutCharacteristicProfile002, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutCharacteristicProfile(testing::_))
        .WillRepeatedly(testing::Return(DistributedDeviceProfile::DP_CACHE_EXIST));
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string enabledStatus = "";
    DevProfile::GetInstance().PutEnabledStatus(enabledStatus);
    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: PutCharacteristicProfile003
 * @tc.desc: PutCharacteristicProfile
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileMockTest, PutCharacteristicProfile003, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(*distributedDeviceProfileClientMock_, PutCharacteristicProfile(testing::_))
        .WillRepeatedly(testing::Return(DistributedDeviceProfile::DP_INVALID_PARAMS));
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string enabledStatus = "";
    DevProfile::GetInstance().PutEnabledStatus(enabledStatus);
    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: GetRemoteDeviceVersionTest001
 * @tc.desc: GetRemoteDeviceVersionTest
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileMockTest, GetRemoteDeviceVersionTest001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(DistributedDeviceProfile::DP_SUCCESS));
    uint32_t versionId;
    std::string bundleName = "com.dev.profile";
    DevProfile::GetInstance().GetRemoteDeviceVersion(bundleName, versionId);
    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: GetRemoteDeviceVersionTest002
 * @tc.desc: GetRemoteDeviceVersionTest
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DevProfileMockTest, GetRemoteDeviceVersionTest002, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(DistributedDeviceProfile::DP_INVALID_PARAMS));
    uint32_t versionId;
    std::string bundleName = "com.dev.profile";
    DevProfile::GetInstance().GetRemoteDeviceVersion(bundleName, versionId);
    EXPECT_TRUE(true);
#else
    EXPECT_TRUE(true);
#endif
}

} // namespace MiscServices
} // namespace OHOS