/*
* Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <thread>
#include <gtest/gtest.h>
#include "iservice_registry.h"
#include "pasteboard_dialog.h"
#include "pasteboard_error.h"

using namespace OHOS;

sptr<ISystemAbilityManager> OHOS::SystemAbilityManagerClient::GetSystemAbilityManager()
{
    return nullptr;
}

namespace OHOS::MiscServices {
using namespace testing::ext;
class PasteboardDialogAbnormalBranchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardDialogAbnormalBranchTest::SetUpTestCase(void)
{
}

void PasteboardDialogAbnormalBranchTest::TearDownTestCase(void)
{
}

void PasteboardDialogAbnormalBranchTest::SetUp(void)
{
}

void PasteboardDialogAbnormalBranchTest::TearDown(void)
{
}

/**
* @tc.name: ShowToastAbnormalTest
* @tc.desc: Show Toast test abnormal branch.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardDialogAbnormalBranchTest, ShowToastAbnormalTest, TestSize.Level0)
{
    PasteBoardDialog::ToastMessageInfo message;
    message.appName = "myAppName";
    int32_t ret = PasteBoardDialog::GetInstance().ShowToast(message);
    EXPECT_TRUE(ret == static_cast<int32_t>(PasteboardError::OBTAIN_SERVER_SA_ERROR));
}

/**
* @tc.name: CancelToastAbnormalTest
* @tc.desc: Cancel Toast test abnormal branch.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardDialogAbnormalBranchTest, CancelToastAbnormalTest, TestSize.Level0)
{
    PasteBoardDialog::GetInstance().CancelToast();
    EXPECT_TRUE(true);
}
}