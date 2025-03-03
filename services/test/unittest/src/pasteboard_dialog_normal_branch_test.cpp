/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include <thread>

#include "iservice_registry.h"
#include "pasteboard_dialog.h"
#include "pasteboard_error.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
class PasteboardDialogNormalBranchTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardDialogNormalBranchTest::SetUpTestCase(void) { }

void PasteboardDialogNormalBranchTest::TearDownTestCase(void) { }

void PasteboardDialogNormalBranchTest::SetUp(void) { }

void PasteboardDialogNormalBranchTest::TearDown(void) { }

/**
 * @tc.name: ShowToastNormalTest
 * @tc.desc: Show Toast test normal branch.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDialogNormalBranchTest, ShowToastNormalTest, TestSize.Level0)
{
    PasteBoardDialog::ToastMessageInfo message;
    message.appName = "myAppName";
    int32_t ret = PasteBoardDialog::GetInstance().ShowToast(message);
    EXPECT_FALSE(ret == static_cast<int32_t>(PasteboardError::TASK_PROCESSING));
}

/**
 * @tc.name: CancelToastNormalTest
 * @tc.desc: Cancel Toast test.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDialogNormalBranchTest, CancelToastNormalTest, TestSize.Level0)
{
    PasteBoardDialog::GetInstance().CancelToast();
    EXPECT_TRUE(true);
}
} // namespace OHOS::MiscServices