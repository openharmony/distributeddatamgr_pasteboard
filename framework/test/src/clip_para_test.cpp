/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
#include "device/clip_para.h"

#include <chrono>
#include <gtest/gtest.h>

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace std::chrono;
using namespace OHOS::MiscServices;
class ClipParaTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ClipParaTest::SetUpTestCase(void)
{
}

void ClipParaTest::TearDownTestCase(void)
{
}

void ClipParaTest::SetUp(void)
{
}

void ClipParaTest::TearDown(void)
{
    ClipPara::GetInstance().InitMemberVariable();
}

/**
* @tc.name: InitMemberVariable_001
* @tc.desc: Init Member Variable
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, InitMemberVariable_001, TestSize.Level0)
{
    ClipPara::GetInstance().InitMemberVariable();
    auto sendInformation = ClipPara::GetInstance().GetSendInformation();
    ASSERT_TRUE(sendInformation == nullptr);
    auto remoteExpiration = ClipPara::GetInstance().GetRemoteExpiration();
    ASSERT_TRUE(remoteExpiration == 0);
    auto lastSyncNetworkId = ClipPara::GetInstance().GetLastSyncNetworkId();
    ASSERT_TRUE(lastSyncNetworkId.empty());
    auto isPullEvent = ClipPara::GetInstance().GetPullEvent();
    ASSERT_FALSE(isPullEvent);
    auto isPullEventResult = ClipPara::GetInstance().GetPullEventResult();
    ASSERT_FALSE(isPullEventResult);
    auto isPasted = ClipPara::GetInstance().GetPasted();
    ASSERT_FALSE(isPasted);
}

/**
* @tc.name: LastLocalSyncKey_001
* @tc.desc: test LastLocalSyncKey
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, LastLocalSyncKey_001, TestSize.Level0)
{
    ClipPara::GetInstance().InitMemberVariable();
    std::string lastLocalSyncKey = "event_100";
    ClipPara::GetInstance().SetLastLocalSyncKey(lastLocalSyncKey);
    auto ret = ClipPara::GetInstance().GetLastLocalSyncKey();
    ASSERT_TRUE(ret == lastLocalSyncKey);
}

/**
* @tc.name: LastRemoteSyncKey_001
* @tc.desc: test LastRemoteSyncKey
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, LastRemoteSyncKey_001, TestSize.Level0)
{
    ClipPara::GetInstance().InitMemberVariable();
    std::string lastRemoteSyncKey = "event_100";
    ClipPara::GetInstance().SetLastRemoteSyncKey(lastRemoteSyncKey);
    auto ret = ClipPara::GetInstance().GetLastRemoteSyncKey();
    ASSERT_TRUE(ret == lastRemoteSyncKey);
}

/**
* @tc.name: LocalExpiration_001
* @tc.desc: test LocalExpiration
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, LocalExpiration_001, TestSize.Level0)
{
    ClipPara::GetInstance().InitMemberVariable();
    uint64_t localExpiration = 10000;
    ClipPara::GetInstance().SetLocalExpiration(localExpiration);
    auto ret = ClipPara::GetInstance().GetLocalExpiration();
    ASSERT_TRUE(ret == localExpiration);
}

/**
* @tc.name: RemoteExpiration_001
* @tc.desc: test RemoteExpiration
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, RemoteExpiration_001, TestSize.Level0)
{
    ClipPara::GetInstance().InitMemberVariable();
    uint64_t remoteExpiration = 10000;
    ClipPara::GetInstance().SetRemoteExpiration(remoteExpiration);
    auto ret = ClipPara::GetInstance().GetRemoteExpiration();
    ASSERT_TRUE(ret == remoteExpiration);
}

/**
* @tc.name: SendInformation_001
* @tc.desc: Init Member Variable
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, SendInformation_001, TestSize.Level0)
{
    ClipPara::GetInstance().InitMemberVariable();
    ClipPara::SendInformation sendInformation;
    ClipPara::GetInstance().SetSendInformation(sendInformation);
    auto ret = ClipPara::GetInstance().GetSendInformation();
    ASSERT_TRUE(ret != nullptr);
}

/**
* @tc.name: PullEventResult_001
* @tc.desc: test PullEventResult
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, PullEventResult_001, TestSize.Level0)
{
    ClipPara::GetInstance().InitMemberVariable();
    ClipPara::GetInstance().SetPullEventResult(true);
    auto ret = ClipPara::GetInstance().GetPullEventResult();
    ASSERT_TRUE(ret);
}

/**
* @tc.name: HasRemoteData
* @tc.desc: Check Has Remote Data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, HasRemoteData_001, TestSize.Level0)
{
    ClipPara::GetInstance().InitMemberVariable();
    auto has = ClipPara::GetInstance().HasRemoteData();
    ASSERT_FALSE(has);
}

/**
* @tc.name: UpdateStageValue_001
* @tc.desc: Update Stage Value.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, UpdateStageValue_001, TestSize.Level0)
{
    uint64_t curTime = duration_cast<milliseconds>((system_clock::now() + minutes(2)).time_since_epoch()).count();
    ClipPara::GetInstance().InitMemberVariable();
    ClipPara::GetInstance().SetFirstStageValue(2);
    ClipPara::GetInstance().SetSecondStageValue(0);
    ClipPara::GetInstance().UpdateStageValue(curTime, false);
    auto firstStageValue1 = ClipPara::GetInstance().GetFirstStageValue();
    ASSERT_TRUE(firstStageValue1 == 2);
    auto secondStageValue1 = ClipPara::GetInstance().GetSecondStageValue();
    ASSERT_TRUE(secondStageValue1 == 0);
    ClipPara::GetInstance().UpdateStageValue(curTime, true);
    auto firstStageValue = ClipPara::GetInstance().GetFirstStageValue();
    ASSERT_TRUE(firstStageValue == 2);
    auto secondStageValue = ClipPara::GetInstance().GetSecondStageValue();
    ASSERT_TRUE(secondStageValue == 0);
    auto ret = ClipPara::GetInstance().GetPasted();
    ASSERT_TRUE(ret);
}

/**
* @tc.name: UpdateStageValue_002
* @tc.desc: Update Stage Value.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, UpdateStageValue_002, TestSize.Level0)
{
    uint64_t curTime = duration_cast<milliseconds>((system_clock::now() + minutes(2)).time_since_epoch()).count();
    ClipPara::GetInstance().InitMemberVariable();
    ClipPara::GetInstance().SetPasted(true);
    ClipPara::GetInstance().SetFirstStageValue(0);
    ClipPara::GetInstance().SetSecondStageValue(2);
    ClipPara::GetInstance().UpdateStageValue(curTime, true);
    auto firstStageValue = ClipPara::GetInstance().GetFirstStageValue();
    ASSERT_TRUE(firstStageValue == 0);
    auto secondStageValue = ClipPara::GetInstance().GetSecondStageValue();
    ASSERT_TRUE(secondStageValue == 2);
}

/**
* @tc.name: UpdateStageValue_003
* @tc.desc: Update Stage Value.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, UpdateStageValue_003, TestSize.Level0)
{
    uint64_t curTime = duration_cast<milliseconds>((system_clock::now() + minutes(2)).time_since_epoch()).count();
    ClipPara::GetInstance().InitMemberVariable();
    ClipPara::GetInstance().SetFirstStageValue(0);
    ClipPara::GetInstance().SetSecondStageValue(2);
    ClipPara::GetInstance().UpdateStageValue(curTime, true);
    auto firstStageValue = ClipPara::GetInstance().GetFirstStageValue();
    ASSERT_TRUE(firstStageValue == 2);
    auto secondStageValue = ClipPara::GetInstance().GetSecondStageValue();
    ASSERT_TRUE(secondStageValue == 0);
}

/**
* @tc.name: UpdateStageValue_004
* @tc.desc: Update Stage Value.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(ClipParaTest, UpdateStageValue_004, TestSize.Level0)
{
    uint64_t curTime = duration_cast<milliseconds>((system_clock::now()).time_since_epoch()).count();
    ClipPara::GetInstance().InitMemberVariable();
    ClipPara::GetInstance().SetPullEvent(true);
    ClipPara::GetInstance().SetPasted(false);
    ClipPara::GetInstance().SetFirstStageValue(1);
    ClipPara::GetInstance().SetSecondStageValue(0);
    ClipPara::GetInstance().UpdateStageValue(curTime, true);
    auto firstStageValue = ClipPara::GetInstance().GetFirstStageValue();
    ASSERT_TRUE(firstStageValue == 0);
    auto secondStageValue = ClipPara::GetInstance().GetSecondStageValue();
    ASSERT_TRUE(secondStageValue == 0);
}
}