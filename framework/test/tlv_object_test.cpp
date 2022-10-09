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

#include "tlv_object.h"

#include <gtest/gtest.h>

#include "pasteboard_client.h"
#include "pasteboard_hilog.h"
namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace OHOS::AAFwk;
class TLVObjectTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::shared_ptr<PasteDataRecord> GenRecord(std::uint32_t index);
};

void TLVObjectTest::SetUpTestCase(void)
{
}

void TLVObjectTest::TearDownTestCase(void)
{
}

void TLVObjectTest::SetUp(void)
{
}

void TLVObjectTest::TearDown(void)
{
}

std::shared_ptr<PasteDataRecord> TLVObjectTest::GenRecord(std::uint32_t index)
{
    std::string indexStr = "";
    auto plainText = std::make_shared<std::string>("hello" + indexStr);
    auto htmlText = std::make_shared<std::string>("<span>hello" + indexStr + "</span>");
    auto uri = std::make_shared<OHOS::Uri>("dataability://hello" + indexStr + ".txt");
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    want->SetParam(key, id);

    PasteDataRecord::Builder builder(MIMETYPE_TEXT_HTML);
    auto record1 = builder.SetPlainText(plainText).SetHtmlText(htmlText).SetUri(uri).SetWant(want).Build();
    return record1;
}

/**
* @tc.name: TLVOjbectTest001
* @tc.desc: test tlv coder.
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: baoyayong
*/
HWTEST_F(TLVObjectTest, TLVOjbectTest001, TestSize.Level0)
{
    PasteData pasteData1;
    for (uint32_t i = 0; i < 10; ++i) {
        pasteData1.AddRecord(TLVObjectTest::GenRecord(i));
    }

    std::vector<uint8_t> buffer;
    auto ret = pasteData1.Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1.Count());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1.GetRecordCount());

    for (uint32_t i = 0; i < 10; ++i) {
        auto record2 = pasteData2.GetRecordAt(i);
        auto record1 = pasteData1.GetRecordAt(i);
        EXPECT_EQ(*(record2->GetHtmlText()), *(record1->GetHtmlText()));
        EXPECT_EQ(*(record2->GetPlainText()), *(record1->GetPlainText()));
        EXPECT_TRUE(record2->GetUri()->ToString() == record1->GetUri()->ToString());
        EXPECT_EQ(record2->GetWant()->OperationEquals(*(record1->GetWant())), true);
    }
}

/**
* @tc.name: TLVOjbectTest002
* @tc.desc: test tlv coder.
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: baoyayong
*/
HWTEST_F(TLVObjectTest, TLVOjbectTest002, TestSize.Level0)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    auto pasteData1 = PasteboardClient::GetInstance()->CreateWantData(std::make_shared<Want>(wantIn));

    std::vector<uint8_t> buffer;
    auto ret = pasteData1->Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1->Count());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1->GetRecordCount());

    auto record2 = pasteData2.GetRecordAt(0);
    auto record1 = pasteData1->GetRecordAt(0);
    EXPECT_EQ(record2->GetWant()->OperationEquals(*(record1->GetWant())), true);
}

/**
* @tc.name: TLVOjbectTest003
* @tc.desc: test tlv coder map.
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: baoyayong
*/
HWTEST_F(TLVObjectTest, TLVOjbectTest003, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData1 = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType1 = "image/jpg";
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    customData->AddItemData(mimeType1, arrayBuffer);
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_PLAIN);
    std::shared_ptr<PasteDataRecord> pasteDataRecord = builder.SetCustomData(customData).Build();
    pasteData1->AddRecord(pasteDataRecord);

    std::vector<uint8_t> buffer;
    auto ret = pasteData1->Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1->Count());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1->GetRecordCount());
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1->GetRecordCount());

    auto record1 = pasteData1->GetRecordAt(0);
    auto record2 = pasteData2.GetRecordAt(0);
    ASSERT_TRUE(record2 != nullptr);
    auto custom2 = record2->GetCustomData();
    auto custom1 = record1->GetCustomData();
    ASSERT_TRUE(custom1 != nullptr && custom2 != nullptr);
    EXPECT_EQ(custom2->GetItemData().size(), custom1->GetItemData().size());
}
} // namespace OHOS::MiscServices