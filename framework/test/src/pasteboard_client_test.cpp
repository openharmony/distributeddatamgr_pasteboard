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

#include "common/block_object.h"
#include "int_wrapper.h"
#include "pasteboard_client.h"
#include "remote_file_share.h"
#include <gtest/gtest.h>

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
    (void) PasteboardClient::GetInstance()->GetDataSource(bundleName);
    EXPECT_FALSE(bundleName.empty());
}
} // namespace OHOS::MiscServices