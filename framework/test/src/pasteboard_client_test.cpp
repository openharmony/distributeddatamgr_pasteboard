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
#include "unistd.h"
#include <gtest/gtest.h>

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::Media;
constexpr const uid_t EDM_UID = 3057;
class PasteboardClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardClientTest::SetUpTestCase(void)
{
    setuid(EDM_UID);
}

void PasteboardClientTest::TearDownTestCase(void)
{
    setuid(0);
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
* @tc.name: SetGlobalShareOption
* @tc.desc: Set global shareOption
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, SetGlobalShareOption, TestSize.Level0)
{
    std::map<uint32_t, ShareOption> settings = {
        {100, ShareOption::InApp},
        {200, ShareOption::LocalDevice},
        {300, ShareOption::CrossDevice}};
    PasteboardClient::GetInstance()->SetGlobalShareOption(settings);
    auto result = PasteboardClient::GetInstance()->GetGlobalShareOption({});
    EXPECT_TRUE(result.size() == 3);
    EXPECT_EQ(result[100], ShareOption::InApp);
    EXPECT_EQ(result[200], ShareOption::LocalDevice);
    EXPECT_EQ(result[300], ShareOption::CrossDevice);
    std::map<uint32_t, ShareOption> modify = {{100, ShareOption::CrossDevice},
                                              {400, ShareOption::InApp}};
    PasteboardClient::GetInstance()->SetGlobalShareOption(modify);
    result = PasteboardClient::GetInstance()->GetGlobalShareOption({});
    EXPECT_TRUE(result.size() == 4);
    EXPECT_EQ(result[100], ShareOption::CrossDevice);
    EXPECT_EQ(result[400], ShareOption::InApp);
    PasteboardClient::GetInstance()->RemoveGlobalShareOption({100, 200, 300, 400});
}

/**
* @tc.name: GetGlobalShareOption
* @tc.desc: Get global shareOption
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, GetGlobalShareOption, TestSize.Level0)
{
    std::map<uint32_t, ShareOption> settings = {
        {100, ShareOption::InApp},
        {200, ShareOption::LocalDevice},
        {300, ShareOption::CrossDevice}};
    PasteboardClient::GetInstance()->SetGlobalShareOption(settings);
    auto result = PasteboardClient::GetInstance()->GetGlobalShareOption({});
    EXPECT_TRUE(result.size() == 3);
    EXPECT_EQ(result[100], ShareOption::InApp);
    EXPECT_EQ(result[200], ShareOption::LocalDevice);
    EXPECT_EQ(result[300], ShareOption::CrossDevice);
    result = PasteboardClient::GetInstance()->GetGlobalShareOption({100, 400});
    EXPECT_TRUE(result.size() == 1);
    EXPECT_EQ(result[100], ShareOption::InApp);
    EXPECT_TRUE(result.find(400) == result.end());
    PasteboardClient::GetInstance()->RemoveGlobalShareOption({100, 200, 300});
}

/**
* @tc.name: RemoveGlobalShareOption
* @tc.desc: Remove global shareOption
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, RemoveGlobalShareOption, TestSize.Level0)
{
    std::map<uint32_t, ShareOption> settings = {
        {100, ShareOption::InApp},
        {200, ShareOption::LocalDevice},
        {300, ShareOption::CrossDevice}};
    PasteboardClient::GetInstance()->SetGlobalShareOption(settings);
    auto result = PasteboardClient::GetInstance()->GetGlobalShareOption({});
    EXPECT_TRUE(result.size() == 3);
    EXPECT_EQ(result[100], ShareOption::InApp);
    EXPECT_EQ(result[200], ShareOption::LocalDevice);
    EXPECT_EQ(result[300], ShareOption::CrossDevice);
    PasteboardClient::GetInstance()->RemoveGlobalShareOption({});
    result = PasteboardClient::GetInstance()->GetGlobalShareOption({});
    EXPECT_TRUE(result.size() == 3);
    PasteboardClient::GetInstance()->RemoveGlobalShareOption({100, 400});
    result = PasteboardClient::GetInstance()->GetGlobalShareOption({});
    EXPECT_TRUE(result.size() == 2);
    EXPECT_TRUE(result.find(100) == result.end());
    PasteboardClient::GetInstance()->RemoveGlobalShareOption({200, 300});
}
} // namespace OHOS::MiscServices