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

#include <gtest/gtest.h>
#include <thread>

#include "accesstoken_kit.h"
#include "mock_native_token.h"
#include "pasteboard_client.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_window_manager.h"
#include "permission/permission_utils.h"
#include "token_setproc.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace OHOS::Security::AccessToken;

constexpr pid_t SELECTION_SERVICE_UID = 1080;
const std::string BUNDLE_NAME = "com.pasteboard.disposableTest";

class PasteboardDisposableClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static void AllocTestAppTokenId();
    static inline pid_t selfUid_ = 0;
    static inline uint64_t selfTokenId_ = 0;
    static inline uint64_t testAppTokenId_ = 0;
    static inline int32_t windowId_ = 0;
};

void PasteboardDisposableClientTest::SetUpTestCase()
{
    {
        MockNativeToken mock("pasteboard_service");
        windowId_ = WindowManager::GetFocusWindowId();
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "focusWindowId=%{public}d", windowId_);
    }
    selfTokenId_ = GetSelfTokenID();
    AllocTestAppTokenId();
    selfUid_ = getuid();
    setuid(SELECTION_SERVICE_UID);
}

void PasteboardDisposableClientTest::TearDownTestCase()
{
    AccessTokenKit::DeleteToken(testAppTokenId_);
    SetSelfTokenID(selfTokenId_);
    setuid(selfUid_);
}

void PasteboardDisposableClientTest::SetUp()
{
    PasteboardClient::GetInstance()->Clear();
}

void PasteboardDisposableClientTest::TearDown()
{
}

void PasteboardDisposableClientTest::AllocTestAppTokenId()
{
    HapInfoParams infoParams = {
        .userID = 100,
        .bundleName = BUNDLE_NAME,
        .instIndex = 0,
        .appIDDesc = "desc",
    };
    PermissionStateFull testState = {
        .permissionName = PermissionUtils::PERMISSION_READ_PASTEBOARD,
        .isGeneral = true,
        .resDeviceID = { "local" },
        .grantStatus = { PermissionState::PERMISSION_GRANTED },
        .grantFlags = { 1 },
    };
    HapPolicyParams policyParams = {
        .apl = APL_NORMAL,
        .domain = "test.domain.pasteboard",
        .permList = {},
        .permStateList = { testState },
    };

    AccessTokenKit::AllocHapToken(infoParams, policyParams);
    testAppTokenId_ = Security::AccessToken::AccessTokenKit::GetHapTokenID(infoParams.userID, infoParams.bundleName,
        infoParams.instIndex);
    SetSelfTokenID(testAppTokenId_);
    AccessTokenKit::GrantPermission(testAppTokenId_, PermissionUtils::PERMISSION_READ_PASTEBOARD, PERMISSION_USER_SET);
}

class DisposableObserverImpl : public PasteboardDisposableObserver {
public:
    void OnTextReceived(const std::string &text, int32_t errCode) override
    {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "text=%{public}s, errCode=%{public}d", text.c_str(), errCode);
        text_ = text;
        errCode_ = errCode;
    }

    std::string text_;
    int32_t errCode_;
};

