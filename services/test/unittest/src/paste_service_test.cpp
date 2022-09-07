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

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
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
    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
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
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMapIn);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    InitializationOptions opts1 = { { 6, 9 }, PixelFormat::RGB_565 };
    std::unique_ptr<PixelMap> pixelMap1 = PixelMap::Create(color, 100, opts1);
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
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    auto data = PasteboardClient::GetInstance()->CreateWantData(std::make_shared<Want>(wantIn));
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
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == true);
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
    PasteData newPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == true);
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
    PasteData newPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == true);
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
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto pasteData = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMapIn);
    ASSERT_TRUE(pasteData != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == true);
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
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    pasteData->AddPixelMapRecord(pixelMapIn);
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == true);
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
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == true);
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
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == true);
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
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == true);
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
* @tc.desc: Create paste board data test.
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
    std::string appId = pasteData->GetAppId();
    ASSERT_TRUE(appId.empty() == true);
    pasteData->SetAppId("abc");
    PasteboardClient::GetInstance()->Clear();
    auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData != true);
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
    ASSERT_TRUE(hasPasteData == true);
    PasteData newPasteData;
    auto ret = PasteboardClient::GetInstance()->GetPasteData(newPasteData);
    ASSERT_TRUE(ret == true);
    shareOption = newPasteData.GetShareOption();
    ASSERT_TRUE(shareOption == ShareOption::InApp);
}
} // namespace OHOS::MiscServices