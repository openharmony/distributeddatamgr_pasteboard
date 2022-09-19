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

class UriHandlerMock : public UriHandler {
public:
    UriHandlerMock() = default;
    virtual ~UriHandlerMock() = default;

    MOCK_METHOD1(ToUri, std::string(int32_t fd));
    MOCK_METHOD1(ToFd, int32_t(const std::string &uri));
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

    //mock
    UriHandlerMock mock;
    std::string mockUri = "/mnt/hmdfs/100/account/merge_view/services/psteboard_service/.share/xxx.txt";
    EXPECT_CALL(mock, ToUri(_)).WillRepeatedly(Return(mockUri));
    EXPECT_CALL(mock, ToFd(_)).WillRepeatedly(Return(10));
    EXPECT_CALL(mock, IsFile(_)).WillRepeatedly(Return(true));

    data.AddRecord(record);
    MessageParcel parcel;
    data.WriteUriFd(parcel, mock, 0xff);
    bool result = data.ReadUriFd(parcel, mock);
    EXPECT_TRUE(result);
    EXPECT_EQ(mockUri, data.GetPrimaryUri()->ToString());
    data.ReplaceShareUri(200);
    std::string mockUri2 = "/mnt/hmdfs/200/account/merge_view/services/psteboard_service/.share/xxx.txt";
    EXPECT_EQ(mockUri2, data.GetPrimaryUri()->ToString());
}
} // namespace OHOS::MiscServices