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
#include <gmock/gmock.h>

#include "datashare_delegate_mock.h"
#include "dev_profile.h"
#include "pasteboard_hilog.h"
#include "pasteboard_switch.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MiscServices;

namespace OHOS {
namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr int32_t TEST_USER_ID_2 = 101;
} // namespace

class PasteboardSwitchTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardSwitchTest SetUpTestCase");
    }

    static void TearDownTestCase(void)
    {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardSwitchTest TearDownTestCase");
    }

    void SetUp()
    {
        mock_ = new DataShareDelegateMock();
        DataShareDelegateMock::SetMock(mock_);
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardSwitchTest SetUp");
    }

    void TearDown()
    {
        DataShareDelegateMock::SetMock(nullptr);
        delete mock_;
        mock_ = nullptr;
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardSwitchTest TearDown");
    }

protected:
    DataShareDelegateMock *mock_ = nullptr;
};

namespace MiscServices {

/**
 * @tc.name: GetWifiSwitchTest001
 * @tc.desc: test GetWifiSwitch returns true when value is empty
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardSwitchTest, GetWifiSwitchTest001, TestSize.Level1)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest001 start");
    PastedSwitch pasteSwitch;
    
    // Mock DataShareDelegate::GetValue to return empty string
    std::string emptyValue = "";
    EXPECT_CALL(*mock_, GetValue(testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgReferee<1>(emptyValue),
            testing::Return(static_cast<int32_t>(PasteboardError::QUERY_SETTING_NO_DATA_ERROR))));
    
    // When value is empty, GetWifiSwitch should return true
    bool result = pasteSwitch.GetWifiSwitch(TEST_USER_ID);
    EXPECT_TRUE(result);
    
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest001 end");
}

/**
 * @tc.name: GetWifiSwitchTest002
 * @tc.desc: test GetWifiSwitch returns true when value is "1"
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardSwitchTest, GetWifiSwitchTest002, TestSize.Level1)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest002 start");
    PastedSwitch pasteSwitch;
    
    // Mock DataShareDelegate::GetValue to return "1"
    std::string value = "1";
    EXPECT_CALL(*mock_, GetValue(testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgReferee<1>(value),
            testing::Return(static_cast<int32_t>(PasteboardError::E_OK))));
    
    // When value is "1", GetWifiSwitch should return true
    bool result = pasteSwitch.GetWifiSwitch(TEST_USER_ID);
    EXPECT_TRUE(result);
    
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest002 end");
}

/**
 * @tc.name: GetWifiSwitchTest003
 * @tc.desc: test GetWifiSwitch returns false when value is "0"
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardSwitchTest, GetWifiSwitchTest003, TestSize.Level1)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest003 start");
    PastedSwitch pasteSwitch;
    
    // Mock DataShareDelegate::GetValue to return "0"
    std::string value = "0";
    EXPECT_CALL(*mock_, GetValue(testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgReferee<1>(value),
            testing::Return(static_cast<int32_t>(PasteboardError::E_OK))));
    
    // When value is "0", GetWifiSwitch should return false
    bool result = pasteSwitch.GetWifiSwitch(TEST_USER_ID);
    EXPECT_FALSE(result);
    
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest003 end");
}

/**
 * @tc.name: GetWifiSwitchTest004
 * @tc.desc: test GetWifiSwitch returns false when value is other value
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardSwitchTest, GetWifiSwitchTest004, TestSize.Level1)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest004 start");
    PastedSwitch pasteSwitch;
    
    // Mock DataShareDelegate::GetValue to return other value
    std::string value = "2";
    EXPECT_CALL(*mock_, GetValue(testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgReferee<1>(value),
            testing::Return(static_cast<int32_t>(PasteboardError::E_OK))));
    
    // When value is not "1", GetWifiSwitch should return false
    bool result = pasteSwitch.GetWifiSwitch(TEST_USER_ID);
    EXPECT_FALSE(result);
    
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest004 end");
}

/**
 * @tc.name: GetWifiSwitchTest005
 * @tc.desc: test GetWifiSwitch with different userId
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardSwitchTest, GetWifiSwitchTest005, TestSize.Level1)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest005 start");
    PastedSwitch pasteSwitch;
    
    // Mock DataShareDelegate::GetValue to return "1"
    std::string value = "1";
    EXPECT_CALL(*mock_, GetValue(testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArgReferee<1>(value),
            testing::Return(static_cast<int32_t>(PasteboardError::E_OK))));
    
    bool result = pasteSwitch.GetWifiSwitch(TEST_USER_ID_2);
    EXPECT_TRUE(result);
    
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "GetWifiSwitchTest005 end");
}

} // namespace MiscServices
} // namespace OHOS