/**
 * @tc.name: TestDisposable001
 * @tc.desc: should callback text when set paste data with disposable observer
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDisposableClientTest, TestDisposable001, TestSize.Level0)
{
    DisposableType type = DisposableType::PLAIN_TEXT;
    uint32_t maxLen = 100;
    int32_t windowId = windowId_;
    sptr<DisposableObserverImpl> observer = sptr<DisposableObserverImpl>::MakeSptr();
    int32_t ret = PasteboardClient::GetInstance()->SubscribeDisposableObserver(observer, windowId, type, maxLen);
    ASSERT_EQ(ret, ERR_OK);

    std::string setText = "123456";
    PasteData setData;
    setData.AddTextRecord(setText);
    ret = PasteboardClient::GetInstance()->SetPasteData(setData);
    EXPECT_EQ(ret, static_cast<int32_t>(PasteboardError::E_OK));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(observer->errCode_, IPasteboardDisposableObserver::ERR_OK);
    EXPECT_STREQ(observer->text_.c_str(), setText.c_str());
}

/**
 * @tc.name: TestDisposable002
 * @tc.desc: should callback timeout when subscribe disposable observer but not set paste data
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDisposableClientTest, TestDisposable002, TestSize.Level0)
{
    DisposableType type = DisposableType::PLAIN_TEXT;
    uint32_t maxLen = 100;
    int32_t windowId = windowId_;
    sptr<DisposableObserverImpl> observer = sptr<DisposableObserverImpl>::MakeSptr();
    int32_t ret = PasteboardClient::GetInstance()->SubscribeDisposableObserver(observer, windowId, type, maxLen);
    ASSERT_EQ(ret, ERR_OK);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(observer->errCode_, IPasteboardDisposableObserver::ERR_TIMEOUT);
    EXPECT_STREQ(observer->text_.c_str(), "");
}

/**
 * @tc.name: TestDisposable003
 * @tc.desc: should get paste data failed when set paste data with disposable observer
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDisposableClientTest, TestDisposable003, TestSize.Level0)
{
    DisposableType type = DisposableType::PLAIN_TEXT;
    uint32_t maxLen = 100;
    int32_t windowId = windowId_;
    sptr<DisposableObserverImpl> observer = sptr<DisposableObserverImpl>::MakeSptr();
    int32_t ret = PasteboardClient::GetInstance()->SubscribeDisposableObserver(observer, windowId, type, maxLen);
    ASSERT_EQ(ret, ERR_OK);

    std::string setText = "123456";
    PasteData setData;
    setData.AddTextRecord(setText);
    ret = PasteboardClient::GetInstance()->SetPasteData(setData);
    ASSERT_EQ(ret, static_cast<int32_t>(PasteboardError::E_OK));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(observer->errCode_, IPasteboardDisposableObserver::ERR_OK);
    EXPECT_STREQ(observer->text_.c_str(), setText.c_str());

    PasteData getData;
    ret = PasteboardClient::GetInstance()->GetPasteData(getData);
    EXPECT_NE(ret, static_cast<int32_t>(PasteboardError::E_OK));
}

/**
 * @tc.name: TestDisposable004
 * @tc.desc: should get paste data success when set paste data without disposable observer
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDisposableClientTest, TestDisposable004, TestSize.Level0)
{
    DisposableType type = DisposableType::PLAIN_TEXT;
    uint32_t maxLen = 100;
    int32_t windowId = windowId_;
    sptr<DisposableObserverImpl> observer = sptr<DisposableObserverImpl>::MakeSptr();
    int32_t ret = PasteboardClient::GetInstance()->SubscribeDisposableObserver(observer, windowId, type, maxLen);
    ASSERT_EQ(ret, ERR_OK);

    std::string setText = "123456";
    PasteData setData;
    setData.AddTextRecord(setText);
    ret = PasteboardClient::GetInstance()->SetPasteData(setData);
    ASSERT_EQ(ret, static_cast<int32_t>(PasteboardError::E_OK));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(observer->errCode_, IPasteboardDisposableObserver::ERR_OK);
    EXPECT_STREQ(observer->text_.c_str(), setText.c_str());

    ret = PasteboardClient::GetInstance()->SetPasteData(setData);
    ASSERT_EQ(ret, static_cast<int32_t>(PasteboardError::E_OK));

    PasteData getData;
    ret = PasteboardClient::GetInstance()->GetPasteData(getData);
    ASSERT_EQ(ret, static_cast<int32_t>(PasteboardError::E_OK));
    std::shared_ptr<std::string> getText = getData.GetPrimaryText();
    ASSERT_TRUE(getText != nullptr);
    EXPECT_STREQ(getText->c_str(), setText.c_str());
}

/**
 * @tc.name: TestDisposable005
 * @tc.desc: should callback ERR_DATA_IN_APP when set paste data with ShareOption::InApp
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDisposableClientTest, TestDisposable005, TestSize.Level0)
{
    DisposableType type = DisposableType::PLAIN_TEXT;
    uint32_t maxLen = 100;
    int32_t windowId = windowId_;
    sptr<DisposableObserverImpl> observer = sptr<DisposableObserverImpl>::MakeSptr();
    int32_t ret = PasteboardClient::GetInstance()->SubscribeDisposableObserver(observer, windowId, type, maxLen);
    ASSERT_EQ(ret, ERR_OK);

    std::string setText = "123456";
    PasteData setData;
    setData.AddTextRecord(setText);
    setData.SetShareOption(ShareOption::InApp);
    ret = PasteboardClient::GetInstance()->SetPasteData(setData);
    ASSERT_EQ(ret, static_cast<int32_t>(PasteboardError::E_OK));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    EXPECT_EQ(observer->errCode_, IPasteboardDisposableObserver::ERR_DATA_IN_APP);
    EXPECT_STREQ(observer->text_.c_str(), "");
}
} // namespace OHOS::MiscServices
