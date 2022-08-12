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

#include <cstdint>
#include <vector>

#include "pasteboard_client.h"
#include "pasteboard_observer_callback.h"
#include "pixel_map.h"
#include "uri.h"
#include "want.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::MiscServices;
using namespace OHOS::Media;

class PasteboardServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardServiceTest::SetUpTestCase(void)
{
}

void PasteboardServiceTest::TearDownTestCase(void)
{
}

void PasteboardServiceTest::SetUp(void)
{
}

void PasteboardServiceTest::TearDown(void)
{
    PasteboardClient::GetInstance()->Clear();
}

void PasteboardObserverCallback::OnPasteboardChanged()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "callback.");
}

namespace {
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
    PasteboardClient::GetInstance()->SetPasteData(*data);
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get.");
    ASSERT_TRUE(ok == true);
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
    auto getPlainText = record->GetPlainText();
    ASSERT_TRUE(getPlainText != nullptr);
    ASSERT_TRUE(*getPlainText == plainText);
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
    auto getHtmlText = record->GetHtmlText();
    ASSERT_TRUE(getHtmlText != nullptr);
    ASSERT_TRUE(*getHtmlText == htmlText);
}

/**
* @tc.name: PasteRecordTest003
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest003, TestSize.Level0)
{
    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    auto record = PasteboardClient::GetInstance()->CreateWantRecord(want);
    ASSERT_TRUE(record != nullptr);
    auto getWant = record->GetWant();
    ASSERT_TRUE(getWant != nullptr);
    int32_t defaultValue = 333;
    ASSERT_TRUE(getWant->GetIntParam(key, defaultValue) == id);
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
    auto getUri = record->GetUri();
    ASSERT_TRUE(getUri != nullptr);
    ASSERT_TRUE(getUri->ToString() == uri.ToString());
}

/**
* @tc.name: PasteDataTest001
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest001, TestSize.Level0)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    auto data = PasteboardClient::GetInstance()->CreateWantData(std::make_shared<Want>(wantIn));
    ASSERT_TRUE(data != nullptr);
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has != true);
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ok == true);
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
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ok == true);
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
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ok == true);
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
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_TRUE(ok == true);
    auto record = pasteData.GetPrimaryHtml();
    ASSERT_TRUE(record != nullptr);
    ASSERT_TRUE(*record == htmlText);
}

/**
* @tc.name: PasteDataTest005
* @tc.desc: marshalling test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest005, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(htmlText);
    ASSERT_TRUE(pasteData != nullptr);
    std::string plainText = "plain text";
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_PLAIN);
    std::shared_ptr<PasteDataRecord> pasteDataRecord = builder.SetPlainText(std::make_shared<std::string>(plainText))
                                                           .SetHtmlText(std::make_shared<std::string>(htmlText))
                                                           .Build();
    pasteData->AddRecord(pasteDataRecord);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData getPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(getPasteData);
    ASSERT_TRUE(ret == true);
    auto primaryHtml = getPasteData.GetPrimaryHtml();
    ASSERT_TRUE(primaryHtml != nullptr);
    ASSERT_TRUE(*primaryHtml == htmlText);
    auto firstRecord = getPasteData.GetRecordAt(0);
    ASSERT_TRUE(firstRecord != nullptr);
    ASSERT_TRUE(firstRecord->GetMimeType() == MIMETYPE_TEXT_PLAIN);
    auto getPlainText = firstRecord->GetPlainText();
    ASSERT_TRUE(getPlainText != nullptr);
    ASSERT_TRUE(*getPlainText == plainText);
    auto getHtmlText = firstRecord->GetHtmlText();
    ASSERT_TRUE(getHtmlText != nullptr);
    ASSERT_TRUE(*getHtmlText == htmlText);
}

/**
* @tc.name: PasteDataTest006
* @tc.desc: marshalling test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest006, TestSize.Level0)
{
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
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData getPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(getPasteData);
    ASSERT_TRUE(ret == true);
    auto firstRecord = getPasteData.GetRecordAt(0);
    ASSERT_TRUE(firstRecord != nullptr);
    ASSERT_TRUE(firstRecord->GetMimeType() == MIMETYPE_TEXT_WANT);
    auto getWant = firstRecord->GetWant();
    ASSERT_TRUE(getWant != nullptr);
    int32_t defaultValue = 333;
    ASSERT_TRUE(getWant->GetIntParam(key, defaultValue) == id);
    auto getPlainText = firstRecord->GetPlainText();
    ASSERT_TRUE(getPlainText != nullptr);
    ASSERT_TRUE(*getPlainText == plainText);
}

/**
* @tc.name: PasteDataTest007
* @tc.desc: marshalling test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest007, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(htmlText);
    ASSERT_TRUE(pasteData != nullptr);
    OHOS::Uri uri("uri");
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = {};
    opts.size.height = 4;
    opts.size.width = 7;
    opts.pixelFormat = static_cast<PixelFormat>(AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::shared_ptr<PasteDataRecord> pasteDataRecord =
        builder.SetUri(std::make_shared<OHOS::Uri>(uri)).SetPixelMap(pixelMapIn).Build();
    pasteData->AddRecord(pasteDataRecord);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData getPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(getPasteData);
    ASSERT_TRUE(ret == true);
    auto firstRecord = getPasteData.GetRecordAt(0);
    ASSERT_TRUE(firstRecord != nullptr);
    ASSERT_TRUE(firstRecord->GetMimeType() == MIMETYPE_TEXT_URI);
    auto getUri = firstRecord->GetUri();
    ASSERT_TRUE(getUri != nullptr);
    ASSERT_TRUE(getUri->ToString() == uri.ToString());
    auto getPixelMap = firstRecord->GetPixelMap();
    ASSERT_TRUE(getPixelMap != nullptr);
    ImageInfo imageInfo = {};
    getPixelMap->GetImageInfo(imageInfo);
    ASSERT_TRUE(imageInfo.size.height == opts.size.height);
    ASSERT_TRUE(imageInfo.size.width == opts.size.width);
    ASSERT_TRUE(imageInfo.pixelFormat == opts.pixelFormat);
}
} // namespace