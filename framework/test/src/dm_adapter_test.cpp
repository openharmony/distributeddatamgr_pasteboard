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

#include "device/dm_adapter.h"
#include "device_manager.h"
#include "distributed_clip.h"
#include "pasteboard_error.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
class DMAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DMAdapterTest::SetUpTestCase(void) {}

void DMAdapterTest::TearDownTestCase(void) {}

void DMAdapterTest::SetUp(void) {}

void DMAdapterTest::TearDown(void) {}

/**
* @tc.name: GetLocalDeviceUdid001
* @tc.desc: Get the local device udid.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetLocalDeviceUdid001, TestSize.Level0)
{
    std::string bundleName = "com.example.myapplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    ASSERT_FALSE(res);
    std::string device = "deviceTestName";
    auto fromDevice = DMAdapter::GetInstance().GetDeviceName(device);
    ASSERT_FALSE(fromDevice.empty());
    auto &udid = DMAdapter::GetInstance().GetLocalDeviceUdid();
    ASSERT_FALSE(udid.empty());
}

/**
* @tc.name: GetLocalDeviceUdid002
* @tc.desc: Get the local device udid.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetLocalDeviceUdid002, TestSize.Level0)
{
    std::string bundleName = "com.example.myapplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    std::string device = "deviceTestName";
    auto fromDevice = DMAdapter::GetInstance().GetDeviceName(device);
    DmDeviceInfo info;
    std::string pkgName_ = "pkgName";
    std::string localDeviceUdid_ = "localDeviceUdid";
    DeviceManager::GetInstance().GetUdidByNetworkId(pkgName_, info.networkId, localDeviceUdid_);
    auto &udid = DMAdapter::GetInstance().GetLocalDeviceUdid();
    ASSERT_FALSE(udid.empty());
}

/**
* @tc.name: GetLocalNetworkId
* @tc.desc: Get the local network id.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetLocalNetworkId, TestSize.Level0)
{
    std::string bundleName = "com.example.myapplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    ASSERT_FALSE(res);
    auto networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    ASSERT_FALSE(networkId.empty());
}

/**
* @tc.name: DistributedClipRegister
* @tc.desc: DistributedClip Register and Unregister.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, DistributedClipRegister, TestSize.Level0)
{
    DistributedClip *observer = new DistributedClip();
    DMAdapter::GetInstance().Register(observer);
    DMAdapter::GetInstance().Unregister(observer);
    ASSERT_TRUE(true);
}

/**
* @tc.name: GetRemoteDeviceInfo
* @tc.desc: Get the remote device info.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetRemoteDeviceInfo, TestSize.Level0)
{
#ifdef PB_DEVICE_MANAGER_ENABLE
    DmDeviceInfo remoteDevice;
    auto ret = DMAdapter::GetInstance().GetRemoteDeviceInfo("", remoteDevice);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::NO_TRUST_DEVICE_ERROR));
#else
    ASSERT_TRUE(true);
#endif
}

/**
* @tc.name: GetUdidByNetworkId
* @tc.desc: Get Udid By NetworkId.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetUdidByNetworkId, TestSize.Level0)
{
    auto udid = DMAdapter::GetInstance().GetUdidByNetworkId("");
    ASSERT_TRUE(udid.empty());
}

/**
* @tc.name: IsSameAccount
* @tc.desc: is same account.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, IsSameAccount, TestSize.Level0)
{
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    bool ret = DMAdapter::GetInstance().IsSameAccount(networkId);
    ASSERT_FALSE(ret);
}

/**
* @tc.name: GetDevices
* @tc.desc: Get Devices.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetDevices, TestSize.Level0)
{
    DMAdapter::GetInstance().SetDevices();
    std::vector<DmDeviceInfo> devices = DMAdapter::GetInstance().GetDevices();
    ASSERT_TRUE(devices.empty());
}

/**
* @tc.name: GetLocalDeviceType
* @tc.desc: Get Local Device Type
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetLocalDeviceType, TestSize.Level0)
{
    int32_t type = 14;
    int32_t ret = DMAdapter::GetInstance().GetLocalDeviceType();
    ASSERT_EQ(ret, type);
}

/**
* @tc.name: GetDeviceName001
* @tc.desc: Get Local Device Type
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetDeviceName001, TestSize.Level0)
{
    std::string networkId = "invalidnetworkId";
    std::string expectedDeviceName = "unknown";
    std::string actualDeviceName = DMAdapter::GetInstance().GetDeviceName(networkId);
    EXPECT_EQ(expectedDeviceName, actualDeviceName);
}

/**
* @tc.name: GetDeviceName002
* @tc.desc: Get Local Device Type
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetDeviceName002, TestSize.Level0)
{
    std::string networkId = DMAdapter::GetInstance().GetLocalNetworkId();
    std::string expectedDeviceName = "unknown";
    std::string actualDeviceName = DMAdapter::GetInstance().GetDeviceName(networkId);
    EXPECT_EQ(expectedDeviceName, actualDeviceName);
}
} // namespace OHOS::MiscServices
