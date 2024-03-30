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

#include "pasteboard_client.h"
#include <gtest/gtest.h>

#include "plain_text.h"
#include "system_defined_pixelmap.h"
#include "file.h"
#include "html.h"
namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::Media;
class PasteboardClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardClientTest::SetUpTestCase(void)
{
}

void PasteboardClientTest::TearDownTestCase(void)
{
}

void PasteboardClientTest::SetUp(void)
{
}

void PasteboardClientTest::TearDown(void)
{
}

/**
* @tc.name: IsRemoteData001
* @tc.desc: pasteData is local data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, IsRemoteData001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    bool ret = PasteboardClient::GetInstance()->IsRemoteData();
    ASSERT_FALSE(ret);
}

/**
* @tc.name: IsRemoteData002
* @tc.desc: pasteData is remote data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, IsRemoteData002, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    pasteData->SetRemote(true);
    PasteboardClient::GetInstance()->SetPasteData(*pasteData);
    bool ret = PasteboardClient::GetInstance()->IsRemoteData();
    ASSERT_TRUE(ret);
}

/**
* @tc.name: HasDataType001
* @tc.desc: data type is MIMETYPE_TEXT_PLAIN.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, HasDataType001, TestSize.Level0)
{
    std::string plainText = "helloWorld";
    auto newData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    PasteboardClient::GetInstance()->SetPasteData(*newData);
    auto ret = PasteboardClient::GetInstance()->HasDataType(MIMETYPE_TEXT_PLAIN);
    ASSERT_TRUE(ret);
    auto result = PasteboardClient::GetInstance()->HasDataType(MIMETYPE_TEXT_URI);
    ASSERT_FALSE(result);
}

/**
* @tc.name: HasDataType002
* @tc.desc: data type is MIMETYPE_TEXT_HTML.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, HasDataType002, TestSize.Level0)
{
    std::string htmlText = "<div class='disable'>helloWorld</div>";
    auto newPasteData = PasteboardClient::GetInstance()->CreateHtmlData(htmlText);
    PasteboardClient::GetInstance()->SetPasteData(*newPasteData);
    auto ret = PasteboardClient::GetInstance()->HasDataType(MIMETYPE_TEXT_HTML);
    ASSERT_TRUE(ret);
    auto result = PasteboardClient::GetInstance()->HasDataType(MIMETYPE_TEXT_PLAIN);
    ASSERT_FALSE(result);
}

/**
* @tc.name: HasDataType003
* @tc.desc: data type is MIMETYPE_TEXT_URI
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, HasDataType003, TestSize.Level0)
{
    OHOS::Uri uri("uri");
    auto newPasteData = PasteboardClient::GetInstance()->CreateUriData(uri);
    PasteboardClient::GetInstance()->SetPasteData(*newPasteData);
    auto ret = PasteboardClient::GetInstance()->HasDataType(MIMETYPE_TEXT_URI);
    ASSERT_TRUE(ret);
    auto result = PasteboardClient::GetInstance()->HasDataType(MIMETYPE_TEXT_PLAIN);
    ASSERT_FALSE(result);
}

/**
* @tc.name: HasDataType004
* @tc.desc: data type is MIMETYPE_PIXELMAP
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, HasDataType004, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto newPasteData = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMapIn);
    PasteboardClient::GetInstance()->SetPasteData(*newPasteData);
    auto ret = PasteboardClient::GetInstance()->HasDataType(MIMETYPE_PIXELMAP);
    ASSERT_TRUE(ret);
    auto result = PasteboardClient::GetInstance()->HasDataType(MIMETYPE_TEXT_URI);
    ASSERT_FALSE(result);
}

/**
* @tc.name: GetDataSource001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, GetDataSource001, TestSize.Level0)
{
    std::string plainText = "helloWorld";
    auto newData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    PasteboardClient::GetInstance()->SetPasteData(*newData);
    std::string bundleName;
    PasteboardClient::GetInstance()->GetDataSource(bundleName);
    EXPECT_FALSE(bundleName.empty());
}

/**
* @tc.name: SetUnifiedData001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, SetUnifiedData001, TestSize.Level0)
{
    std::string text = "helloWorld";
    UDMF::UnifiedData data;
    std::shared_ptr<UDMF::PlainText> plainTextRecord = std::make_shared<UDMF::PlainText>(text, text);
    data.AddRecord(plainTextRecord);
    PasteboardClient::GetInstance()->SetUnifiedData(data);

    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    auto record = newData.GetRecordAt(0);
    auto type = record->GetType();
    ASSERT_EQ(type, UDMF::PLAIN_TEXT);
    auto content = static_cast<UDMF::PlainText *>(record.get())->GetContent();
    ASSERT_EQ(content, text);

    UDMF::UnifiedDataProperties properties = newData.GetProperties();
    ASSERT_EQ(UDMF::PLAIN_TEXT, properties.types.at(0));

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    auto pasteRecord = pasteData.GetRecordAt(0);
    auto mimeType = pasteRecord->GetMimeType();
    ASSERT_EQ(mimeType, MIMETYPE_TEXT_PLAIN);
    auto pasteContent = pasteRecord->GetPlainText();
    ASSERT_EQ(*pasteContent, text);

    auto pasteProp = pasteData.GetProperty();
    ASSERT_EQ(pasteProp.mimeTypes.size(), 1);
    ASSERT_TRUE(std::count(pasteProp.mimeTypes.begin(), pasteProp.mimeTypes.end(), MIMETYPE_TEXT_PLAIN));

}

/**
* @tc.name: SetUnifiedData002
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, SetUnifiedData002, TestSize.Level0)
{
    UDMF::UnifiedData data;

    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto rawData = PasteDataRecord::PixelMap2Vector(pixelMapIn);
    auto pixelMapRecord = std::make_shared<UDMF::SystemDefinedPixelMap>(rawData);
    data.AddRecord(pixelMapRecord);
    PasteboardClient::GetInstance()->SetUnifiedData(data);

    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    auto record = newData.GetRecordAt(0);
    auto type = record->GetType();
    ASSERT_EQ(type, UDMF::SYSTEM_DEFINED_PIXEL_MAP);
    auto content = static_cast<UDMF::SystemDefinedPixelMap *>(record.get())->GetRawData();
    ASSERT_EQ(rawData, content);

    UDMF::UnifiedDataProperties properties;
    newData.GetProperties();
    ASSERT_EQ(UDMF::SYSTEM_DEFINED_PIXEL_MAP, properties.types.at(0));
}

/**
* @tc.name: SetUnifiedData003
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, SetUnifiedData003, TestSize.Level0)
{
    UDMF::UnifiedData data;
    std::string str = "file:/uri";
    OHOS::Uri uri(str);
    auto file = std::make_shared<UDMF::File>(uri.ToString());

    data.AddRecord(file);
    PasteboardClient::GetInstance()->SetUnifiedData(data);

    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    auto record = newData.GetRecordAt(0);
    auto type = record->GetType();
    ASSERT_EQ(type, UDMF::FILE);
    auto content = static_cast<UDMF::File *>(record.get())->GetUri();
    ASSERT_EQ(str, content);

    UDMF::UnifiedDataProperties properties;
    newData.GetProperties();
    ASSERT_EQ(UDMF::FILE, properties.types.at(0));
}

/**
* @tc.name: SetUnifiedData004
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, SetUnifiedData004, TestSize.Level0)
{
    UDMF::UnifiedData data;
    std::string htmlText = "<div class='disable'>helloWorld</div>";
    std::string plainText = "helloWorld";
    auto html = std::make_shared<UDMF::Html>(htmlText, plainText);

    data.AddRecord(html);
    PasteboardClient::GetInstance()->SetUnifiedData(data);

    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    auto record = newData.GetRecordAt(0);
    auto type = record->GetType();
    ASSERT_EQ(type, UDMF::HTML);
    auto content = static_cast<UDMF::Html *>(record.get())->GetHtmlContent();
    auto plainContent = static_cast<UDMF::Html *>(record.get())->GetPlainContent();
    ASSERT_EQ(content, htmlText);
    ASSERT_EQ(plainContent, ""); //

    UDMF::UnifiedDataProperties properties = newData.GetProperties();
    ASSERT_EQ(UDMF::HTML, properties.types.at(0));
}

/**
* @tc.name: SetUnifiedData005
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, SetUnifiedData005, TestSize.Level0)
{
    UDMF::UnifiedData data;

    std::string text = "helloWorld";
    std::shared_ptr<UDMF::PlainText> plainTextRecord = std::make_shared<UDMF::PlainText>(text, text);
    data.AddRecord(plainTextRecord);

    std::string htmlText = "<div class='disable'>helloWorld</div>";
    auto html = std::make_shared<UDMF::Html>(htmlText, text);
    data.AddRecord(html);

    std::string str = "file:/uri";
    OHOS::Uri uri(str);
    auto file = std::make_shared<UDMF::File>(uri.ToString());
    data.AddRecord(file);

    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto rawData = PasteDataRecord::PixelMap2Vector(pixelMapIn);
    auto pixelMapRecord = std::make_shared<UDMF::SystemDefinedPixelMap>(rawData);
    data.AddRecord(pixelMapRecord);

    PasteboardClient::GetInstance()->SetUnifiedData(data);

    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    auto records = newData.GetRecords();
    ASSERT_EQ(records.size(), 4);

    auto record = newData.GetRecordAt(0);
    auto type = record->GetType();
    ASSERT_EQ(type, UDMF::PLAIN_TEXT);
    auto content = static_cast<UDMF::PlainText *>(record.get())->GetContent();
    ASSERT_EQ(content, text);

    record = newData.GetRecordAt(1);
    type = record->GetType();
    ASSERT_EQ(type, UDMF::HTML);
    content = static_cast<UDMF::Html *>(record.get())->GetHtmlContent();
    auto plainContent = static_cast<UDMF::Html *>(record.get())->GetPlainContent();
    ASSERT_EQ(content, htmlText);

    record = newData.GetRecordAt(2);
    type = record->GetType();
    ASSERT_EQ(type, UDMF::FILE);
    content = static_cast<UDMF::File *>(record.get())->GetUri();
    ASSERT_EQ(str, content);

    record = newData.GetRecordAt(3);
    type = record->GetType();
    ASSERT_EQ(type, UDMF::SYSTEM_DEFINED_PIXEL_MAP);
    auto contentRawData = static_cast<UDMF::SystemDefinedPixelMap *>(record.get())->GetRawData();
    ASSERT_EQ(rawData, contentRawData);

    UDMF::UnifiedDataProperties properties = newData.GetProperties();
    ASSERT_EQ(properties.types.size(), 4);
    ASSERT_TRUE(std::count(properties.types.begin(), properties.types.end(), UDMF::PLAIN_TEXT));
    ASSERT_TRUE(std::count(properties.types.begin(), properties.types.end(), UDMF::FILE));
    ASSERT_TRUE(std::count(properties.types.begin(), properties.types.end(), UDMF::HTML));
    ASSERT_TRUE(std::count(properties.types.begin(), properties.types.end(), UDMF::SYSTEM_DEFINED_PIXEL_MAP));

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    auto pasteProp = pasteData.GetProperty();
    ASSERT_EQ(pasteProp.mimeTypes.size(), 4);
    ASSERT_TRUE(std::count(pasteProp.mimeTypes.begin(), pasteProp.mimeTypes.end(), MIMETYPE_TEXT_PLAIN));
    ASSERT_TRUE(std::count(pasteProp.mimeTypes.begin(), pasteProp.mimeTypes.end(), MIMETYPE_TEXT_URI));
    ASSERT_TRUE(std::count(pasteProp.mimeTypes.begin(), pasteProp.mimeTypes.end(), MIMETYPE_TEXT_HTML));
    ASSERT_TRUE(std::count(pasteProp.mimeTypes.begin(), pasteProp.mimeTypes.end(), MIMETYPE_PIXELMAP));
}
} // namespace OHOS::MiscServices