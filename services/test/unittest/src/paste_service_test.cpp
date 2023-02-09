/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <unistd.h>

#include <cstdint>
#include <vector>

#include "access_token.h"
#include "accesstoken_kit.h"
#include "hap_token_info.h"
#include "os_account_manager.h"
#include "pasteboard_client.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_observer_callback.h"
#include "permission_state_full.h"
#include "pixel_map.h"
#include "token_setproc.h"
#include "uri.h"
#include "want.h"
namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace OHOS::Media;
using namespace OHOS::Security::AccessToken;
constexpr const char *CMD = "hidumper -s 3701 -a --data";
constexpr const uint16_t EACH_LINE_LENGTH = 50;
constexpr const uint16_t TOTAL_LENGTH = 500;
class PasteboardServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static bool ExecuteCmd(std::string &result);

    static void AllocTestTokenId();
    static void DeleteTestTokenId();
    static void SetTestTokenId();
    static void RestoreSelfTokenId();
    static sptr<PasteboardObserver> pasteboardObserver_;
    static sptr<PasteboardObserver> pasteboardEventObserver_;
    static std::atomic_bool pasteboardChangedFlag_;
    static std::atomic_int32_t pasteboardEventStatus_;
    static uint64_t selfTokenId_;
    static AccessTokenID testTokenId_;
};
std::atomic_bool PasteboardServiceTest::pasteboardChangedFlag_ = false;
std::atomic_int32_t PasteboardServiceTest::pasteboardEventStatus_ = -1;
sptr<PasteboardObserver> PasteboardServiceTest::pasteboardObserver_ = nullptr;
sptr<PasteboardObserver> PasteboardServiceTest::pasteboardEventObserver_ = nullptr;
uint64_t PasteboardServiceTest::selfTokenId_ = 0;
AccessTokenID PasteboardServiceTest::testTokenId_ = 0;

void PasteboardServiceTest::SetUpTestCase(void)
{
    selfTokenId_ = GetSelfTokenID();
    AllocTestTokenId();
}

void PasteboardServiceTest::TearDownTestCase(void)
{
    DeleteTestTokenId();
}

void PasteboardServiceTest::SetUp(void)
{
}

void PasteboardServiceTest::TearDown(void)
{
    if (PasteboardServiceTest::pasteboardObserver_ != nullptr) {
        PasteboardClient::GetInstance()->RemovePasteboardEventObserver(PasteboardServiceTest::pasteboardObserver_);
    }
    if (PasteboardServiceTest::pasteboardEventObserver_ != nullptr) {
        PasteboardClient::GetInstance()->RemovePasteboardEventObserver(PasteboardServiceTest::pasteboardEventObserver_);
    }
    PasteboardClient::GetInstance()->Clear();
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "TearDown.");
}

void PasteboardObserverCallback::OnPasteboardChanged()
{
    PasteboardServiceTest::pasteboardChangedFlag_ = true;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "callback.");
}

void PasteboardEventObserverCallback::OnPasteboardEvent(std::string bundleName, int32_t status)
{
    PasteboardServiceTest::pasteboardEventStatus_ = status;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "event callback bundleName: %{public}s,status:%{public}d",
        bundleName.c_str(), status);
}

bool PasteboardServiceTest::ExecuteCmd(std::string &result)
{
    char buff[EACH_LINE_LENGTH] = { 0x00 };
    char output[TOTAL_LENGTH] = { 0x00 };
    FILE *ptr = NULL;
    if ((ptr = popen(CMD, "r")) != NULL) {
        while (fgets(buff, sizeof(buff), ptr) != nullptr) {
            if (strcat_s(output, sizeof(output), buff) != 0) {
                pclose(ptr);
                ptr = NULL;
                return false;
            }
        }
        pclose(ptr);
        ptr = NULL;
    } else {
        return false;
    }
    result = std::string(output);
    return true;
}

void PasteboardServiceTest::AllocTestTokenId()
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

void PasteboardServiceTest::DeleteTestTokenId()
{
    AccessTokenKit::DeleteToken(testTokenId_);
}


void PasteboardServiceTest::SetTestTokenId()
{
    auto ret = SetSelfTokenID(testTokenId_);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "testTokenId = 0x%{public}x, ret = %{public}d!", testTokenId_, ret);
}

void PasteboardServiceTest::RestoreSelfTokenId()
{
    auto ret = SetSelfTokenID(selfTokenId_);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ret = %{public}d!", ret);
}

