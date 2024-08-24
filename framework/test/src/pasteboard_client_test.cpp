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
#include "pasteboard_pattern.h"
#include "unistd.h"
#include <gtest/gtest.h>

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::Media;
constexpr const uid_t EDM_UID = 3057;
using Patterns = std::unordered_set<Pattern>;
class PasteboardClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
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

/**
* @tc.name: DetectPatterns001
* @tc.desc: Cover Permutation
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, DetectPatterns001, TestSize.Level0)
{
    std::string plainText("每天三次抢红I包机会，速抢！】qd rqdswww.comsski,.sjopwe"
    "ihhtpsdhttp我也带过去给他№のjioijhhu");
    std::string plainText0("https://github.com/makelove/Taobao_to"
    "psdk/blob/master.md）");
    std::string plainText1("最高888元82h7");
    std::string plainText2("uhiyqydueuw@kahqw.oisko.sji");

    std::vector<std::string> plainTextVec{
        plainText, plainText+plainText0, plainText+plainText1, plainText+plainText2,
        plainText+plainText0+plainText1, plainText0+plainText2+plainText, plainText1+plainText+plainText2,
        plainText0+plainText1+plainText+plainText2
    };
    std::vector<Patterns> patternsVec{
        {}, {Pattern::URL}, {Pattern::Number}, {Pattern::EmailAddress},
        {Pattern::URL, Pattern::Number}, {Pattern::URL, Pattern::EmailAddress},
        {Pattern::Number, Pattern::EmailAddress}, {Pattern::URL, Pattern::Number, Pattern::EmailAddress}
    };
    std::vector<std::vector<int>> patternsRightIndexVec{
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 1, 1, 0, 1},
        {0, 0, 2, 0, 2, 0, 2, 2},
        {0, 0, 0, 3, 0, 3, 3, 3},
        {0, 1, 2, 0, 4, 1, 2, 4},
        {0, 1, 0, 3, 1, 5, 3, 5},
        {0, 0, 2, 3, 2, 3, 6, 6},
        {0, 1, 2, 3, 4, 5, 6, 7}
    };
    for (int i = 0; i != 8; ++i) {
        for (int j = 0; j != 8; ++j) {
            auto newData = PasteboardClient::GetInstance()->CreatePlainTextData(
                plainTextVec[i]);
            PasteboardClient::GetInstance()->SetPasteData(*newData);
            auto ret = PasteboardClient::GetInstance()->DetectPatterns(
                patternsVec[j]);
            int rightIndex = patternsRightIndexVec[i][j];
            ASSERT_EQ(ret, patternsVec[rightIndex]);
        }
    }
}

/**
* @tc.name: DetectPatterns002
* @tc.desc: check HTML
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, DetectPatterns002, TestSize.Level0)
{
    std::string htmlText1 = "<!DOCTYPE html><html><head><title>"
    "超链接示例</title></head><body><h2>访问我的网站</h2>"
    "<p>点击下面的链接访问我的<a href=\"https://example.com\">"
    "个人网站https://example.com</a>。</p></body></html>";
    auto newData1 = PasteboardClient::GetInstance()->CreateHtmlData(htmlText1);
    PasteboardClient::GetInstance()->SetPasteData(*newData1);
    std::unordered_set<Pattern> patternsToCheck1{Pattern::URL, Pattern::EmailAddress};
    auto ret1 = PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck1);
    std::unordered_set<Pattern> expected1{Pattern::URL};
    ASSERT_EQ(ret1, expected1);

    std::string htmlText2 = "<!DOCTYPE html><html><head><title>"
    "超链接示例</title></head><body><h2>访问我的网站</h2>"
    "<p>点击下面的链接访https://example.com问我的<a href=\"https://example.com\">"
    "个人网站weqkqo23334@example.com</a>。</p></body></html>";
    auto newData2 = PasteboardClient::GetInstance()->CreateHtmlData(htmlText2);
    PasteboardClient::GetInstance()->SetPasteData(*newData2);
    std::unordered_set<Pattern> patternsToCheck2{Pattern::URL, Pattern::EmailAddress, Pattern::Number};
    auto ret2 = PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck2);
    std::unordered_set<Pattern> expected1{Pattern::URL, Pattern::EmailAddress};
    ASSERT_EQ(ret2, expected2);
}

/**
* @tc.name: DetectPatterns003
* @tc.desc: Outlier force cast uint32_t to unsurportted Pattern
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, DetectPatterns003, TestSize.Level0)
{
    std::string plainText1 = "来自网盘分享文件：\n"
    "「格式测试」\n"
    "链接：\n"
    "https://pre-drive.uc.cn/s/42e1d77f3dab4"
    "网盘——上传下载不限速，网速有多快，速度就有多快。复制整段内容后打开最新版「浏览器」打开:\n"
    "~b00433tddj~";
    auto newData1 = PasteboardClient::GetInstance()->CreatePlainTextData(plainText1);
    PasteboardClient::GetInstance()->SetPasteData(*newData1);
    std::unordered_set<Pattern> patternsToCheck{
        Pattern::Number, Pattern::URL, Pattern::EmailAddress, static_cast<Pattern>(1023)};
    auto ret1 = PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck);
    std::unordered_set<Pattern> expected1{Pattern::Number, Pattern::URL};
    ASSERT_EQ(ret1, expected1);
    std::string plainText2 = "【最高元，每天三次抢红包机会，速抢！】"
    "年货合家欢，五折开抢！复制此条信息，打开手机淘宝即可 查看，淘口令：trfwrtg"
    "(￥￥软骨素用人员为bdfdgse https://github.com/makelove/ usdq12_22swe@163.com）";
    auto newData2 = PasteboardClient::GetInstance()->CreatePlainTextData(plainText2);
    PasteboardClient::GetInstance()->SetPasteData(*newData2);
    auto ret2 = PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck);
    std::unordered_set<Pattern> expected2{Pattern::Number, Pattern::URL, Pattern::EmailAddress};
    ASSERT_EQ(ret2, expected2);
    std::string plainText3 = "【最高元，每天三次抢红aefw包机会，速抢！】"
    "年货合家欢，五折开抢！复制此条信息，打开sfsdf手机淘宝即可 查看，淘口令：defeqfdt）";
    auto newData3 = PasteboardClient::GetInstance()->CreatePlainTextData(plainText3);
    PasteboardClient::GetInstance()->SetPasteData(*newData3);
    auto ret3 = PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck);
    ASSERT_EQ(ret3, std::unordered_set<Pattern>{});
}

/**
* @tc.name: DetectPatterns004
* @tc.desc: Outlier force cast uint32_t 0xffffffff to unsurportted Pattern
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardClientTest, DetectPatterns004, TestSize.Level0)
{
    std::string plainText1 = "来自网盘分享文件：\n"
    "「格式测试」\n"
    "链接：\n"
    "https://pre-drive.uc.cn/s/42e1d77f3dab4"
    "网盘——上传下载不限速，网速有多快，速度就有多快。复制整段内容后打开最新版「浏览器」打开:\n"
    "~b00433tddj~";
    auto newData1 = PasteboardClient::GetInstance()->CreatePlainTextData(plainText1);
    PasteboardClient::GetInstance()->SetPasteData(*newData1);
    std::unordered_set<Pattern> patternsToCheck{
        Pattern::Number, Pattern::URL, Pattern::EmailAddress,
        static_cast<Pattern>(0xffffffff), static_cast<Pattern>(0xffffff1a)};
    auto ret1 = PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck);
    std::unordered_set<Pattern> expected1{Pattern::Number, Pattern::URL};
    ASSERT_EQ(ret1, expected1);
    std::string plainText2 = "【最高元，每天三次抢红包机会，速抢！】"
    "年货合家欢，五折开抢！复制此条信息，打开手机淘宝即可 查看，淘口令：trfwrtg"
    "(￥￥软骨素用人员为bdfdgse https://github.com/makelove/ usdq12_22swe@163.com）";
    auto newData2 = PasteboardClient::GetInstance()->CreatePlainTextData(plainText2);
    PasteboardClient::GetInstance()->SetPasteData(*newData2);
    auto ret2 = PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck);
    std::unordered_set<Pattern> expected2{Pattern::Number, Pattern::URL, Pattern::EmailAddress};
    ASSERT_EQ(ret2, expected2);
    std::string plainText3 = "【最高元，每天三次抢红aefw包机会，速抢！】"
    "年货合家欢，五折开抢！复制此条信息，打开sfsdf手机淘宝即可 查看，淘口令：defeqfdt）";
    auto newData3 = PasteboardClient::GetInstance()->CreatePlainTextData(plainText3);
    PasteboardClient::GetInstance()->SetPasteData(*newData3);
    auto ret3 = PasteboardClient::GetInstance()->DetectPatterns(patternsToCheck);
    ASSERT_EQ(ret3, std::unordered_set<Pattern>{});
}

} // namespace OHOS::MiscServices