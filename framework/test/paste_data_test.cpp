/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "copy_uri_handler.h"
#include "paste_uri_handler.h"
#include "pasteboard_client.h"
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
* @tc.name: PasteDataMarshallingUnMarshalling001
* @tc.desc: PasteData: Marshalling unMarshalling
* @tc.type: FUNC
* @tc.require: AROOOH5R5G
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, PasteDataMarshallingUnMarshalling001, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(htmlText);
    Parcel parcel;
    auto ret = pasteData->Marshalling(parcel);
    ASSERT_TRUE(ret);
    std::shared_ptr<PasteData> pasteData1(pasteData->Unmarshalling(parcel));
    ASSERT_TRUE(pasteData1 != nullptr);
    auto html = pasteData1->GetPrimaryHtml();
    ASSERT_TRUE(html != nullptr);
    EXPECT_EQ(*html, htmlText);
}

/**
* @tc.name: PasteDataRecordMarshallingUnMarshalling001
* @tc.desc: PasteDataRecord: Marshalling unMarshalling
* @tc.type: FUNC
* @tc.require: AROOOH5R5G
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, PasteDataRecordMarshallingUnMarshalling001, TestSize.Level0)
{
    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    auto record = PasteboardClient::GetInstance()->CreateWantRecord(want);
    ASSERT_TRUE(record != nullptr);
    Parcel parcel;
    auto ret = record->Marshalling(parcel);
    ASSERT_TRUE(ret);
    std::shared_ptr<PasteDataRecord> record1(record->Unmarshalling(parcel));
    ASSERT_TRUE(record1 != nullptr);
    auto newWant = record->GetWant();
    ASSERT_TRUE(newWant != nullptr);
    int32_t defaultValue = 333;
    EXPECT_EQ(newWant->GetIntParam(key, defaultValue), id);
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
} // namespace OHOS::MiscServices