/*
* Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "copy_uri_handler.h"
#include "common/block_object.h"
#include "clip/clip_plugin.h"
#include "clip_factory.h"
#include "int_wrapper.h"
#include "paste_uri_handler.h"
#include "pasteboard_client.h"
#include "remote_file_share.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::AAFwk;
using namespace OHOS::Media;
constexpr const int32_t INVALID_FD = -1;
constexpr const char *FILE_URI = "/data/test/resource/pasteboardTest.txt";
class PasteDataTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteDataTest::SetUpTestCase(void)
{
}

void PasteDataTest::TearDownTestCase(void)
{
}

void PasteDataTest::SetUp(void)
{
}

void PasteDataTest::TearDown(void)
{
}

ClipFactory::ClipFactory()
{
    ClipPlugin::RegCreator("distributed_clip", this);
}

class UriHandlerMock : public UriHandler {
public:
    UriHandlerMock() = default;
    virtual ~UriHandlerMock() = default;

    MOCK_METHOD1(ToUri, std::string(int32_t fd));
    MOCK_METHOD2(ToFd, int32_t(const std::string &uri, bool isClient));
    MOCK_CONST_METHOD1(IsFile, bool(const std::string &uri));
};
/**
* @tc.name: ReplaceShareUri001
* @tc.desc: replace user id in share path
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: baoyayong
*/
HWTEST_F(PasteDataTest, ReplaceShareUri001, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = "/data/storage/100/haps/caches/xxx.txt";
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();

    // mock
    UriHandlerMock mock;
    std::string mockUri = "/mnt/hmdfs/100/account/merge_view/services/psteboard_service/.share/xxx.txt";
    EXPECT_CALL(mock, ToUri(_)).WillRepeatedly(Return(mockUri));
    EXPECT_CALL(mock, ToFd(_, _)).WillRepeatedly(Return(2));
    EXPECT_CALL(mock, IsFile(_)).WillRepeatedly(Return(true));

    data.AddRecord(record);
    MessageParcel parcel;
    data.WriteUriFd(parcel, mock);
    bool result = data.ReadUriFd(parcel, mock);
    EXPECT_TRUE(result);
    EXPECT_EQ(mockUri, data.GetPrimaryUri()->ToString());
    data.ReplaceShareUri(200);
    std::string mockUri2 = "/mnt/hmdfs/200/account/merge_view/services/psteboard_service/.share/xxx.txt";
    EXPECT_EQ(mockUri2, data.GetPrimaryUri()->ToString());
}
/**
* @tc.name: uriConvertTest001
* @tc.desc: uri convert(in same app)
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, uriConvertTest001, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = FILE_URI;
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();
    data.AddRecord(record);

    MessageParcel parcel;
    CopyUriHandler copyHandler;
    data.WriteUriFd(parcel, copyHandler);
    bool result = data.ReadUriFd(parcel, copyHandler);
    EXPECT_TRUE(result);
    auto distributedUri = data.GetPrimaryUri()->ToString();
    EXPECT_FALSE(uriStr == distributedUri);

    MessageParcel parcel1;
    PasteUriHandler pasteHandler;
    int32_t fd = 5;
    pasteHandler.ToUri(fd);

    data.SetLocalPasteFlag(true);
    data.WriteUriFd(parcel1, pasteHandler);
    result = data.ReadUriFd(parcel1, pasteHandler);
    EXPECT_TRUE(result);
    ASSERT_TRUE(data.GetPrimaryUri() != nullptr);
    auto convertedUri = data.GetPrimaryUri()->ToString();
    EXPECT_EQ(distributedUri, convertedUri);
    EXPECT_FALSE(uriStr == convertedUri);
}

/**
* @tc.name: uriConvertTest002
* @tc.desc: uri convert(in same app)
* @tc.type: FUNC
*/
HWTEST_F(PasteDataTest, uriConvertTest002, TestSize.Level0)
{
    PasteUriHandler pasteHandler;
    int32_t fd = -100;
    std::string convertUri = pasteHandler.ToUri(fd);
    EXPECT_TRUE(convertUri == "");
}