/**
* @tc.name: PasteboardTest001
* @tc.desc: Create paste board test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteboardTest001, TestSize.Level0)
{
    auto record = PasteboardClient::GetInstance()->CreatePlainTextRecord("paste record1");
    ASSERT_TRUE(record != nullptr);
    std::string plainText = "plain text";
    auto data = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(data != nullptr);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*data);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get.");
    auto primaryText = pasteData.GetPrimaryText();
    ASSERT_TRUE(primaryText != nullptr);
    ASSERT_TRUE(*primaryText == plainText);
}

/**
* @tc.name: PasteRecordTest001
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest001, TestSize.Level0)
{
    std::string plainText = "helloWorld";
    auto record = PasteboardClient::GetInstance()->CreatePlainTextRecord(plainText);
    ASSERT_TRUE(record != nullptr);
    auto newPlainText = record->GetPlainText();
    ASSERT_TRUE(newPlainText != nullptr);
    ASSERT_TRUE(*newPlainText == plainText);
}

/**
* @tc.name: PasteRecordTest002
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest002, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto record = PasteboardClient::GetInstance()->CreateHtmlTextRecord(htmlText);
    ASSERT_TRUE(record != nullptr);
    auto newHtmlText = record->GetHtmlText();
    ASSERT_TRUE(newHtmlText != nullptr);
    ASSERT_TRUE(*newHtmlText == htmlText);
}

/**
* @tc.name: PasteRecordTest003
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest003, TestSize.Level0)
{
    using namespace OHOS::AAFwk;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    auto record = PasteboardClient::GetInstance()->CreateWantRecord(want);
    ASSERT_TRUE(record != nullptr);
    auto newWant = record->GetWant();
    ASSERT_TRUE(newWant != nullptr);
    int32_t defaultValue = 333;
    ASSERT_TRUE(newWant->GetIntParam(key, defaultValue) == id);
}

/**
* @tc.name: PasteRecordTest004
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest004, TestSize.Level0)
{
    OHOS::Uri uri("uri");
    auto record = PasteboardClient::GetInstance()->CreateUriRecord(uri);
    ASSERT_TRUE(record != nullptr);
    auto newUri = record->GetUri();
    ASSERT_TRUE(newUri != nullptr);
    ASSERT_TRUE(newUri->ToString() == uri.ToString());
}

/**
* @tc.name: PasteRecordTest005
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
* @tc.require: AR000H5GKU
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest005, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMapIn);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    auto newPixelMap = pasteDataRecord->GetPixelMap();
    ASSERT_TRUE(newPixelMap != nullptr);
    ImageInfo imageInfo = {};
    newPixelMap->GetImageInfo(imageInfo);
    ASSERT_TRUE(imageInfo.size.height == opts.size.height);
    ASSERT_TRUE(imageInfo.size.width == opts.size.width);
    ASSERT_TRUE(imageInfo.pixelFormat == opts.pixelFormat);
}

/**
* @tc.name: PasteRecordTest006
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
* @tc.require: AR000H5GKU
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest006, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color)/sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMapIn);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    InitializationOptions opts1 = { { 6, 9 }, PixelFormat::RGB_565 };
    std::unique_ptr<PixelMap> pixelMap1 = PixelMap::Create(color, sizeof(color)/sizeof(color[0]), opts1);
    std::shared_ptr<PixelMap> pixelMapIn1 = move(pixelMap1);
    pasteDataRecord = pasteDataRecord->NewPixelMapRecord(pixelMapIn1);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    auto newPixelMap = pasteDataRecord->GetPixelMap();
    ASSERT_TRUE(newPixelMap != nullptr);
    ImageInfo imageInfo = {};
    newPixelMap->GetImageInfo(imageInfo);
    ASSERT_TRUE(imageInfo.size.height == opts1.size.height);
    ASSERT_TRUE(imageInfo.size.width == opts1.size.width);
    ASSERT_TRUE(imageInfo.pixelFormat == opts1.pixelFormat);
}

/**
* @tc.name: PasteRecordTest007
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
* @tc.require: AR000HEECD
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest007, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    auto customData = pasteDataRecord->GetCustomData();
    ASSERT_TRUE(customData != nullptr);
    auto itemData = customData->GetItemData();
    ASSERT_TRUE(itemData.size() == 1);
    auto item = itemData.find(mimeType);
    ASSERT_TRUE(item != itemData.end());
    ASSERT_TRUE(item->second == arrayBuffer);
}

/**
* @tc.name: PasteRecordTest008
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
* @tc.require: AR000HEECD
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest008, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    std::string mimeType1 = "img/png";
    std::vector<uint8_t> arrayBuffer1(46);
    arrayBuffer1 = { 2, 7, 6, 8, 9 };
    pasteDataRecord = pasteDataRecord->NewKvRecord(mimeType1, arrayBuffer1);
    auto customData = pasteDataRecord->GetCustomData();
    ASSERT_TRUE(customData != nullptr);
    auto itemData = customData->GetItemData();
    ASSERT_TRUE(itemData.size() == 1);
    auto item = itemData.find(mimeType1);
    ASSERT_TRUE(item != itemData.end());
    ASSERT_TRUE(item->second == arrayBuffer1);
}

/**
* @tc.name: PasteDataTest001
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest001, TestSize.Level0)
{
    using namespace OHOS::AAFwk;
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    auto data = PasteboardClient::GetInstance()->CreateWantData(std::make_shared<Want>(wantIn));
    ASSERT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*data);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto record = pasteData.GetPrimaryWant();
    ASSERT_TRUE(record != nullptr);
    int32_t defaultValue = 333;
    ASSERT_TRUE(record->GetIntParam(key, defaultValue) == id);
}

/**
* @tc.name: PasteDataTest002
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest002, TestSize.Level0)
{
    OHOS::Uri uri("uri");
    auto data = PasteboardClient::GetInstance()->CreateUriData(uri);
    ASSERT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*data);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto record = pasteData.GetPrimaryUri();
    ASSERT_TRUE(record != nullptr);
    ASSERT_TRUE(record->ToString() == uri.ToString());
}

/**
* @tc.name: PasteDataTest003
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest003, TestSize.Level0)
{
    std::string text = "plain text";
    auto data = PasteboardClient::GetInstance()->CreatePlainTextData(text);
    ASSERT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*data);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto record = pasteData.GetPrimaryText();
    ASSERT_TRUE(record != nullptr);
    ASSERT_TRUE(*record == text);
}

/**
* @tc.name: PasteDataTest004
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest004, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto data = PasteboardClient::GetInstance()->CreateHtmlData(htmlText);
    ASSERT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*data);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto record = pasteData.GetPrimaryHtml();
    ASSERT_TRUE(record != nullptr);
    ASSERT_TRUE(*record == htmlText);
}

/**
* @tc.name: PasteDataTest005
* @tc.desc: marshalling test.
* @tc.type: FUNC
* @tc.require: AR000HEECD
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest005, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(htmlText);
    ASSERT_TRUE(pasteData != nullptr);
    std::string plainText = "plain text";
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_HTML);
    std::string mimeType = MIMETYPE_TEXT_PLAIN;
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType1 = "image/jpg";
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    customData->AddItemData(mimeType1, arrayBuffer);
    std::shared_ptr<PasteDataRecord> pasteDataRecord = builder.SetMimeType(mimeType)
                                                           .SetPlainText(std::make_shared<std::string>(plainText))
                                                           .SetHtmlText(std::make_shared<std::string>(htmlText))
                                                           .SetCustomData(customData)
                                                           .Build();
    pasteData->AddRecord(pasteDataRecord);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto primaryHtml = newPasteData.GetPrimaryHtml();
    ASSERT_TRUE(primaryHtml != nullptr);
    ASSERT_TRUE(*primaryHtml == htmlText);
    auto firstRecord = newPasteData.GetRecordAt(0);
    ASSERT_TRUE(firstRecord != nullptr);
    ASSERT_TRUE(firstRecord->GetMimeType() == mimeType);
    auto newPlainText = firstRecord->GetPlainText();
    ASSERT_TRUE(newPlainText != nullptr);
    ASSERT_TRUE(*newPlainText == plainText);
    auto newHtmlText = firstRecord->GetHtmlText();
    ASSERT_TRUE(newHtmlText != nullptr);
    ASSERT_TRUE(*newHtmlText == htmlText);
    customData = pasteDataRecord->GetCustomData();
    ASSERT_TRUE(customData != nullptr);
    auto itemData = customData->GetItemData();
    ASSERT_TRUE(itemData.size() == 1);
    auto item = itemData.find(mimeType1);
    ASSERT_TRUE(item != itemData.end());
    ASSERT_TRUE(item->second == arrayBuffer);
}

/**
* @tc.name: PasteDataTest006
* @tc.desc: marshalling test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest006, TestSize.Level0)
{
    using namespace OHOS::AAFwk;
    std::string plainText = "helloWorld";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_WANT);
    std::shared_ptr<PasteDataRecord> pasteDataRecord =
        builder.SetWant(std::make_shared<Want>(wantIn)).SetPlainText(std::make_shared<std::string>(plainText)).Build();
    pasteData->AddRecord(pasteDataRecord);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto firstRecord = newPasteData.GetRecordAt(0);
    ASSERT_TRUE(firstRecord != nullptr);
    ASSERT_TRUE(firstRecord->GetMimeType() == MIMETYPE_TEXT_WANT);
    auto newWant = firstRecord->GetWant();
    ASSERT_TRUE(newWant != nullptr);
    int32_t defaultValue = 333;
    ASSERT_TRUE(newWant->GetIntParam(key, defaultValue) == id);
    auto newPlainText = firstRecord->GetPlainText();
    ASSERT_TRUE(newPlainText != nullptr);
    ASSERT_TRUE(*newPlainText == plainText);
}

/**
* @tc.name: PasteDataTest007
* @tc.desc: marshalling test.
* @tc.type: FUNC
* @tc.require: AR000H5GKU
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest007, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(htmlText);
    ASSERT_TRUE(pasteData != nullptr);
    OHOS::Uri uri("uri");
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color)/sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::shared_ptr<PasteDataRecord> pasteDataRecord =
        builder.SetUri(std::make_shared<OHOS::Uri>(uri)).SetPixelMap(pixelMapIn).Build();
    pasteData->AddRecord(pasteDataRecord);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto firstRecord = newPasteData.GetRecordAt(0);
    ASSERT_TRUE(firstRecord != nullptr);
    ASSERT_TRUE(firstRecord->GetMimeType() == MIMETYPE_TEXT_URI);
    auto newUri = firstRecord->GetUri();
    ASSERT_TRUE(newUri != nullptr);
    ASSERT_TRUE(newUri->ToString() == uri.ToString());
    auto newPixelMap = firstRecord->GetPixelMap();
    ASSERT_TRUE(newPixelMap != nullptr);
    ImageInfo imageInfo = {};
    newPixelMap->GetImageInfo(imageInfo);
    ASSERT_TRUE(imageInfo.size.height == opts.size.height);
    ASSERT_TRUE(imageInfo.size.width == opts.size.width);
    ASSERT_TRUE(imageInfo.pixelFormat == opts.pixelFormat);
}

/**
* @tc.name: PasteDataTest008
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
* @tc.require: AR000H5GKU
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest008, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color)/sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto pasteData = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMapIn);
    ASSERT_TRUE(pasteData != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto primaryPixelMap = newPasteData.GetPrimaryPixelMap();
    ASSERT_TRUE(primaryPixelMap != nullptr);
    ImageInfo imageInfo = {};
    primaryPixelMap->GetImageInfo(imageInfo);
    ASSERT_TRUE(imageInfo.size.height == opts.size.height);
    ASSERT_TRUE(imageInfo.size.width == opts.size.width);
    ASSERT_TRUE(imageInfo.pixelFormat == opts.pixelFormat);
}

/**
* @tc.name: PasteDataTest009
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
* @tc.require: AR000H5GKU
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest009, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color)/sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    pasteData->AddPixelMapRecord(pixelMapIn);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto primaryPlainText = newPasteData.GetPrimaryText();
    ASSERT_TRUE(primaryPlainText != nullptr);
    ASSERT_TRUE(*primaryPlainText == plainText);
    auto primaryPixelMap = newPasteData.GetPrimaryPixelMap();
    ASSERT_TRUE(primaryPixelMap != nullptr);
    ImageInfo imageInfo = {};
    primaryPixelMap->GetImageInfo(imageInfo);
    ASSERT_TRUE(imageInfo.size.height == opts.size.height);
    ASSERT_TRUE(imageInfo.size.width == opts.size.width);
    ASSERT_TRUE(imageInfo.pixelFormat == opts.pixelFormat);
}

/**
* @tc.name: PasteDataTest0010
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
* @tc.require: AR000HEECD
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest0010, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    auto pasteData = PasteboardClient::GetInstance()->CreateKvData(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteData != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto firstRecord = newPasteData.GetRecordAt(0);
    auto customData = firstRecord->GetCustomData();
    ASSERT_TRUE(customData != nullptr);
    auto itemData = customData->GetItemData();
    ASSERT_TRUE(itemData.size() == 1);
    auto item = itemData.find(mimeType);
    ASSERT_TRUE(item != itemData.end());
    ASSERT_TRUE(item->second == arrayBuffer);
}

/**
* @tc.name: PasteDataTest0011
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
* @tc.require: AR000HEECD
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest0011, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    pasteData->AddKvRecord(mimeType, arrayBuffer);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto firstRecord = newPasteData.GetRecordAt(0);
    auto customData = firstRecord->GetCustomData();
    ASSERT_TRUE(customData != nullptr);
    auto itemData = customData->GetItemData();
    ASSERT_TRUE(itemData.size() == 1);
    auto item = itemData.find(mimeType);
    ASSERT_TRUE(item != itemData.end());
    ASSERT_TRUE(item->second == arrayBuffer);
    auto primaryPlainText = newPasteData.GetPrimaryText();
    ASSERT_TRUE(primaryPlainText != nullptr);
    ASSERT_TRUE(*primaryPlainText == plainText);
}

/**
* @tc.name: PasteDataTest0012
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
* @tc.require: AR000HEECD
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest0012, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    pasteData->AddKvRecord(mimeType, arrayBuffer);
    auto record = pasteData->GetRecordAt(0);
    ASSERT_TRUE(record != nullptr);
    std::string mimeType1 = "img/png";
    std::vector<uint8_t> arrayBuffer1(54);
    arrayBuffer1 = { 4, 7, 9, 8, 7 };
    auto customData = record->GetCustomData();
    ASSERT_TRUE(customData != nullptr);
    customData->AddItemData(mimeType1, arrayBuffer1);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto firstRecord = newPasteData.GetRecordAt(0);
    ASSERT_TRUE(firstRecord != nullptr);
    customData = firstRecord->GetCustomData();
    ASSERT_TRUE(customData != nullptr);
    auto itemData = customData->GetItemData();
    ASSERT_TRUE(itemData.size() == 2);
    auto item = itemData.find(mimeType);
    ASSERT_TRUE(item != itemData.end());
    ASSERT_TRUE(item->second == arrayBuffer);
    item = itemData.find(mimeType1);
    ASSERT_TRUE(item != itemData.end());
    ASSERT_TRUE(item->second == arrayBuffer1);
    auto primaryPlainText = newPasteData.GetPrimaryText();
    ASSERT_TRUE(primaryPlainText != nullptr);
    ASSERT_TRUE(*primaryPlainText == plainText);
    auto secondRecord = newPasteData.GetRecordAt(1);
    ASSERT_TRUE(secondRecord != nullptr);
    auto secondRecordMimeType = secondRecord->GetMimeType();
    ASSERT_TRUE(secondRecordMimeType == MIMETYPE_TEXT_PLAIN);
}

/**
* @tc.name: PasteDataTest0013
* @tc.desc: MineCustomData: Marshalling unMarshalling
* @tc.type: FUNC
* @tc.require: AR000HEECD
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest0013, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    pasteData->AddKvRecord(mimeType, arrayBuffer);
    auto record = pasteData->GetRecordAt(0);
    ASSERT_TRUE(record != nullptr);
    std::string mimeType1 = "img/png";
    std::vector<uint8_t> arrayBuffer1(54);
    arrayBuffer1 = { 4, 7, 9, 8, 7 };
    auto customData = record->GetCustomData();
    ASSERT_TRUE(customData != nullptr);
    customData->AddItemData(mimeType1, arrayBuffer1);
    Parcel parcel;
    auto ret = customData->Marshalling(parcel);
    ASSERT_TRUE(ret == true);
    std::shared_ptr<MineCustomData> customDataGet(customData->Unmarshalling(parcel));
    ASSERT_TRUE(customDataGet != nullptr);
    auto itemData = customDataGet->GetItemData();
    ASSERT_TRUE(itemData.size() == 2);
    auto item = itemData.find(mimeType);
    ASSERT_TRUE(item != itemData.end());
    ASSERT_TRUE(item->second == arrayBuffer);
    item = itemData.find(mimeType1);
    ASSERT_TRUE(item != itemData.end());
    ASSERT_TRUE(item->second == arrayBuffer1);
}

/**
* @tc.name: PasteDataTest0014
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
* @tc.require: AROOOH5R5G
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest0014, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    auto shareOption = pasteData->GetShareOption();
    ASSERT_TRUE(shareOption == ShareOption::CrossDevice);
    pasteData->SetShareOption(ShareOption::InApp);
    auto tokenId = pasteData->GetTokenId();
    ASSERT_TRUE(tokenId == 0);
    pasteData->SetTokenId(1);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    shareOption = newPasteData.GetShareOption();
    ASSERT_TRUE(shareOption == ShareOption::InApp);
    tokenId = pasteData->GetTokenId();
    ASSERT_TRUE(tokenId != 0);
}

/**
* @tc.name: PasteDataTest0015
* @tc.desc: isLocalPaste and isDraggedData test.
* @tc.type: FUNC
* @tc.require: AROOOH5R5G
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest0015, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    auto isDraggedData = pasteData->IsDraggedData();
    ASSERT_FALSE(isDraggedData);
    pasteData->SetDraggedDataFlag(true);
    auto isLocalPaste = pasteData->IsLocalPaste();
    ASSERT_FALSE(isLocalPaste);
    pasteData->SetLocalPasteFlag(true);
    isLocalPaste = pasteData->IsLocalPaste();
    ASSERT_TRUE(isLocalPaste);
    pasteData->SetLocalPasteFlag(false);
    isLocalPaste = pasteData->IsLocalPaste();
    ASSERT_FALSE(isLocalPaste);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_FALSE(hasPasteData);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    isDraggedData = newPasteData.IsDraggedData();
    ASSERT_TRUE(isDraggedData);
    isLocalPaste = newPasteData.IsLocalPaste();
    ASSERT_TRUE(isLocalPaste);
}

/**
* @tc.name: PasteDataTest0016
* @tc.desc: RemoveRecordAt HasMimeType test.
* @tc.type: FUNC
* @tc.require: AROOOH5R5G
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest0016, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    pasteData->AddKvRecord(mimeType, arrayBuffer);
    pasteData->RemoveRecordAt(1);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_FALSE(hasPasteData);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    ASSERT_TRUE(newPasteData.HasMimeType(mimeType));
    ASSERT_TRUE(newPasteData.GetRecordCount() == 1);
    auto record = newPasteData.GetRecordAt(0);
    ASSERT_TRUE(record != nullptr);
    auto customData = record->GetCustomData();
    ASSERT_TRUE(customData != nullptr);
    auto itemData = customData->GetItemData();
    ASSERT_EQ(itemData.size(), 1);
    auto item = itemData.find(mimeType);
    ASSERT_TRUE(item != itemData.end());
    EXPECT_EQ(item->second, arrayBuffer);
}

/**
* @tc.name: PasteDataTest0017
* @tc.desc: ReplaceRecordAt GetProperty GetTag test.
* @tc.type: FUNC
* @tc.require: AROOOH5R5G
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest0017, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color)/sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto pasteData = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMapIn);
    ASSERT_TRUE(pasteData != nullptr);
    auto pixelMap1 = pasteData->GetPrimaryPixelMap();
    ASSERT_TRUE(pixelMap1 != nullptr);
    ImageInfo imageInfo = {};
    pixelMap1->GetImageInfo(imageInfo);
    ASSERT_EQ(imageInfo.size.height, opts.size.height);
    ASSERT_EQ(imageInfo.size.width, opts.size.width);
    ASSERT_EQ(imageInfo.pixelFormat, opts.pixelFormat);
    std::string plainText = "plain text";
    auto record = PasteboardClient::GetInstance()->CreatePlainTextRecord(plainText);
    ASSERT_TRUE(record != nullptr);
    ASSERT_TRUE(pasteData->ReplaceRecordAt(0, record));
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_FALSE(hasPasteData);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    ASSERT_EQ(newPasteData.GetRecordCount(), 1);
    auto record1 = newPasteData.GetRecordAt(0);
    ASSERT_TRUE(record1 != nullptr);
    auto plainText1 = record1->GetPlainText();
    ASSERT_TRUE(plainText1 != nullptr);
    EXPECT_EQ(*plainText1, plainText);
    auto property = newPasteData.GetProperty();
    EXPECT_TRUE(property.additions.IsEmpty());
    EXPECT_EQ(property.mimeTypes.size(), 1);
    EXPECT_EQ(property.mimeTypes[0], MIMETYPE_TEXT_PLAIN);
    EXPECT_TRUE(property.tag.empty());
    EXPECT_EQ(property.shareOption, ShareOption::CrossDevice);
    EXPECT_TRUE(property.tokenId != 0);
    auto tag = newPasteData.GetTag();
    EXPECT_TRUE(tag.empty());
}

/**
* @tc.name: PasteDataTest0018
* @tc.desc: AddPasteboardChangedObserver RemovePasteboardChangedObserver OnRemoteDied OnRemoteSaDied test.
* @tc.type: FUNC
* @tc.require: AROOOH5R5G
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest0018, TestSize.Level0)
{
    if (PasteboardServiceTest::pasteboardObserver_ == nullptr) {
        PasteboardServiceTest::pasteboardObserver_ = new PasteboardObserverCallback();
    }
    ASSERT_TRUE(PasteboardServiceTest::pasteboardObserver_ != nullptr);
    PasteboardClient::GetInstance()->AddPasteboardChangedObserver(PasteboardServiceTest::pasteboardObserver_);
    ASSERT_FALSE(PasteboardServiceTest::pasteboardChangedFlag_);
    const wptr<IRemoteObject> object;
    PasteboardSaDeathRecipient death;
    death.OnRemoteDied(object);
    PasteboardClient::GetInstance()->OnRemoteSaDied(object);
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color)/sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto pasteData = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMapIn);
    ASSERT_TRUE(pasteData != nullptr);
    PasteboardClient::GetInstance()->Clear();
    ASSERT_FALSE(PasteboardServiceTest::pasteboardChangedFlag_);
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_FALSE(hasPasteData);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    ASSERT_TRUE(PasteboardServiceTest::pasteboardChangedFlag_);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    PasteboardServiceTest::pasteboardChangedFlag_ = false;
    PasteboardClient::GetInstance()->RemovePasteboardChangedObserver(PasteboardServiceTest::pasteboardObserver_);
    PasteboardClient::GetInstance()->Clear();
    ASSERT_FALSE(PasteboardServiceTest::pasteboardChangedFlag_);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_FALSE(hasPasteData);
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_FALSE(PasteboardServiceTest::pasteboardChangedFlag_);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData);
}


/**
 * @tc.name: PasteDataTest0019
 * @tc.desc: AddPasteboardEventObserver RemovePasteboardEventObserver OnRemoteDied OnRemoteSaDied test.
 * @tc.type: FUNC
 * @tc.require: AROOOH5R5G
 */
