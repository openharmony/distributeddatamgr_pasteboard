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

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::AAFwk;
using namespace OHOS::Media;
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
* @tc.author: baoyayong
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
* @tc.name: WriteFdReadFd001
* @tc.desc: PasteDataRecord: WriteFd ReadFd NeedFd
* @tc.type: FUNC
* @tc.require:AR000H5I1D
* @tc.author: baoyayong
*/
HWTEST_F(PasteDataTest, WriteFdReadFd001, TestSize.Level0)
{
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = "/data/PasteboardFrameworkTest";
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();

    ASSERT_TRUE(record->NeedFd(true));

    MessageParcel parcel;
    bool result = record->WriteFd(parcel, true);
    EXPECT_TRUE(result);
    result = record->ReadFd(parcel, true);
    EXPECT_TRUE(result);
    ASSERT_TRUE(record->GetUri() != nullptr);
    EXPECT_TRUE(uriStr != record->GetUri()->ToString());
}
} // namespace OHOS::MiscServices