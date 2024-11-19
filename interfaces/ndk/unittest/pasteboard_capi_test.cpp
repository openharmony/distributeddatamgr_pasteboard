/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "PasteboardCapiTest"

#include <gtest/gtest.h>
#include <unistd.h>

#include "token_setproc.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"

#include "oh_pasteboard.h"
#include "oh_pasteboard_err_code.h"
#include "oh_pasteboard_observer_impl.h"
#include "udmf.h"
#include "uds.h"
#include "pasteboard_hilog.h"
#include "pasteboard_client.h"
#include "os_account_manager.h"

using namespace testing::ext;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::MiscServices;

namespace OHOS::Test {
class PasteboardCapiTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static void CallbackFunc(void* context, Pasteboard_NotifyType type);
    static void RemoveCallbackSideEffects();
    static void ContextFinalizeFunc(void* context);
    static int callbackValue;
    static void AllocTestTokenId();
    static void DeleteTestTokenId();
    static void SetTestTokenId();
    static void RestoreSelfTokenId();
    static void* GetDataCallback(void* context, const char* type);
    static constexpr int INIT_VALUE = 0;
    static constexpr int UPDATE_VALUE = 1;
    static uint64_t selfTokenId_;
    static AccessTokenID testTokenId_;
    static constexpr char PLAINTEXT_CONTENT[] = "PLAINTEXT_CONTENT";
    static constexpr char HYPERLINK_URL[] = "file://data/image.png";
    static constexpr char HTML_URL[] = "file://data/image.png";
    static constexpr char HTML_TEXT[] = "<P>html text</P>";
};
uint64_t PasteboardCapiTest::selfTokenId_ = 0;
AccessTokenID PasteboardCapiTest::testTokenId_ = 0;
int PasteboardCapiTest::callbackValue = 0;

void PasteboardCapiTest::SetUpTestCase(void)
{
    callbackValue = INIT_VALUE;
    selfTokenId_ = GetSelfTokenID();
    AllocTestTokenId();
}

void PasteboardCapiTest::TearDownTestCase(void)
{
    RemoveCallbackSideEffects();
    DeleteTestTokenId();
}

void PasteboardCapiTest::SetUp(void)
{
}

void PasteboardCapiTest::TearDown(void)
{
}

void PasteboardCapiTest::AllocTestTokenId()
{
    std::vector<int32_t> ids;
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "query active user failed errCode = %{public}d", ret);
        return;
    }
    HapInfoParams infoParams = {
        .userID = ids[0],
        .bundleName = "ohos.privacy_test.pasteboard",
        .instIndex = 0,
        .appIDDesc = "privacy_test.pasteboard"
    };
    PermissionStateFull testState = {
        .permissionName = "ohos.permission.DUMP",
        .isGeneral = true,
        .resDeviceID = { "local" },
        .grantStatus = { PermissionState::PERMISSION_GRANTED },
        .grantFlags = { 1 }
    };
    HapPolicyParams policyParams = {
        .apl = APL_NORMAL,
        .domain = "test.domain.pasteboard",
        .permList = {},
        .permStateList = { testState }
    };

    AccessTokenKit::AllocHapToken(infoParams, policyParams);
    testTokenId_ = Security::AccessToken::AccessTokenKit::GetHapTokenID(
        infoParams.userID, infoParams.bundleName, infoParams.instIndex);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "userID = %{public}d, testTokenId = 0x%{public}x.", infoParams.userID,
        testTokenId_);
}

void PasteboardCapiTest::DeleteTestTokenId()
{
    AccessTokenKit::DeleteToken(testTokenId_);
}


void PasteboardCapiTest::SetTestTokenId()
{
    auto ret = SetSelfTokenID(testTokenId_);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "testTokenId = 0x%{public}x, ret = %{public}d!", testTokenId_, ret);
}

void PasteboardCapiTest::RestoreSelfTokenId()
{
    auto ret = SetSelfTokenID(selfTokenId_);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ret = %{public}d!", ret);
}