HWTEST_F(PasteboardServiceTest, PasteDataTest0019, TestSize.Level0)
{
    if (PasteboardServiceTest::pasteboardEventObserver_ == nullptr) {
        PasteboardServiceTest::pasteboardEventObserver_ = new PasteboardEventObserverCallback();
    }
    ASSERT_TRUE(PasteboardServiceTest::pasteboardEventObserver_ != nullptr);
    PasteboardClient::GetInstance()->AddPasteboardEventObserver(PasteboardServiceTest::pasteboardEventObserver_);
    ASSERT_FALSE(PasteboardServiceTest::pasteboardChangedFlag_);
    ASSERT_EQ(PasteboardServiceTest::pasteboardEventStatus_, -1);
    const wptr<IRemoteObject> object;
    PasteboardSaDeathRecipient death;
    death.OnRemoteDied(object);
    PasteboardClient::GetInstance()->OnRemoteSaDied(object);
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto pasteData = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMapIn);
    ASSERT_TRUE(pasteData != nullptr);
    PasteboardClient::GetInstance()->Clear();
    ASSERT_FALSE(PasteboardServiceTest::pasteboardChangedFlag_);
    ASSERT_EQ(PasteboardServiceTest::pasteboardEventStatus_, -1);
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_FALSE(hasPasteData);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    ASSERT_FALSE(PasteboardServiceTest::pasteboardChangedFlag_);
    ASSERT_EQ(PasteboardServiceTest::pasteboardEventStatus_,
        static_cast<int32_t>(PasteboardEventStatus::PASTEBOARD_WRITE));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData);
    PasteData newPasteData;
    ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    ASSERT_EQ(PasteboardServiceTest::pasteboardEventStatus_,
        static_cast<int32_t>(PasteboardEventStatus::PASTEBOARD_READ));
    PasteboardClient::GetInstance()->Clear();
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    ASSERT_EQ(PasteboardServiceTest::pasteboardEventStatus_,
        static_cast<int32_t>(PasteboardEventStatus::PASTEBOARD_CLEAR));
    PasteboardServiceTest::pasteboardEventStatus_ = -1;
    PasteboardClient::GetInstance()->RemovePasteboardEventObserver(PasteboardServiceTest::pasteboardEventObserver_);
    PasteboardClient::GetInstance()->Clear();
    ASSERT_FALSE(PasteboardServiceTest::pasteboardChangedFlag_);
    ASSERT_EQ(PasteboardServiceTest::pasteboardEventStatus_, -1);
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    ASSERT_FALSE(PasteboardServiceTest::pasteboardChangedFlag_);
    ASSERT_EQ(PasteboardServiceTest::pasteboardEventStatus_, -1);
}

