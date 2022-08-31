/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
using namespace testing::ext;
using namespace OHOS::MiscServices;

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
* @tc.name: GetLocalDeviceId
* @tc.desc: Get the local device id.
* @tc.type: FUNC
*/
HWTEST_F(DMAdapterTest, GetLocalDeviceId, TestSize.Level0)
{
    auto &deviceId = DMAdapter::GetInstance().GetLocalDevice();
    ASSERT_FALSE(deviceId.empty());
}