void PasteboardCapiTest::CallbackFunc(void* context, Pasteboard_NotifyType type)
{
    callbackValue = UPDATE_VALUE;
}

void PasteboardCapiTest::RemoveCallbackSideEffects()
{
    callbackValue = INIT_VALUE;
}

void PasteboardCapiTest::ContextFinalizeFunc(void* context) {}

void* PasteboardCapiTest::GetDataCallback(void* context, const char* type)
{
    if (std::string(type) == "general.plain-text") {
        OH_UdsPlainText* plainText = OH_UdsPlainText_Create();
        OH_UdsPlainText_SetContent(plainText, PLAINTEXT_CONTENT);
        return plainText;
    } else if (std::string(type) == "general.hyperlink") {
        OH_UdsHyperlink* link = OH_UdsHyperlink_Create();
        OH_UdsHyperlink_SetUrl(link, HYPERLINK_URL);
        return link;
    } else if (std::string(type) == "general.html") {
        OH_UdsHtml* html = OH_UdsHtml_Create();
        OH_UdsHtml_SetContent(html, HTML_URL);
        return html;
    }
    return nullptr;
}

/**
 * @tc.name: OH_PasteboardSubscriber_Create001
 * @tc.desc: OH_PasteboardObserver_Create test
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_PasteboardSubscriber_Create001, TestSize.Level1)
{
    OH_PasteboardObserver* observer = OH_PasteboardObserver_Create();
    EXPECT_NE(observer, nullptr);

    OH_PasteboardObserver_Destroy(observer);
    OH_PasteboardObserver_Destroy(nullptr);
}

/**
 * @tc.name: OH_PasteboardObserver_SetData001
 * @tc.desc: OH_PasteboardObserver_SetData test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_PasteboardObserver_SetData001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    void* context = static_cast<void*>(pasteboard);
    OH_PasteboardObserver* observer = OH_PasteboardObserver_Create();

    int setRes1 = OH_PasteboardObserver_SetData(observer, context, CallbackFunc, ContextFinalizeFunc);
    EXPECT_EQ(setRes1, ERR_OK);

    OH_PasteboardObserver_Destroy(observer);
    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_PasteboardObserver_SetData002
 * @tc.desc: OH_PasteboardObserver_SetData test invalid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_PasteboardObserver_SetData002, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    void* context = static_cast<void*>(pasteboard);
    OH_PasteboardObserver* observer = OH_PasteboardObserver_Create();

    int setRes1 = OH_PasteboardObserver_SetData(nullptr, context, CallbackFunc, ContextFinalizeFunc);
    EXPECT_EQ(setRes1, ERR_INVALID_PARAMETER);

    int setRes2 = OH_PasteboardObserver_SetData(observer, context, nullptr, ContextFinalizeFunc);
    EXPECT_EQ(setRes2, ERR_INVALID_PARAMETER);

    int setRes3 = OH_PasteboardObserver_SetData(observer, context, CallbackFunc, nullptr);
    EXPECT_EQ(setRes3, ERR_INVALID_PARAMETER);

    OH_PasteboardObserver_Destroy(observer);
    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_Pasteboard_Create001
 * @tc.desc: OH_Pasteboard_Create test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_Create001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    EXPECT_NE(pasteboard, nullptr);

    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_Pasteboard_Subscribe001
 * @tc.desc: OH_Pasteboard_Subscribe test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_Subscribe001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_PasteboardObserver* observer = OH_PasteboardObserver_Create();
    OH_PasteboardObserver_SetData(observer, nullptr, CallbackFunc, ContextFinalizeFunc);

    int res = OH_Pasteboard_Subscribe(pasteboard, NOTIFY_LOCAL_DATA_CHANGE, observer);
    EXPECT_EQ(res, ERR_OK);

    int resRepeat = OH_Pasteboard_Subscribe(pasteboard, NOTIFY_LOCAL_DATA_CHANGE, observer);
    EXPECT_EQ(resRepeat, ERR_OK);

    OH_Pasteboard_Destroy(pasteboard);
    OH_PasteboardObserver_Destroy(observer);
}

/**
 * @tc.name: OH_Pasteboard_Subscribe002
 * @tc.desc: OH_Pasteboard_Subscribe test invalid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_Subscribe002, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_PasteboardObserver* observer = OH_PasteboardObserver_Create();
    OH_PasteboardObserver_SetData(observer, nullptr, CallbackFunc, ContextFinalizeFunc);

    int res1 = OH_Pasteboard_Subscribe(nullptr, NOTIFY_LOCAL_DATA_CHANGE, observer);
    EXPECT_EQ(res1, ERR_INVALID_PARAMETER);

    int res2 = OH_Pasteboard_Subscribe(pasteboard, 10, observer);
    EXPECT_EQ(res2, ERR_INVALID_PARAMETER);

    int res3 = OH_Pasteboard_Subscribe(pasteboard, -1, observer);
    EXPECT_EQ(res3, ERR_INVALID_PARAMETER);

    int res4 = OH_Pasteboard_Subscribe(pasteboard, NOTIFY_LOCAL_DATA_CHANGE, nullptr);
    EXPECT_EQ(res4, ERR_INVALID_PARAMETER);

    OH_Pasteboard_Destroy(pasteboard);
    OH_PasteboardObserver_Destroy(observer);
}

/**
 * @tc.name: OH_Pasteboard_Unsubcribe001
 * @tc.desc: OH_Pasteboard_Unsubcribe test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_Unsubcribe001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_PasteboardObserver* observer = OH_PasteboardObserver_Create();
    OH_PasteboardObserver_SetData(observer, nullptr, CallbackFunc, ContextFinalizeFunc);

    OH_Pasteboard_Subscribe(pasteboard, NOTIFY_LOCAL_DATA_CHANGE, observer);

    int res = OH_Pasteboard_Unsubscribe(pasteboard, NOTIFY_LOCAL_DATA_CHANGE, observer);
    EXPECT_EQ(res, ERR_OK);

    OH_Pasteboard_Destroy(pasteboard);
    OH_PasteboardObserver_Destroy(observer);
}

/**
 * @tc.name: OH_Pasteboard_Unsubscribe002
 * @tc.desc: OH_Pasteboard_Unsubscribe test invalid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_Unsubcribe002, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_PasteboardObserver* observer = OH_PasteboardObserver_Create();
    OH_PasteboardObserver_SetData(observer, nullptr, CallbackFunc, ContextFinalizeFunc);

    int res1 = OH_Pasteboard_Unsubscribe(nullptr, NOTIFY_LOCAL_DATA_CHANGE, observer);
    EXPECT_EQ(res1, ERR_INVALID_PARAMETER);

    int res2 = OH_Pasteboard_Unsubscribe(pasteboard, 10, observer);
    EXPECT_EQ(res2, ERR_INVALID_PARAMETER);

    int res3 = OH_Pasteboard_Unsubscribe(pasteboard, -1, observer);
    EXPECT_EQ(res3, ERR_INVALID_PARAMETER);

    int res4 = OH_Pasteboard_Unsubscribe(pasteboard, NOTIFY_LOCAL_DATA_CHANGE, nullptr);
    EXPECT_EQ(res4, ERR_INVALID_PARAMETER);

    OH_Pasteboard_Destroy(pasteboard);
    OH_PasteboardObserver_Destroy(observer);
}

/**
 * @tc.name: OH_Pasteboard_IsRemoteData001
 * @tc.desc: OH_Pasteboard_IsRemoteData test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_IsRemoteData001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();

    bool res = OH_Pasteboard_IsRemoteData(pasteboard);
    EXPECT_FALSE(res);

    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_Pasteboard_GetDataSrouce001
 * @tc.desc: OH_Pasteboard_GetDataSrouce test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetDataSrouce001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_UdmfData* setData = OH_UdmfData_Create();
    OH_UdmfRecord* record = OH_UdmfRecord_Create();
    OH_UdsPlainText* plainText = OH_UdsPlainText_Create();
    char content[] = "hello world";
    OH_UdsPlainText_SetContent(plainText, content);
    OH_UdmfRecord_AddPlainText(record, plainText);
    OH_UdmfData_AddRecord(setData, record);

    OH_Pasteboard_SetData(pasteboard, setData);

    int len = 100;
    char source[100];
    int res = OH_Pasteboard_GetDataSource(pasteboard, source, len);
    EXPECT_EQ(res, ERR_OK);

    int res1 = OH_Pasteboard_GetDataSource(nullptr, source, len);
    EXPECT_EQ(res1, ERR_INVALID_PARAMETER);

    int res2 = OH_Pasteboard_GetDataSource(pasteboard, nullptr, len);
    EXPECT_EQ(res2, ERR_INVALID_PARAMETER);

    int res3 = OH_Pasteboard_GetDataSource(pasteboard, source, 0);
    EXPECT_EQ(res3, ERR_INVALID_PARAMETER);
    
    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_Pasteboard_GetMimeTypes001
 * @tc.desc: OH_Pasteboard_GetMimeTypes test empty data
 * @tc.type: FUNC
 * @tc.require: AR20241012964265
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetMimeTypes001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    ASSERT_TRUE(pasteboard);
    OH_Pasteboard_ClearData(pasteboard);
    unsigned int count = 1000;
    char** res = OH_Pasteboard_GetMimeTypes(pasteboard, &count);
    EXPECT_EQ(0, count);
    EXPECT_TRUE(res == nullptr);
}

/**
 * @tc.name: OH_Pasteboard_GetMimeTypes002
 * @tc.desc: OH_Pasteboard_GetMimeTypes test plainText
 * @tc.type: FUNC
 * @tc.require: AR20241012964265
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetMimeTypes002, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    ASSERT_TRUE(pasteboard);
    OH_UdmfData* setData = OH_UdmfData_Create();
    ASSERT_TRUE(setData);
    OH_UdmfRecord* record = OH_UdmfRecord_Create();
    ASSERT_TRUE(record);
    OH_UdsPlainText* plainText = OH_UdsPlainText_Create();
    ASSERT_TRUE(plainText);
    char content[] = "hello world";
    OH_UdsPlainText_SetContent(plainText, content);
    OH_UdmfRecord_AddPlainText(record, plainText);
    OH_UdmfData_AddRecord(setData, record);
    OH_Pasteboard_SetData(pasteboard, setData);

    unsigned int count = 1000;
    char** res = OH_Pasteboard_GetMimeTypes(pasteboard, &count);
    EXPECT_EQ(1, count);
    EXPECT_TRUE(res != nullptr);
    EXPECT_STREQ(MIMETYPE_TEXT_PLAIN, res[0]);
    
    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_Pasteboard_GetMimeTypes003
 * @tc.desc: OH_Pasteboard_GetMimeTypes test multi Mime types
 * @tc.type: FUNC
 * @tc.require: AR20241012964265
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetMimeTypes003, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    ASSERT_TRUE(pasteboard);
    OH_UdmfData* setData = OH_UdmfData_Create();
    ASSERT_TRUE(setData);
    OH_UdmfRecord* record = OH_UdmfRecord_Create();
    ASSERT_TRUE(record);
    OH_UdsPlainText* plainText = OH_UdsPlainText_Create();
    ASSERT_TRUE(plainText);
    char content[] = "hello world";
    OH_UdsPlainText_SetContent(plainText, content);
    OH_UdmfRecord_AddPlainText(record, plainText);
    OH_UdmfData_AddRecord(setData, record);

    OH_UdmfRecord* record2 = OH_UdmfRecord_Create();
    ASSERT_TRUE(record2);
    OH_UdsHtml* htmlText = OH_UdsHtml_Create();
    ASSERT_TRUE(htmlText);
    char html[] = "<div class='disabled'>hello</div>";
    OH_UdsHtml_SetContent(htmlText, html);
    OH_UdmfRecord_AddHtml(record2, htmlText);
    OH_UdmfData_AddRecord(setData, record2);
    OH_Pasteboard_SetData(pasteboard, setData);

    unsigned int count = 1000;
    char** res = OH_Pasteboard_GetMimeTypes(pasteboard, &count);
    EXPECT_EQ(2, count);
    EXPECT_TRUE(res != nullptr);
    EXPECT_TRUE((strcmp(MIMETYPE_TEXT_PLAIN, res[0]) == 0 && strcmp(MIMETYPE_TEXT_HTML, res[1]) == 0) ||
        (strcmp(MIMETYPE_TEXT_PLAIN, res[1]) == 0 && strcmp(MIMETYPE_TEXT_HTML, res[0]) == 0));
    
    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_Pasteboard_HasType001
 * @tc.desc: OH_Pasteboard_HasType test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_HasType001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_UdmfData* setData = OH_UdmfData_Create();
    OH_UdmfRecord* record = OH_UdmfRecord_Create();
    OH_UdsPlainText* plainText = OH_UdsPlainText_Create();
    char content[] = "hello world";
    OH_UdsPlainText_SetContent(plainText, content);
    OH_UdmfRecord_AddPlainText(record, plainText);
    OH_UdmfData_AddRecord(setData, record);

    OH_Pasteboard_SetData(pasteboard, setData);
    char type[] = "general.plain-text";
    bool res = OH_Pasteboard_HasType(pasteboard, type);
    EXPECT_FALSE(res);
    
    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_Pasteboard_HasData001
 * @tc.desc: OH_Pasteboard_HasData test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_HasData001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_Pasteboard_ClearData(pasteboard);
    bool res = OH_Pasteboard_HasData(pasteboard);
    EXPECT_FALSE(res);

    OH_UdmfData* setData = OH_UdmfData_Create();
    OH_UdmfRecord* record = OH_UdmfRecord_Create();
    OH_UdsPlainText* plainText = OH_UdsPlainText_Create();
    char content[] = "hello world";
    OH_UdsPlainText_SetContent(plainText, content);
    OH_UdmfRecord_AddPlainText(record, plainText);
    OH_UdmfData_AddRecord(setData, record);

    OH_Pasteboard_SetData(pasteboard, setData);

    res = OH_Pasteboard_HasData(pasteboard);
    EXPECT_TRUE(res);
    
    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_Pasteboard_ClearData001
 * @tc.desc: OH_Pasteboard_ClearData test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_ClearData001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    bool res = OH_Pasteboard_ClearData(pasteboard);
    EXPECT_EQ(res, ERR_OK);

    int res2 = OH_Pasteboard_ClearData(nullptr);
    EXPECT_EQ(res2, ERR_INVALID_PARAMETER);
    
    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: OH_Pasteboard_SetData001
 * @tc.desc: OH_Pasteboard_SetData test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_SetData001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_UdmfData *setData = OH_UdmfData_Create();
    OH_UdmfRecord *record = OH_UdmfRecord_Create();
    OH_UdsPlainText *plainText = OH_UdsPlainText_Create();
    char content[] = "hello world";
    OH_UdsPlainText_SetContent(plainText, content);
    OH_UdmfRecord_AddPlainText(record, plainText);
    OH_UdmfData_AddRecord(setData, record);

    bool res = OH_Pasteboard_SetData(pasteboard, setData);
    EXPECT_EQ(res, ERR_OK);

    int res1 = OH_Pasteboard_SetData(pasteboard, nullptr);
    EXPECT_EQ(res1, ERR_INVALID_PARAMETER);

    int res2 = OH_Pasteboard_SetData(nullptr, setData);
    EXPECT_EQ(res2, ERR_INVALID_PARAMETER);
    
    OH_Pasteboard_Destroy(pasteboard);
    OH_UdsPlainText_Destroy(plainText);
    OH_UdmfRecord_Destroy(record);
    OH_UdmfData_Destroy(setData);
}

/**
 * @tc.name: OH_Pasteboard_GetData001
 * @tc.desc: OH_Pasteboard_GetData test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetData001, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_UdmfData* setData = OH_UdmfData_Create();
    OH_UdmfRecord* record = OH_UdmfRecord_Create();
    OH_UdsPlainText* plainText = OH_UdsPlainText_Create();
    char content[] = "hello world";
    OH_UdsPlainText_SetContent(plainText, content);
    OH_UdmfRecord_AddPlainText(record, plainText);
    OH_UdmfData_AddRecord(setData, record);

    int res = OH_Pasteboard_SetData(pasteboard, setData);
    EXPECT_EQ(res, ERR_OK);

    int status = -1;
    OH_UdmfData* getData = OH_Pasteboard_GetData(pasteboard, &status);
    EXPECT_EQ(status, ERR_OK);
    EXPECT_NE(getData, nullptr);

    unsigned int count = 0;
    OH_UdmfRecord **getRecords = OH_UdmfData_GetRecords(getData, &count);
    EXPECT_EQ(count, 1);
    OH_UdsPlainText *getPlainText = OH_UdsPlainText_Create();
    OH_UdmfRecord_GetPlainText(getRecords[0], getPlainText);
    const char *getContent = OH_UdsPlainText_GetContent(getPlainText);
    EXPECT_EQ(strcmp(getContent, content), 0);

    OH_Pasteboard_Destroy(pasteboard);
    OH_UdsPlainText_Destroy(plainText);
    OH_UdsPlainText_Destroy(getPlainText);
    OH_UdmfRecord_Destroy(record);
    OH_UdmfData_Destroy(setData);
    OH_UdmfData_Destroy(getData);
}

/**
 * @tc.name: OH_Pasteboard_GetData002
 * @tc.desc: OH_Pasteboard_GetData test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetData002, TestSize.Level1)
{
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_UdmfData* setData = OH_UdmfData_Create();
    OH_UdmfRecord* record = OH_UdmfRecord_Create();
    OH_UdmfRecordProvider* provider = OH_UdmfRecordProvider_Create();
    EXPECT_NE(provider, nullptr);
    OH_UdmfRecordProvider_SetData(provider, static_cast<void *>(record), GetDataCallback, ContextFinalizeFunc);
    OH_UdmfData_AddRecord(setData, record);

    const char* types[3] = { "general.plain-text", "general.hyperlink", "general.html" };
    OH_UdmfRecord_SetProvider(record, types, 3, provider);
    int res = OH_Pasteboard_SetData(pasteboard, setData);
    EXPECT_EQ(res, ERR_OK);

    int status = -1;
    OH_UdmfData* getData = OH_Pasteboard_GetData(pasteboard, &status);
    EXPECT_EQ(status, ERR_OK);
    EXPECT_NE(getData, nullptr);

    unsigned int count = 0;
    OH_UdmfRecord **getRecords = OH_UdmfData_GetRecords(getData, &count);
    EXPECT_EQ(count, 1);
    OH_UdsPlainText *getPlainText = OH_UdsPlainText_Create();
    OH_UdmfRecord_GetPlainText(getRecords[0], getPlainText);
    const char *getContent = OH_UdsPlainText_GetContent(getPlainText);
    EXPECT_EQ(strcmp(getContent, PLAINTEXT_CONTENT), 0);

    OH_UdsHyperlink *getHyperLink = OH_UdsHyperlink_Create();
    OH_UdmfRecord_GetHyperlink(getRecords[0], getHyperLink);
    const char *getUrl = OH_UdsHyperlink_GetUrl(getHyperLink);
    EXPECT_EQ(strcmp(getUrl, HYPERLINK_URL), 0);
    OH_Pasteboard_Destroy(pasteboard);
    OH_UdsPlainText_Destroy(getPlainText);
    OH_UdsHyperlink_Destroy(getHyperLink);
    OH_UdmfRecord_Destroy(record);
    OH_UdmfData_Destroy(setData);
    OH_UdmfData_Destroy(getData);
}

/**
 * @tc.name: OH_Pasteboard_GetData003
 * @tc.desc: OH_Pasteboard_GetData test valid
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetData003, TestSize.Level1)
{
    char typeId[] = "ApplicationDefined-myType";
    unsigned char entry[] = "CreateGeneralRecord1";
    unsigned int count = sizeof(entry);
    OH_UdmfRecord *record = OH_UdmfRecord_Create();
    int addRes1 = OH_UdmfRecord_AddGeneralEntry(record, typeId, entry, count);
    EXPECT_EQ(addRes1, ERR_OK);

    OH_UdmfData* setData = OH_UdmfData_Create();
    OH_UdmfData_AddRecord(setData, record);
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    int res = OH_Pasteboard_SetData(pasteboard, setData);
    EXPECT_EQ(res, ERR_OK);

    int status = -1;
    OH_UdmfData* getData = OH_Pasteboard_GetData(pasteboard, &status);
    EXPECT_EQ(status, ERR_OK);
    EXPECT_NE(getData, nullptr);

    unsigned int getrecordCount = 0;
    OH_UdmfRecord **getRecords = OH_UdmfData_GetRecords(getData, &getrecordCount);
    EXPECT_EQ(getrecordCount, 1);

    unsigned int getCount = 0;
    unsigned char *getEntry;
    int getRes = OH_UdmfRecord_GetGeneralEntry(getRecords[0], typeId, &getEntry, &getCount);
    EXPECT_EQ(getRes, ERR_OK);
    EXPECT_EQ(getCount, count);
    EXPECT_EQ(memcmp(entry, getEntry, getCount), 0);

    OH_Pasteboard_Destroy(pasteboard);
    OH_UdmfRecord_Destroy(record);
    OH_UdmfData_Destroy(setData);
    OH_UdmfData_Destroy(getData);
}

/**
 * @tc.name: OH_Pasteboard_GetData004
 * @tc.desc: OH_Pasteboard_GetData test data type
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetData004, TestSize.Level1)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OH_Pasteboard_GetData004 start");
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_UdmfData* setData = OH_UdmfData_Create();
    OH_UdmfRecord* record = OH_UdmfRecord_Create();

    OH_UdmfRecordProvider* provider = OH_UdmfRecordProvider_Create();
    EXPECT_NE(provider, nullptr);
    OH_UdmfRecordProvider_SetData(provider, static_cast<void *>(record), GetDataCallback, ContextFinalizeFunc);

    const char* types[3] = { "general.plain-text", "general.hyperlink", "general.html" };
    OH_UdmfRecord_SetProvider(record, types, 3, provider);
    OH_UdmfData_AddRecord(setData, record);

    int res = OH_Pasteboard_SetData(pasteboard, setData);
    EXPECT_EQ(res, ERR_OK);
    
    PasteData pasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));

    auto record1 = pasteData.GetRecordAt(0);
    auto mimeType = record1->GetMimeType();
    EXPECT_EQ(mimeType, MIMETYPE_TEXT_PLAIN);

    OH_Pasteboard_Destroy(pasteboard);
    OH_UdmfRecord_Destroy(record);
    OH_UdmfData_Destroy(setData);
    OH_UdmfRecordProvider_Destroy(provider);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OH_Pasteboard_GetData004 end");
}

/**
 * @tc.name: OH_Pasteboard_GetData005
 * @tc.desc: OH_Pasteboard_GetData test data type
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetData005, TestSize.Level1)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OH_Pasteboard_GetData005 start");
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_UdmfData* setData = OH_UdmfData_Create();
    OH_UdmfRecord* record = OH_UdmfRecord_Create();

    OH_UdmfRecordProvider* provider = OH_UdmfRecordProvider_Create();
    EXPECT_NE(provider, nullptr);
    OH_UdmfRecordProvider_SetData(provider, static_cast<void *>(record), GetDataCallback, ContextFinalizeFunc);
    const char* types[3] = { "general.plain-text", "general.hyperlink", "general.html" };
    OH_UdmfRecord_SetProvider(record, types, 3, provider);

    OH_UdsHyperlink *link = OH_UdsHyperlink_Create();
    OH_UdsHyperlink_SetUrl(link, HYPERLINK_URL);
    OH_UdmfRecord_AddHyperlink(record, link);

    OH_UdmfData_AddRecord(setData, record);
    auto res = OH_Pasteboard_SetData(pasteboard, setData);
    EXPECT_EQ(res, ERR_OK);
    
    PasteData pasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));

    auto record1 = pasteData.GetRecordAt(0);
    auto mimeType = record1->GetMimeType();
    EXPECT_EQ(mimeType, MIMETYPE_TEXT_PLAIN);

    auto text = record1->GetPlainText();
    EXPECT_EQ(strcmp(text->c_str(), PLAINTEXT_CONTENT), 0);

    OH_Pasteboard_Destroy(pasteboard);
    OH_UdmfRecord_Destroy(record);
    OH_UdmfData_Destroy(setData);
    OH_UdsHyperlink_Destroy(link);
    OH_UdmfRecordProvider_Destroy(provider);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OH_Pasteboard_GetData005 end");
}

/**
 * @tc.name: OH_Pasteboard_GetData006
 * @tc.desc: OH_Pasteboard_GetData test data type
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetData006, TestSize.Level1)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OH_Pasteboard_GetData006 start");
    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    OH_UdmfData* setData = OH_UdmfData_Create();
    OH_UdmfRecord* record = OH_UdmfRecord_Create();

    OH_UdsHtml *html = OH_UdsHtml_Create();
    OH_UdsHtml_SetContent(html, HTML_TEXT);
    OH_UdmfRecord_AddHtml(record, html);

    OH_UdmfRecordProvider* provider = OH_UdmfRecordProvider_Create();
    EXPECT_NE(provider, nullptr);
    OH_UdmfRecordProvider_SetData(provider, static_cast<void *>(record), GetDataCallback, ContextFinalizeFunc);
    const char* types[3] = { "general.plain-text", "general.hyperlink", "general.html" };
    OH_UdmfRecord_SetProvider(record, types, 3, provider);

    OH_UdmfData_AddRecord(setData, record);
    bool res = OH_Pasteboard_SetData(pasteboard, setData);
    EXPECT_EQ(res, ERR_OK);
    
    PasteData pasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));

    auto record1 = pasteData.GetRecordAt(0);
    auto mimeType = record1->GetMimeType();
    EXPECT_EQ(mimeType, MIMETYPE_TEXT_HTML);

    auto html1 = record1->GetHtmlText();
    EXPECT_EQ(strcmp(html1->c_str(), HTML_TEXT), 0);

    OH_Pasteboard_Destroy(pasteboard);
    OH_UdmfRecord_Destroy(record);
    OH_UdmfData_Destroy(setData);
    OH_UdmfRecordProvider_Destroy(provider);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OH_Pasteboard_GetData006 end");
}

/**
 * @tc.name: OH_Pasteboard_GetData007
 * @tc.desc: OH_Pasteboard_GetData test data type
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_GetData007, TestSize.Level1)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OH_Pasteboard_GetData007 start");
    std::string plainText = "helloWorld";
    auto newData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    auto ret = PasteboardClient::GetInstance()->SetPasteData(*newData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));

    OH_Pasteboard* pasteboard = OH_Pasteboard_Create();
    int status = -1;
    OH_UdmfData* getData = OH_Pasteboard_GetData(pasteboard, &status);
    EXPECT_EQ(status, ERR_OK);
    EXPECT_NE(getData, nullptr);

    unsigned int getrecordCount = 0;
    OH_UdmfRecord **getRecords = OH_UdmfData_GetRecords(getData, &getrecordCount);
    EXPECT_EQ(getrecordCount, 1);
    OH_UdsPlainText *getPlainText = OH_UdsPlainText_Create();
    OH_UdmfRecord_GetPlainText(getRecords[0], getPlainText);
    const char *getContent = OH_UdsPlainText_GetContent(getPlainText);
    EXPECT_STREQ(getContent, plainText.c_str());

    OH_Pasteboard_Destroy(pasteboard);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "OH_Pasteboard_GetData007 end");
}
}