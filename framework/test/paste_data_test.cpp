/*
* Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "pasteboard_client.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::AAFwk;
using namespace OHOS::Media;
constexpr const std::uint32_t MAX_RECORD_NUM = 512;
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

/**
* @tc.name: WriteUriFdReadUriFd001
* @tc.desc: PasteData: WriteUriFd ReadUriFd
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, WriteUriFdReadUriFd001, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = "/data/PasteboardFrameworkTest";
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();
    ASSERT_TRUE(record->NeedFd(true));
    data.AddRecord(record);

    MessageParcel parcel;
    bool result = data.WriteUriFd(parcel, true);
    EXPECT_TRUE(result);
    result = data.ReadUriFd(parcel, true);
    EXPECT_TRUE(result);
    ASSERT_TRUE(data.GetPrimaryUri() != nullptr);
    EXPECT_TRUE(uriStr != data.GetPrimaryUri()->ToString());
}

/**
* @tc.name: WriteUriFdReadUriFd002
* @tc.desc: PasteData: WriteUriFd ReadUriFd
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, WriteUriFdReadUriFd002, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = "/data/PasteboardFrameworkTest";
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();
    ASSERT_FALSE(record->NeedFd(false));
    data.AddRecord(record);

    MessageParcel parcel;
    bool result = data.WriteUriFd(parcel, false);
    EXPECT_TRUE(result);
    result = data.ReadUriFd(parcel, false);
    EXPECT_TRUE(result);
    ASSERT_TRUE(data.GetPrimaryUri() != nullptr);
    EXPECT_TRUE(uriStr == data.GetPrimaryUri()->ToString());
}

/**
* @tc.name: WriteUriFdReadUriFd003
* @tc.desc: PasteData: WriteUriFd ReadUriFd
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, WriteUriFdReadUriFd003, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = "/data/PasteboardFrameworkTest";
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();
    data.AddRecord(record);

    MessageParcel parcel;
    bool result = data.WriteUriFd(parcel, true);
    EXPECT_TRUE(result);
    result = data.ReadUriFd(parcel, false);
    EXPECT_TRUE(result);
    ASSERT_TRUE(data.GetPrimaryUri() != nullptr);
    EXPECT_TRUE(uriStr != data.GetPrimaryUri()->ToString());
}

/**
* @tc.name: WriteUriFdReadUriFd004
* @tc.desc: PasteData: WriteUriFd ReadUriFd
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, WriteUriFdReadUriFd004, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = "/data/PasteboardFrameworkTest";
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();
    data.AddRecord(record);

    MessageParcel parcel;
    bool result = data.WriteUriFd(parcel, false);
    EXPECT_TRUE(result);
    result = data.ReadUriFd(parcel, true);
    EXPECT_TRUE(result);
    ASSERT_TRUE(data.GetPrimaryUri() != nullptr);
    EXPECT_TRUE(uriStr == data.GetPrimaryUri()->ToString());
}

/**
* @tc.name: WriteUriFdReadUriFd005
* @tc.desc: PasteData: WriteUriFd ReadUriFd
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, WriteUriFdReadUriFd005, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = "/data/PasteboardFrameworkTest";
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();
    data.AddRecord(record);

    MessageParcel parcel;
    bool result = data.WriteUriFd(parcel, true);
    EXPECT_TRUE(result);
    result = data.ReadUriFd(parcel, false);
    EXPECT_TRUE(result);
    ASSERT_TRUE(record->NeedFd(false));
    result = data.WriteUriFd(parcel, false);
    EXPECT_TRUE(result);
    result = data.ReadUriFd(parcel, true);
    EXPECT_TRUE(result);
    ASSERT_TRUE(data.GetPrimaryUri() != nullptr);
    EXPECT_TRUE(uriStr != data.GetPrimaryUri()->ToString());
}

/**
* @tc.name: AddRecord001
* @tc.desc: AddRecord: records_.size() > MAX_RECORD_NUM
* @tc.type: FUNC
* @tc.require: AR000HEECD
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, AddRecord001, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto record = PasteboardClient::GetInstance()->CreateHtmlTextRecord(htmlText);
    ASSERT_TRUE(record != nullptr);
    auto pasteData = std::make_shared<PasteData>();
    for (size_t i = 0; i < MAX_RECORD_NUM; i++) {
        pasteData->AddRecord(*record);
    }
    ASSERT_EQ(pasteData->GetRecordCount(), MAX_RECORD_NUM);
    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    record = PasteboardClient::GetInstance()->CreateWantRecord(want);
    ASSERT_TRUE(record != nullptr);
    pasteData->AddRecord(*record);
    ASSERT_EQ(pasteData->GetRecordCount(), MAX_RECORD_NUM);
    auto record0 = pasteData->GetRecordAt(0);
    ASSERT_TRUE(record0 != nullptr);
    auto newWant = record0->GetWant();
    ASSERT_TRUE(newWant != nullptr);
    int32_t defaultValue = 333;
    EXPECT_EQ(newWant->GetIntParam(key, defaultValue), id);
    auto lastRecord = pasteData->GetRecordAt(MAX_RECORD_NUM - 1);
    ASSERT_TRUE(lastRecord != nullptr);
    auto newHtml = lastRecord->GetHtmlText();
    ASSERT_TRUE(newHtml != nullptr);
    EXPECT_EQ(*newHtml, htmlText);
}

/**
* @tc.name: PartGetInterfaceAbnormalTest001
* @tc.desc: GetPrimaryPixelMap、GetPrimaryText、GetPrimaryUri、GetPrimaryWant
* @tc.type: FUNC
* @tc.require: AR000HEECD
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, PartGetInterfaceAbnormalTest001, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto pasteData = PasteboardClient::GetInstance()->CreateHtmlData(htmlText);
    ASSERT_TRUE(pasteData != nullptr);
    auto html = pasteData->GetPrimaryHtml();
    EXPECT_TRUE(html != nullptr);
    auto want = pasteData->GetPrimaryWant();
    EXPECT_TRUE(want == nullptr);
    auto pixelMap = pasteData->GetPrimaryPixelMap();
    EXPECT_TRUE(pixelMap == nullptr);
    auto text = pasteData->GetPrimaryText();
    EXPECT_TRUE(text == nullptr);
    auto uri = pasteData->GetPrimaryUri();
    EXPECT_TRUE(uri == nullptr);
}

/**
* @tc.name: PartGetInterfaceAbnormalTest002
* @tc.desc: GetPrimaryMimeType、GetPrimaryHtml、GetRecordAt
* @tc.type: FUNC
* @tc.require: AR000HEECD
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, PartGetInterfaceAbnormalTest002, TestSize.Level0)
{
    auto pasteData = std::make_shared<PasteData>();
    auto html = pasteData->GetPrimaryHtml();
    EXPECT_TRUE(html == nullptr);
    auto mimeType = pasteData->GetPrimaryMimeType();
    EXPECT_TRUE(mimeType == nullptr);
    auto Record0 = pasteData->GetRecordAt(0);
    EXPECT_TRUE(Record0 == nullptr);
}

/**
* @tc.name: RemoveRecordAt001
* @tc.desc: RemoveRecordAt异常测试
* @tc.type: FUNC
* @tc.require: AR000HEECD
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, RemoveRecordAt001, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto record = PasteboardClient::GetInstance()->CreateHtmlTextRecord(htmlText);
    ASSERT_TRUE(record != nullptr);
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddRecord(*record);
    EXPECT_FALSE(pasteData->RemoveRecordAt(1));
}

/**
* @tc.name: ReplaceRecordAt001
* @tc.desc: ReplaceRecordAt异常测试
* @tc.type: FUNC
* @tc.require: AR000HEECD
* @tc.author: chenyu
*/
HWTEST_F(PasteDataTest, ReplaceRecordAt001, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto record = PasteboardClient::GetInstance()->CreateHtmlTextRecord(htmlText);
    ASSERT_TRUE(record != nullptr);
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddRecord(*record);
    EXPECT_FALSE(pasteData->ReplaceRecordAt(1, record));
    EXPECT_FALSE(pasteData->HasMimeType(MIMETYPE_TEXT_WANT));
}
} // namespace OHOS::MiscServices