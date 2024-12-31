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

#include "pasteboard_web_controller.h"
#include "pasteboard_client.h"
#include <gtest/gtest.h>

using namespace testing;
using namespace testing::ext;
using namespace OHOS::MiscServices;
class WebControllerTest : public testing::Test {
public:
    WebControllerTest() {};
    ~WebControllerTest() {};
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void WebControllerTest::SetUpTestCase(void)
{
}

void WebControllerTest::TearDownTestCase(void)
{
}

void WebControllerTest::SetUp(void)
{
}

void WebControllerTest::TearDown(void)
{
}

/**
 * @tc.name: SplitHtmlTest_001.
 * @tc.desc: Test did not use local image address HTML with SplitHtml.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, SplitHtmlTest_001, TestSize.Level1)
{
    auto webClipboardController = PasteboardWebController::GetInstance();
    std::shared_ptr<std::string> html(
        new std::string("<img data-ohos='clipboard' src='http://file1.jpg'><img data-ohos='clipboard' "
                        "src='https://data/storage/el2/distributedfiles/202305301.png'>"));
    auto records = webClipboardController.SplitHtml2Records(html, 1);
    EXPECT_EQ(records.size(), 0);
}

/**
 * @tc.name: SplitHtmlTest_002.
 * @tc.desc: Test contains a local image address HTML with SplitHtml.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, SplitHtmlTest_002, TestSize.Level1)
{
    auto webClipboardController = PasteboardWebController::GetInstance();
    std::shared_ptr<std::string> html(
        new std::string("<img data-ohos='clipboard' src='file:///file1.jpg'><img data-ohos='clipboard' "
                        "src='https://data/storage/el2/distributedfiles/202305301.png'>"));
    auto records = webClipboardController.SplitHtml2Records(html, 1);
    EXPECT_EQ(records.size(), 1);
}

/**
 * @tc.name: SplitHtmlTest_003.
 * @tc.desc: Test contains multiple image addresses, but no local address with SplitHtml.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, SplitHtmlTest_003, TestSize.Level1)
{
    auto webClipboardController = PasteboardWebController::GetInstance();
    std::shared_ptr<std::string> html(
        new std::string("<img data-ohos='clipboard' src='file://file1.jpg'><img data-ohos='clipboard' "
                        "src='file2.jpg'><img data-ohos='clipboard' "
                        "src='https://data/storage/el2/distributedfiles/202305301.png'>"));
    auto records = webClipboardController.SplitHtml2Records(html, 1);
    EXPECT_FALSE(records.empty());
}

/**
 * @tc.name: RebuildHtmlTest_004.
 * @tc.desc: Test does not include local image address HTML with RebuildHtml.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, RebuildHtmlTest_004, TestSize.Level1)
{
    std::shared_ptr<std::string> html(
            new std::string("<img data-ohos='clipboard' src='http://file1.jpg'><img data-ohos='clipboard' "
                            "src='https://data/storage/el2/distributedfiles/202305301.png'>"));
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(*html);
    auto htmlRecord = pasteData->GetRecordAt(0);

    auto webClipboardController = PasteboardWebController::GetInstance();
    auto records = webClipboardController.SplitHtml2Records(html, htmlRecord->GetRecordId());
    EXPECT_EQ(records.size(), 0);

    webClipboardController.MergeExtraUris2Html(*pasteData);
    EXPECT_EQ(pasteData->GetRecordCount(), 1);
}

/**
 * @tc.name: RebuildHtmlTest_005.
 * @tc.desc: Test contains a local image address HTML with RebuildHtml.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, RebuildHtmlTest_005, TestSize.Level1)
{
    const int32_t splitRecordCount = 1;
    const std::string uri = "file:///data/storage/el2/distributedfiles/temp.png";
    auto webClipboardController = PasteboardWebController::GetInstance();
    std::shared_ptr<std::string> html(
        new std::string("<img data-ohos='clipboard' src='file:///file1.jpg'><img data-ohos='clipboard' "
                        "src='https://data/storage/el2/distributedfiles/202305301.png'>"));
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(*html);
    auto htmlRecord = pasteData->GetRecordAt(0);
    htmlRecord->SetFrom(htmlRecord->GetRecordId());

    const char* execptHtml =
        "<img data-ohos='clipboard' src='file:///data/storage/el2/distributedfiles/temp.png'><img "
        "data-ohos='clipboard' "
        "src='https://data/storage/el2/distributedfiles/202305301.png'>";
    auto records = webClipboardController.SplitHtml2Records(html, htmlRecord->GetRecordId());
    EXPECT_EQ(records.size(), splitRecordCount);
    pasteData->AddRecord(records[0]);

    std::shared_ptr<PasteData> newPasteData = std::make_shared<PasteData>();
    std::vector<std::shared_ptr<PasteDataRecord>> pasteDataRecords = pasteData->AllRecords();
    EXPECT_EQ(*(pasteDataRecords[pasteData->GetRecordCount() - 1]->GetHtmlText()), *html);

    newPasteData->AddHtmlRecord(*html);
    for (auto i = 0; i < pasteData->GetRecordCount() - 1; i++) {
        PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
        auto newUri = std::make_shared<OHOS::Uri>(uri);
        builder.SetUri(newUri);
        builder.SetCustomData(pasteDataRecords[i]->GetCustomData());
        auto record = builder.Build();
        newPasteData->AddRecord(record);
    }
    EXPECT_EQ(newPasteData->GetRecordCount(), 2);
    std::shared_ptr<std::string> newHtml = webClipboardController.RebuildHtml(newPasteData);
    EXPECT_EQ(newPasteData->GetRecordCount(), 1);
    const char* newHtmlStr = newHtml.get()->c_str();
    EXPECT_STREQ(newHtmlStr, execptHtml);
}

/**
 * @tc.name: RebuildHtmlTest_006.
 * @tc.desc: Test contains a multiple image address HTML with MergeExtraUris2Html.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, RebuildHtmlTest_006, TestSize.Level1)
{
    const std::string uri = "file:///data/storage/el2/distributedfiles/temp.png";
    auto webClipboardController = PasteboardWebController::GetInstance();
    std::shared_ptr<std::string> html(
        new std::string("<img data-ohos='clipboard' src='file:///file1.jpg'><img data-ohos='clipboard' "
                        "src=\"file2.jpg\"><img data-ohos='clipboard' "
                        "src='https://data/storage/el2/distributedfiles/202305301.png'>"));
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(*html);
    auto htmlRecord = pasteData->GetRecordAt(0);
    htmlRecord->SetFrom(htmlRecord->GetRecordId());

    const char* execptHtml =
        "<img data-ohos='clipboard' src='file:///data/storage/el2/distributedfiles/temp.png'><img "
        "data-ohos='clipboard' "
        "src=\"file:///data/storage/el2/distributedfiles/temp.png\"><img data-ohos='clipboard' "
        "src='https://data/storage/el2/distributedfiles/202305301.png'>";
    auto records = webClipboardController.SplitHtml2Records(html, htmlRecord->GetRecordId());
    EXPECT_EQ(records.size(), 2);
    EXPECT_EQ(records[0]->GetFrom(), htmlRecord->GetRecordId());
    pasteData->AddRecord(records[0]);
    pasteData->AddRecord(records[1]);

    std::shared_ptr<PasteData> newPasteData = std::make_shared<PasteData>();
    std::vector<std::shared_ptr<PasteDataRecord>> pasteDataRecords = pasteData->AllRecords();
    EXPECT_EQ(*(pasteDataRecords[pasteData->GetRecordCount() - 1]->GetHtmlText()), *html);

    newPasteData->AddHtmlRecord(*html);
    auto newHtmlRecord = newPasteData->GetRecordAt(0);
    newHtmlRecord->SetFrom(newHtmlRecord->GetRecordId());
    for (auto i = 0; i < pasteData->GetRecordCount() - 1; i++) {
        PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
        auto newUri = std::make_shared<OHOS::Uri>(uri);
        builder.SetUri(newUri);
        builder.SetCustomData(pasteDataRecords[i]->GetCustomData());
        auto record = builder.Build();
        record->SetFrom(newHtmlRecord->GetRecordId());
        newPasteData->AddRecord(record);
    }
    EXPECT_EQ(newPasteData->GetRecordCount(), 3);
    webClipboardController.MergeExtraUris2Html(*newPasteData);
    ASSERT_EQ(newPasteData->GetRecordCount(), 1);
    auto recordGet = newPasteData->GetRecordAt(0);
    auto newHtmlStr = recordGet->GetHtmlText();
    ASSERT_NE(newHtmlStr, nullptr);
    EXPECT_STREQ(newHtmlStr->c_str(), execptHtml);
}

/**
 * @tc.name: SplitHtmlTest_007.
 * @tc.desc: Test contains invalid protocol image link HTML with SplitHtml.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, SplitHtmlTest_007, TestSize.Level1)
{
    auto webClipboardController = PasteboardWebController::GetInstance();
    std::shared_ptr<std::string> html(
        new std::string("<img data-ohos='clipboard' src='xxx://file1.jpg'><img data-ohos='clipboard' "
                        "src='yyy://data/storage/el2/distributedfiles/202305301.png'>"));
    auto records = webClipboardController.SplitHtml2Records(html, 1);
    EXPECT_EQ(records.size(), 0);
}

/**
 * @tc.name: RebuildHtmlTest_008.
 * @tc.desc: Test contains invalid protocol image link HTML with RebuildHtml.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, RebuildHtmlTest_008, TestSize.Level1)
{
    auto webClipboardController = PasteboardWebController::GetInstance();
    std::shared_ptr<std::string> html(
        new std::string("<img data-ohos='clipboard' src='xxx://file1.jpg'><img data-ohos='clipboard' "
                        "src='ttt://data/storage/el2/distributedfiles/202305301.png'>"));
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(*html);
    EXPECT_EQ(pasteData->GetRecordCount(), 1);
    std::shared_ptr<std::string> newHtml = webClipboardController.RebuildHtml(pasteData);
    EXPECT_EQ(*newHtml, *html);
}

/**
 * @tc.name: RebuildHtmlTest_009.
 * @tc.desc: Test contains a local image address HTML with RebuildHtml.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, RebuildHtmlTest_009, TestSize.Level1)
{
    const std::string uri = "file:///data/storage/el2/distributedfiles/temp.png";
    auto webClipboardController = PasteboardWebController::GetInstance();
    std::shared_ptr<std::string> html(
        new std::string("<img src='file:///file1.jpg'><img src=\"file2.jpg\"><img "
                        "src='https://data/storage/el2/distributedfiles/202305301.png'>"));
    const char* execptHtml =
        "<img src='file:///data/storage/el2/distributedfiles/temp.png'><img "
        ""
        "src=\"file:///data/storage/el2/distributedfiles/temp.png\"><img "
        "src='https://data/storage/el2/distributedfiles/202305301.png'>";
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(*html);
    auto htmlRecord = pasteData->GetRecordAt(0);
    htmlRecord->SetFrom(htmlRecord->GetRecordId());
    auto records = webClipboardController.SplitHtml2Records(html, htmlRecord->GetRecordId());
    EXPECT_EQ(records.size(), 2);
    pasteData->AddRecord(records[0]);
    pasteData->AddRecord(records[1]);

    std::shared_ptr<PasteData> newPasteData = std::make_shared<PasteData>();
    std::vector<std::shared_ptr<PasteDataRecord>> pasteDataRecords = pasteData->AllRecords();
    EXPECT_EQ(*(pasteDataRecords[pasteData->GetRecordCount() - 1]->GetHtmlText()), *html);

    newPasteData->AddHtmlRecord(*html);
    for (auto i = 0; i < pasteData->GetRecordCount() - 1; i++) {
        PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
        auto newUri = std::make_shared<OHOS::Uri>(uri);
        builder.SetUri(newUri);
        builder.SetCustomData(pasteDataRecords[i]->GetCustomData());
        auto record = builder.Build();
        newPasteData->AddRecord(record);
    }
    EXPECT_EQ(newPasteData->GetRecordCount(), 3);
    std::shared_ptr<std::string> newHtml = webClipboardController.RebuildHtml(newPasteData);
    EXPECT_EQ(newPasteData->GetRecordCount(), 1);
    const char* newHtmlStr = newHtml.get()->c_str();
    EXPECT_STREQ(newHtmlStr, execptHtml);
}

/**
 * @tc.name: RebuildHtmlTest_010.
 * @tc.desc: Test contains a local image address HTML with MergeExtraUris2Html.
 * @tc.type: FUNC.
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(WebControllerTest, RebuildHtmlTest_010, TestSize.Level1)
{
    const std::string uri = "file:///data/storage/el2/distributedfiles/temp.png";
    auto webClipboardController = PasteboardWebController::GetInstance();
    std::shared_ptr<std::string> html(
            new std::string("<img src='file:///file1.jpg'><img src=\"file2.jpg\"><img "
                            "src='https://data/storage/el2/distributedfiles/202305301.png'>"));
    const char* execptHtml =
            "<img src='file:///data/storage/el2/distributedfiles/temp.png'><img "
            "src=\"file:///data/storage/el2/distributedfiles/temp.png\"><img "
            "src='https://data/storage/el2/distributedfiles/202305301.png'>";
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(*html);
    auto htmlRecord = pasteData->GetRecordAt(0);
    htmlRecord->SetFrom(htmlRecord->GetRecordId());
    auto records = webClipboardController.SplitHtml2Records(html, htmlRecord->GetRecordId());
    ASSERT_EQ(records.size(), 2);
    EXPECT_EQ(records[0]->GetFrom(), htmlRecord->GetRecordId());
    pasteData->AddRecord(records[0]);
    pasteData->AddRecord(records[1]);

    std::shared_ptr<PasteData> newPasteData = std::make_shared<PasteData>();
    std::vector<std::shared_ptr<PasteDataRecord>> pasteDataRecords = pasteData->AllRecords();
    EXPECT_EQ(*(pasteDataRecords[pasteData->GetRecordCount() - 1]->GetHtmlText()), *html);

    newPasteData->AddHtmlRecord(*html);
    auto newHtmlRecord = newPasteData->GetRecordAt(0);
    newHtmlRecord->SetFrom(newHtmlRecord->GetRecordId());
    for (auto i = 0; i < pasteData->GetRecordCount() - 1; i++) {
        PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
        auto newUri = std::make_shared<OHOS::Uri>(uri);
        builder.SetUri(newUri);
        builder.SetCustomData(pasteDataRecords[i]->GetCustomData());
        auto record = builder.Build();
        record->SetFrom(newHtmlRecord->GetRecordId());
        newPasteData->AddRecord(record);
    }
    EXPECT_EQ(newPasteData->GetRecordCount(), 3);
    webClipboardController.MergeExtraUris2Html(*newPasteData);

    ASSERT_EQ(newPasteData->GetRecordCount(), 1);
    auto recordGet = newPasteData->GetRecordAt(0);
    auto newHtmlStr = recordGet->GetHtmlText();
    ASSERT_NE(newHtmlStr, nullptr);
    EXPECT_STREQ(newHtmlStr->c_str(), execptHtml);
}