/**
* @tc.name: GetRealPathFailed001
* @tc.desc: GetRealPath Failed(realpath(inOriPath.c_str(), realPath) == nullptr)
* @tc.type: FUNC
* @tc.require: issuesI5Y6PO
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, GetRealPathFailed001, TestSize.Level0)
{
    std::string uriStr = "/data/storage/100/haps/caches/xxx.txt";
    PasteUriHandler pasteHandler;
    auto ret = pasteHandler.ToFd(uriStr, true);
    EXPECT_EQ(ret, INVALID_FD);
}

/**
* @tc.name: GetRealPathFailed002
* @tc.desc: GetRealPath Failed(inOriPath.size() > PATH_MAX)
* @tc.type: FUNC
* @tc.require: issuesI5Y6PO
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, GetRealPathFailed002, TestSize.Level0)
{
    std::string uriStr(PATH_MAX + 2, '*');
    PasteUriHandler pasteHandler;
    auto ret = pasteHandler.ToFd(uriStr, true);
    EXPECT_EQ(ret, INVALID_FD);
}

/**
* @tc.name: ConvertToText001
* @tc.desc: PasteDataRecord: ConvertToText
* @tc.type: FUNC
* @tc.require: AR000HEECD
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, ConvertToText001, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto record = PasteboardClient::GetInstance()->CreateHtmlTextRecord(htmlText);
    ASSERT_TRUE(record != nullptr);
    auto text = record->ConvertToText();
    EXPECT_EQ(text, htmlText);
}

/**
* @tc.name: ShareOptionToString001
* @tc.desc: PasteData: ShareOptionToString
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, ShareOptionToString001, TestSize.Level0)
{
    std::string shareOption1;
    PasteData::ShareOptionToString(ShareOption::InApp, shareOption1);
    ASSERT_TRUE(shareOption1 == "InAPP");
    std::string shareOption2;
    PasteData::ShareOptionToString(ShareOption::LocalDevice, shareOption2);
    ASSERT_TRUE(shareOption2 == "LocalDevice");
    std::string shareOption3;
    PasteData::ShareOptionToString(ShareOption::CrossDevice, shareOption3);
    ASSERT_TRUE(shareOption3 == "CrossDevice");
}

/**
* @tc.name: SetInvalid001
* @tc.desc: PasteData: SetInvalid001
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, SetInvalid001, TestSize.Level0)
{
    bool result = true;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->SetInvalid();
    result = pasteData->IsValid();
    ASSERT_FALSE(result);
}

/**
* @tc.name: SetLocalOnly001
* @tc.desc: PasteData: SetLocalOnly
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, SetLocalOnly001, TestSize.Level0)
{
    bool result = false;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->SetLocalOnly(true);
    result = pasteData->GetLocalOnly();
    ASSERT_TRUE(result);
}

/**
* @tc.name: SetAddition001
* @tc.desc: PasteData: SetAddition
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, SetAddition001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    size_t fileSize = 0;
    pasteData->SetAddition(PasteData::REMOTE_FILE_SIZE, AAFwk::Integer::Box(fileSize));
    AAFwk::WantParams additions;
    pasteData->SetAdditions(additions);
    ASSERT_TRUE(pasteData != nullptr);
}

/**
* @tc.name: SetRemote001
* @tc.desc: PasteData: SetRemote
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, SetRemote001, TestSize.Level0)
{
    bool isRemote = false;
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    isRemote = true;
    pasteData->SetRemote(isRemote);
    bool result = pasteData->IsRemote();
    ASSERT_TRUE(result);
}

/**
* @tc.name: SetOrginAuthority001
* @tc.desc: PasteData: SetOrginAuthority
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, SetOrginAuthority001, TestSize.Level0)
{
    std::string plainText = "plain text";
    std::string bundleName = "com.example.myapplication";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    pasteData->SetBundleName(bundleName);
    pasteData->SetOrginAuthority(bundleName);
    std::string getBundleName = pasteData->GetBundleName();
    std::string getOrginAuthority = pasteData->GetOrginAuthority();
    ASSERT_TRUE(getBundleName == bundleName);
    ASSERT_TRUE(getOrginAuthority == bundleName);
}

/**
* @tc.name: GetConvertUri001
* @tc.desc: PasteDataRecord: GetConvertUri
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, GetConvertUri001, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    std::string convertUri_ = "/mnt/hmdfs/";
    pasteDataRecord->SetConvertUri(convertUri_);
    std::string result = pasteDataRecord->GetConvertUri();
    ASSERT_TRUE(result == convertUri_);
    std::string newUriStr = "/mnt/hmdfs/test";
    pasteDataRecord->SetUri(std::make_shared<OHOS::Uri>(newUriStr));
    std::shared_ptr<Uri> uri = pasteDataRecord->GetUri();
    ASSERT_TRUE(uri != nullptr);
    std::shared_ptr<Uri> getOriginUri = pasteDataRecord->GetOrginUri();
    ASSERT_TRUE(getOriginUri != nullptr);
}

/**
* @tc.name: LoadSystemAbilityFail001
* @tc.desc: PasteDataRecord: LoadSystemAbilityFail
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, LoadSystemAbilityFail001, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    std::string mimeType = "image/jpg";
    arrayBuffer = { 1, 2, 3, 4, 6 };
    PasteboardClient::GetInstance()->LoadSystemAbilityFail();
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteDataRecord != nullptr);
}

/**
* @tc.name: SetInterval001
* @tc.desc: BlockObject: SetInterval
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, SetInterval001, TestSize.Level0)
{
    uint32_t POPUP_INTERVAL = 1;
    auto block = std::make_shared<BlockObject<std::shared_ptr<PasteData>>>(POPUP_INTERVAL);
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    block->SetValue(pasteData);
    block->SetInterval(POPUP_INTERVAL);
    auto value = block->GetValue();
    EXPECT_TRUE(value != nullptr);
}

/**
* @tc.name: ClipPlugin001
* @tc.desc: API_EXPORT: ClipPlugin
* @tc.type: FUNC
* @tc.require: DTS2023071915769
* @tc.author: z30043299
*/
HWTEST_F(PasteDataTest, ClipPlugin001, TestSize.Level0)
{
    std::string PLUGIN_NAME_VAL = "distributed_clip";
    auto release = [&PLUGIN_NAME_VAL, this](ClipPlugin *plugin) {
        ClipPlugin::DestroyPlugin(PLUGIN_NAME_VAL, plugin);
    };
    auto clipPlugin_ = std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME_VAL), release);
    ClipPlugin::Factory *factory = nullptr;
    auto result = ClipPlugin::RegCreator(PLUGIN_NAME_VAL, factory);
    EXPECT_TRUE(result);
    auto userId = 10000;
    auto events1 = clipPlugin_->GetTopEvents(1, userId);
    EXPECT_TRUE(events1.size() == 0);
    auto events2 = clipPlugin_->GetTopEvents(1);
    EXPECT_TRUE(events2.size() == 0);
    clipPlugin_->Clear();
    clipPlugin_->Clear(userId);
}
} // namespace OHOS::MiscServices