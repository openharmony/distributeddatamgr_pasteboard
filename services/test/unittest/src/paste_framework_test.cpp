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

#include <gtest/gtest.h>

#include "pasteboard_client.h"
#include "serializable/tlv_object.h"
#include "singleton.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
class PasteboardFrameworkTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardFrameworkTest::SetUpTestCase(void)
{
}

void PasteboardFrameworkTest::TearDownTestCase(void)
{
}

void PasteboardFrameworkTest::SetUp(void)
{
}

void PasteboardFrameworkTest::TearDown(void)
{
}
/**
* @tc.name: TLVOjbectTest001
* @tc.desc: test tlv coder.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardFrameworkTest, TLVOjbectTest001, TestSize.Level0)
{
    auto plainText = std::make_shared<std::string>("hello");
    auto htmlText = std::make_shared<std::string>("<span>hello</span>");
    auto uri = std::make_shared<OHOS::Uri>("dataability://hello.txt");
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_HTML);
    auto record1 = builder.SetPlainText(plainText).SetHtmlText(htmlText).SetUri(uri).Build();

    PasteData pasteData1;
    pasteData1.AddRecord(record1);
    std::vector<uint8_t> buffer;
    auto ret = pasteData1.Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1.Count());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1.GetRecordCount());
    auto record2 = pasteData2.GetRecordAt(0);
    EXPECT_EQ(*(record2->GetHtmlText()), *(record1->GetHtmlText()));
    EXPECT_EQ(*(record2->GetPlainText()), *(record1->GetPlainText()));
    EXPECT_EQ(record2->GetUri()->ToString(), record1->GetUri()->ToString());
}
} // namespace OHOS::MiscServices