/**
* @tc.name: BigPixelMap001
* @tc.desc: paste big pixel map image
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: baoyayong
*/
HWTEST_F(PasteboardServiceTest, BigPixelMap001, TestSize.Level1)
{
    constexpr uint32_t COLOR_SIZE = 1024 * 1960;
    auto color = std::make_unique<uint32_t[]>(COLOR_SIZE);
    InitializationOptions opts = { { 1024, 1960 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color.get(), COLOR_SIZE, opts);

    auto pasteData1 = PasteboardClient::GetInstance()->CreatePixelMapData(std::move(pixelMap));
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_FALSE(hasPasteData);
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData1);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData);

    PasteData pasteData2;
    ret = PasteboardClient::GetInstance()->GetPasteData(pasteData2);
    ASSERT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));

    auto pixelMap2 = pasteData2.GetPrimaryPixelMap();
    ASSERT_TRUE(pixelMap2 != nullptr);
    ImageInfo imageInfo{};
    pixelMap2->GetImageInfo(imageInfo);
    EXPECT_TRUE(imageInfo.size.height == opts.size.height);
    EXPECT_TRUE(imageInfo.size.width == opts.size.width);
    EXPECT_TRUE(imageInfo.pixelFormat == opts.pixelFormat);
}

/**
* @tc.name: GetPastedataFail001
* @tc.desc: get paste data fail - SetValue()
* @tc.type: FUNC
* @tc.require: issuesI5WPTM
* @tc.author: chenyu
*/
HWTEST_F(PasteboardServiceTest, GetPastedataFail001, TestSize.Level1)
{
    PasteboardClient::GetInstance()->Clear();
    PasteData data;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(data);
    EXPECT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_ERROR));
}

