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

#include "distributed_module_config_mock_test.h"

#include "device/dev_profile.h"
#include "device/dm_adapter.h"
#include "device/distributed_module_config.h"
#include "pasteboard_error.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace MiscServices {

void DistributedModuleConfigMockTest::SetUpTestCase(void)
{
    DistributedDeviceProfile::PasteDistributedDeviceProfileClient::pasteDistributedDeviceProfileClient =
        distributedDeviceProfileClientMock_;
}

void DistributedModuleConfigMockTest::TearDownTestCase(void)
{
    DistributedDeviceProfile::PasteDistributedDeviceProfileClient::pasteDistributedDeviceProfileClient = nullptr;
    distributedDeviceProfileClientMock_ = nullptr;
}

void DistributedModuleConfigMockTest::SetUp(void) { }

void DistributedModuleConfigMockTest::TearDown(void)
{
    testing::Mock::VerifyAndClear(distributedDeviceProfileClientMock_.get());
}

/**
 * @tc.name: GetEnabledStatusTest001
 * @tc.desc: GetEnabledStatus should return LOCAL_SWITCH_NOT_TURNED_ON when local switch off.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DistributedModuleConfigMockTest, GetEnabledStatusTest001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR)));
    DMAdapter::GetInstance().devices_.clear();
    DevProfile::GetInstance().enabledStatusCache_.Clear();
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    DistributedModuleConfig config;
    int32_t ret = config.GetEnabledStatus();
    ASSERT_EQ(static_cast<int32_t>(PasteboardError::LOCAL_SWITCH_NOT_TURNED_ON), ret);
#else
    EXPECT_EQ(static_cast<int32_t>(PasteboardError::LOCAL_SWITCH_NOT_TURNED_ON), ret);
#endif
}

/**
 * @tc.name: GetEnabledStatusTest002
 * @tc.desc: GetEnabledStatus should return NO_TRUST_DEVICE_ERROR when remote device is invalid.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DistributedModuleConfigMockTest, GetEnabledStatusTest002, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillOnce([](auto, auto, auto, DistributedDeviceProfile::CharacteristicProfile &characteristicProfile) {
            characteristicProfile.characteristicValue_ = "1";
            return DistributedDeviceProfile::DP_SUCCESS;
        })
        .WillRepeatedly([](auto, auto, auto, DistributedDeviceProfile::CharacteristicProfile &characteristicProfile) {
            characteristicProfile.characteristicValue_ = "0";
            return DistributedDeviceProfile::DP_SUCCESS;
        });
    DMAdapter::GetInstance().devices_.clear();
    DevProfile::GetInstance().enabledStatusCache_.Clear();
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string networkId = "testNetworkId";
    std::string testName = "testDeviceName";
    DmDeviceInfo info;
    info.authForm = IDENTICAL_ACCOUNT;
    std::copy(networkId.begin(), networkId.end(), info.networkId);
    std::copy(testName.begin(), testName.end(), info.deviceName);
    DMAdapter::GetInstance().devices_.emplace_back(info);
    DistributedModuleConfig config;
    int32_t ret = config.GetEnabledStatus();
    ASSERT_EQ(static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR), ret);
    DMAdapter::GetInstance().devices_.clear();
#else
    EXPECT_EQ(static_cast<int32_t>(PasteboardError::LOCAL_SWITCH_NOT_TURNED_ON), ret);
#endif
}

/**
 * @tc.name: GetEnabledStatusTest003
 * @tc.desc: GetEnabledStatus should return E_OK when query valid network.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DistributedModuleConfigMockTest, GetEnabledStatusTest003, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly([](auto, auto, auto, DistributedDeviceProfile::CharacteristicProfile &characteristicProfile) {
            characteristicProfile.characteristicValue_ = "1";
            return DistributedDeviceProfile::DP_SUCCESS;
        });
    DMAdapter::GetInstance().devices_.clear();
    DevProfile::GetInstance().enabledStatusCache_.Clear();
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    std::string testName = "testDeviceName";
    DmDeviceInfo info;
    info.authForm = IDENTICAL_ACCOUNT;
    std::copy(networkId.begin(), networkId.end(), info.networkId);
    std::copy(testName.begin(), testName.end(), info.deviceName);
    DMAdapter::GetInstance().devices_.emplace_back(info);
    DistributedModuleConfig config;
    int32_t ret = config.GetEnabledStatus();
    ASSERT_EQ(static_cast<int32_t>(PasteboardError::E_OK), ret);
    DMAdapter::GetInstance().devices_.clear();
#else
    EXPECT_EQ(static_cast<int32_t>(PasteboardError::LOCAL_SWITCH_NOT_TURNED_ON), ret);
#endif
}

/**
 * @tc.name: Notify001
 * @tc.desc: Notify.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DistributedModuleConfigMockTest, Notify001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly([](auto, auto, auto, DistributedDeviceProfile::CharacteristicProfile &characteristicProfile) {
            characteristicProfile.characteristicValue_ = "1";
            return DistributedDeviceProfile::DP_SUCCESS;
        });
    DMAdapter::GetInstance().devices_.clear();
    DevProfile::GetInstance().enabledStatusCache_.Clear();
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    std::string testName = "testDeviceName";
    DmDeviceInfo info;
    info.authForm = IDENTICAL_ACCOUNT;
    std::copy(networkId.begin(), networkId.end(), info.networkId);
    std::copy(testName.begin(), testName.end(), info.deviceName);
    DMAdapter::GetInstance().devices_.emplace_back(info);
    DistributedModuleConfig config;
    config.status_ = false;
    config.Notify();
    ASSERT_TRUE(true);
    DMAdapter::GetInstance().devices_.clear();
#else
    ASSERT_TRUE(true);
#endif
}

/**
 * @tc.name: Notify002
 * @tc.desc: Notify.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DistributedModuleConfigMockTest, Notify002, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly([](auto, auto, auto, DistributedDeviceProfile::CharacteristicProfile &characteristicProfile) {
            characteristicProfile.characteristicValue_ = "1";
            return DistributedDeviceProfile::DP_SUCCESS;
        });
    DMAdapter::GetInstance().devices_.clear();
    DevProfile::GetInstance().enabledStatusCache_.Clear();
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    std::string testName = "testDeviceName";
    DmDeviceInfo info;
    info.authForm = IDENTICAL_ACCOUNT;
    std::copy(networkId.begin(), networkId.end(), info.networkId);
    std::copy(testName.begin(), testName.end(), info.deviceName);
    DMAdapter::GetInstance().devices_.emplace_back(info);
    DistributedModuleConfig config;
    config.status_ = false;
    std::function<void(bool isOn)> func = [](bool isOn) {
        return;
    };
    config.observer_ = func;
    config.Notify();
    ASSERT_TRUE(true);
    DMAdapter::GetInstance().devices_.clear();
#else
    ASSERT_TRUE(true);
#endif
}

/**
 * @tc.name: GetRemoteDeviceMinVersion001
 * @tc.desc: GetRemoteDeviceMinVersion.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DistributedModuleConfigMockTest, GetRemoteDeviceMinVersion001, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly([](auto, auto, auto, DistributedDeviceProfile::CharacteristicProfile &characteristicProfile) {
            characteristicProfile.characteristicValue_ = "0";
            return static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR);
        });
    DMAdapter::GetInstance().devices_.clear();
    DevProfile::GetInstance().enabledStatusCache_.Clear();
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    std::string testName = "testDeviceName";
    DmDeviceInfo info;
    info.authForm = IDENTICAL_ACCOUNT;
    std::copy(networkId.begin(), networkId.end(), info.networkId);
    std::copy(testName.begin(), testName.end(), info.deviceName);
    DMAdapter::GetInstance().devices_.emplace_back(info);
    DistributedModuleConfig config;
    uint32_t minVersion = config.GetRemoteDeviceMinVersion();
    ASSERT_EQ(UINT_MAX, minVersion);
    DMAdapter::GetInstance().devices_.clear();
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: GetRemoteDeviceMinVersion002
 * @tc.desc: GetRemoteDeviceMinVersion.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DistributedModuleConfigMockTest, GetRemoteDeviceMinVersion002, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly([](auto, auto, auto, DistributedDeviceProfile::CharacteristicProfile &characteristicProfile) {
            characteristicProfile.characteristicValue_ = "0";
            return static_cast<int32_t>(DistributedDeviceProfile::DP_SUCCESS);
        });
    DMAdapter::GetInstance().devices_.clear();
    DevProfile::GetInstance().enabledStatusCache_.Clear();
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    std::string testName = "testDeviceName";
    DmDeviceInfo info;
    info.authForm = IDENTICAL_ACCOUNT;
    std::copy(networkId.begin(), networkId.end(), info.networkId);
    std::copy(testName.begin(), testName.end(), info.deviceName);
    DMAdapter::GetInstance().devices_.emplace_back(info);
    DistributedModuleConfig config;
    uint32_t minVersion = config.GetRemoteDeviceMinVersion();
    ASSERT_EQ(UINT_MAX, minVersion);
    DMAdapter::GetInstance().devices_.clear();
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: GetRemoteDeviceMinVersion003
 * @tc.desc: GetRemoteDeviceMinVersion.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DistributedModuleConfigMockTest, GetRemoteDeviceMinVersion003, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly([](auto, auto, auto, DistributedDeviceProfile::CharacteristicProfile &characteristicProfile) {
            characteristicProfile.characteristicValue_ = "1";
            return static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR);
        });
    DMAdapter::GetInstance().devices_.clear();
    DevProfile::GetInstance().enabledStatusCache_.Clear();
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    std::string testName = "testDeviceName";
    DmDeviceInfo info;
    info.authForm = IDENTICAL_ACCOUNT;
    std::copy(networkId.begin(), networkId.end(), info.networkId);
    std::copy(testName.begin(), testName.end(), info.deviceName);
    DMAdapter::GetInstance().devices_.emplace_back(info);
    DistributedModuleConfig config;
    uint32_t minVersion = config.GetRemoteDeviceMinVersion();
    ASSERT_EQ(UINT_MAX, minVersion);
    DMAdapter::GetInstance().devices_.clear();
#else
    EXPECT_TRUE(true);
#endif
}

/**
 * @tc.name: GetRemoteDeviceMinVersion004
 * @tc.desc: GetRemoteDeviceMinVersion.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(DistributedModuleConfigMockTest, GetRemoteDeviceMinVersion004, TestSize.Level0)
{
#ifdef PB_DEVICE_INFO_MANAGER_ENABLE
    EXPECT_CALL(
        *distributedDeviceProfileClientMock_, GetCharacteristicProfile(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly([](auto, auto, auto, DistributedDeviceProfile::CharacteristicProfile &characteristicProfile) {
            characteristicProfile.characteristicValue_ = "1";
            return static_cast<int32_t>(DistributedDeviceProfile::DP_SUCCESS);
        });
    DMAdapter::GetInstance().devices_.clear();
    DevProfile::GetInstance().enabledStatusCache_.Clear();
    std::string bundleName = "com.example.myApplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    std::string testName = "testDeviceName";
    DmDeviceInfo info;
    info.authForm = IDENTICAL_ACCOUNT;
    std::copy(networkId.begin(), networkId.end(), info.networkId);
    std::copy(testName.begin(), testName.end(), info.deviceName);
    DMAdapter::GetInstance().devices_.emplace_back(info);
    DistributedModuleConfig config;
    uint32_t minVersion = config.GetRemoteDeviceMinVersion();
    ASSERT_EQ(UINT_MAX, minVersion);
    DMAdapter::GetInstance().devices_.clear();
#else
    EXPECT_TRUE(true);
#endif
}

} // namespace MiscServices
} // namespace OHOS