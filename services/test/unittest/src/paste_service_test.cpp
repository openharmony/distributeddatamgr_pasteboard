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
{}

void PasteboardServiceTest::TearDownTestCase(void)
{}

void PasteboardServiceTest::SetUp(void)
{}

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
    EXPECT_TRUE(record != nullptr);

    auto data = PasteboardClient::GetInstance()->CreatePlainTextData("paste data1");
    EXPECT_TRUE(data != nullptr);

    PasteboardClient::GetInstance()->SetPasteData(*data);
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get.");
    EXPECT_TRUE(ok == true);
    auto textPtr = pasteData.GetPrimaryText();
}

/**
* @tc.name: PasteRecordTest001
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest001, TestSize.Level0)
{
    auto record = PasteboardClient::GetInstance()->CreatePlainTextRecord("plain text");
    EXPECT_TRUE(record != nullptr);
}

/**
* @tc.name: PasteRecordTest002
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest002, TestSize.Level0)
{
    auto record = PasteboardClient::GetInstance()->CreateHtmlTextRecord("html record");
    EXPECT_TRUE(record != nullptr);
}

/**
* @tc.name: PasteRecordTest003
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest003, TestSize.Level0)
{
    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
    auto record = PasteboardClient::GetInstance()->CreateWantRecord(want);
    EXPECT_TRUE(record != nullptr);
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
    EXPECT_TRUE(record != nullptr);
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
    EXPECT_TRUE(data != nullptr);
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has != true);
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    EXPECT_TRUE(ok == true);
    auto record = pasteData.GetPrimaryWant();
    EXPECT_TRUE(record != nullptr);
    if (record != nullptr) {
        int32_t defaultValue = 333;
        EXPECT_TRUE(record->GetIntParam(key, defaultValue) == id);
    }
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
    EXPECT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has != true);
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    EXPECT_TRUE(ok == true);
    auto record = pasteData.GetPrimaryUri();
    EXPECT_TRUE(record != nullptr);
    if (record != nullptr) {
        EXPECT_TRUE(record->ToString() == uri.ToString());
    }
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
    EXPECT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has != true);
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    EXPECT_TRUE(ok == true);
    auto record = pasteData.GetPrimaryText();
    EXPECT_TRUE(record != nullptr);
    if (record != nullptr) {
        EXPECT_TRUE(*record == text);
    }
}

/**
* @tc.name: PasteDataTest004
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest004, TestSize.Level0)
{
    std::string html = "<div class='disabled item tip user-programs'>";
    auto data = PasteboardClient::GetInstance()->CreateHtmlData(html);
    EXPECT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has != true);
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    EXPECT_TRUE(ok == true);
    auto record = pasteData.GetPrimaryHtml();
    EXPECT_TRUE(record != nullptr);
    if (record != nullptr) {
        EXPECT_TRUE(*record == html);
    }
}

/**
* @tc.name: PasteDataTest005
* @tc.desc: marshalling test.
* @tc.type: FUNC
*/
void MarshallingCheckPasteData(PasteData &pasteData, InitializationOptions &opts, std::string &key, int32_t id)
{
    auto primaryHtml = pasteData.GetPrimaryHtml();
    EXPECT_TRUE(primaryHtml != nullptr);
    if (primaryHtml != nullptr) {
        EXPECT_TRUE(*primaryHtml == "<div class='disabled item tip user-programs'>");
    }
    auto firstRecord = pasteData.GetRecordAt(0);
    EXPECT_TRUE(firstRecord != nullptr);
    if (firstRecord != nullptr) {
        EXPECT_TRUE(firstRecord->GetMimeType() == MIMETYPE_TEXT_URI);
        auto getPlainText = firstRecord->GetPlainText();
        EXPECT_TRUE(getPlainText != nullptr);
        if (getPlainText != nullptr) {
            EXPECT_TRUE(*getPlainText == "plain text");
        }
        auto getUri = firstRecord->GetUri();
        EXPECT_TRUE(getUri != nullptr);
        if (getUri != nullptr) {
            EXPECT_TRUE(getUri->ToString() == "uri");
        }
        auto getPixelMap = firstRecord->GetPixelMap();
        EXPECT_TRUE(getPixelMap != nullptr);
        if (getPixelMap != nullptr) {
            ImageInfo imageInfo = {};
            getPixelMap->GetImageInfo(imageInfo);
            EXPECT_TRUE(imageInfo.size.height == opts.size.height);
            EXPECT_TRUE(imageInfo.size.width == opts.size.width);
            EXPECT_TRUE(imageInfo.pixelFormat == opts.pixelFormat);
        }
        auto getHtmlText = firstRecord->GetHtmlText();
        EXPECT_TRUE(getHtmlText != nullptr);
        if (getHtmlText != nullptr) {
            EXPECT_TRUE(*getHtmlText == "<div class='disabled item tip user-programs'>");
        }
        auto getWant = firstRecord->GetWant();
        EXPECT_TRUE(getWant != nullptr);
        if (getWant != nullptr) {
            int32_t defaultValue = 333;
            EXPECT_TRUE(getWant->GetIntParam(key, defaultValue) == id);
        }
    }
}
HWTEST_F(PasteboardServiceTest, PasteDataTest005, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(htmlText);
    EXPECT_TRUE(pasteData != nullptr);
    if (pasteData != nullptr) {
        std::string plainText = "plain text";
        OHOS::Uri uri("uri");
        uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
        InitializationOptions opts = {};
        opts.size.height = 4;
        opts.size.width = 7;
        opts.pixelFormat = static_cast<PixelFormat>(AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
        std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
        std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
        std::shared_ptr<Want> want = std::make_shared<Want>();
        std::string key = "id";
        int32_t id = 456;
        Want wantIn = want->SetParam(key, id);
        PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
        std::shared_ptr<PasteDataRecord> pasteDataRecord =
            builder.SetPlainText(std::make_shared<std::string>(plainText))
                .SetUri(std::make_shared<OHOS::Uri>(uri))
                .SetPixelMap(pixelMapIn)
                .SetHtmlText(std::make_shared<std::string>(htmlText))
                .SetWant(std::make_shared<Want>(wantIn))
                .Build();
        pasteData->AddRecord(pasteDataRecord);
        PasteboardClient::GetInstance()->Clear();
        auto hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
        EXPECT_TRUE(hasPasteData != true);
        PasteboardClient::GetInstance()->SetPasteData(*pasteData);
        hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
        EXPECT_TRUE(hasPasteData == true);
        PasteData getPasteData;
        auto ret = PasteboardClient::GetInstance()->GetPasteData(getPasteData);
        EXPECT_TRUE(ret == true);
        MarshallingCheckPasteData(getPasteData, opts, key, id);
    }
}
}