/**
* @tc.name: DumpDataTest001
* @tc.desc: DumpData()-remote, CrossDevice
* @tc.type: FUNC
* @tc.require: issueshI5YDEV
* @tc.author: chenyu
*/
HWTEST_F(PasteboardServiceTest, DumpDataTest001, TestSize.Level1)
{
    PasteboardServiceTest::SetTestTokenId();
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    pasteData->SetRemote(true);
    pasteData->SetShareOption(ShareOption::CrossDevice);
    PasteboardClient::GetInstance()->Clear();
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    EXPECT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    std::string result;
    ret = PasteboardServiceTest::ExecuteCmd(result);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(result.find("CrossDevice") != std::string::npos);
    EXPECT_TRUE(result.find("remote") != std::string::npos);
    PasteboardClient::GetInstance()->Clear();
    PasteboardServiceTest::RestoreSelfTokenId();
}

/**
* @tc.name: DumpDataTest002
* @tc.desc: DumpData()-local, LocalDevice
* @tc.type: FUNC
* @tc.require: issueshI5YDEV
* @tc.author: chenyu
*/
HWTEST_F(PasteboardServiceTest, DumpDataTest002, TestSize.Level1)
{
    PasteboardServiceTest::SetTestTokenId();
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    pasteData->SetShareOption(ShareOption::LocalDevice);
    PasteboardClient::GetInstance()->Clear();
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    EXPECT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    std::string result;
    ret = PasteboardServiceTest::ExecuteCmd(result);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(result.find("LocalDevice") != std::string::npos);
    EXPECT_TRUE(result.find("local") != std::string::npos);
    PasteboardClient::GetInstance()->Clear();
    PasteboardServiceTest::RestoreSelfTokenId();
}

