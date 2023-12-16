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

#include "device/dm_adapter.h"
#include "distributed_clip.h"
#include <gtest/gtest.h>

namespace OHOS::MiscServices {
using namespace testing::ext;
class DMAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DMAdapterTest::SetUpTestCase(void)
{
}

void DMAdapterTest::TearDownTestCase(void)
{
}

void DMAdapterTest::SetUp(void)
{
}

void DMAdapterTest::TearDown(void)
{
}

/**
* @tc.name: GetLocalDeviceUdid
* @tc.desc: Get the local device udid.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(DMAdapterTest, GetLocalDeviceUdid, TestSize.Level0)
{
    std::string bundleName = "com.example.myapplication";
    bool res = DMAdapter::GetInstance().Initialize(bundleName);
    ASSERT_FALSE(res);
    std::string device = "deviceTestName";
    auto fromDevice = DMAdapter::GetInstance().GetDeviceName(device);
    ASSERT_FALSE(fromDevice.empty());
    auto &udid = DMAdapter::GetInstance().GetLocalDeviceUdid();
    ASSERT_TRUE(udid.empty());
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
    ASSERT_TRUE(ret == -1);
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
}