/**
* @tc.name: DumpDataTest003
* @tc.desc: DumpData()-local, InApp
* @tc.type: FUNC
* @tc.require: issueshI5YDEV
* @tc.author: chenyu
*/
HWTEST_F(PasteboardServiceTest, DumpDataTest003, TestSize.Level1)
{
    PasteboardServiceTest::SetTestTokenId();
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    pasteData->SetShareOption(ShareOption::InApp);
    PasteboardClient::GetInstance()->Clear();
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    EXPECT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    std::string result;
    ret = PasteboardServiceTest::ExecuteCmd(result);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(result.find("InAPP") != std::string::npos);
    EXPECT_TRUE(result.find("local") != std::string::npos);
    PasteboardClient::GetInstance()->Clear();
    PasteboardServiceTest::RestoreSelfTokenId();
}

/**
* @tc.name: DumpDataTest004
* @tc.desc: DumpData()-no data
* @tc.type: FUNC
* @tc.require: issueshI5YDEV
* @tc.author: chenyu
*/
HWTEST_F(PasteboardServiceTest, DumpDataTest004, TestSize.Level1)
{
    PasteboardServiceTest::SetTestTokenId();
    PasteboardClient::GetInstance()->Clear();

    std::string result;
    auto ret = PasteboardServiceTest::ExecuteCmd(result);
    EXPECT_TRUE(ret);
    EXPECT_EQ(result.find("Share"), std::string::npos);
    EXPECT_EQ(result.find("Option"), std::string::npos);
    PasteboardServiceTest::RestoreSelfTokenId();
}

/**
* @tc.name: HasPastePermissionTest001
* @tc.desc: if (!pasteData->IsDraggedData() && (!isFocusedApp && !IsDefaultIME(GetAppInfo(tokenId))))
* @tc.type: FUNC
* @tc.require: issueshI5YDEV
* @tc.author: chenyu
*/
HWTEST_F(PasteboardServiceTest, HasPastePermissionTest001, TestSize.Level0)
{
    PasteboardServiceTest::SetTestTokenId();
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    auto pasteData = PasteboardClient::GetInstance()->CreateKvData(mimeType, arrayBuffer);
    PasteboardClient::GetInstance()->Clear();
    int32_t ret = PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    EXPECT_TRUE(ret == static_cast<int32_t>(PasteboardError::E_OK));
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    // not DraggedData, not DefaultIME, not FocusedApp
    EXPECT_FALSE(hasPasteData);
    PasteboardClient::GetInstance()->Clear();
    PasteboardServiceTest::RestoreSelfTokenId();
}
} // namespace OHOS::MiscServices