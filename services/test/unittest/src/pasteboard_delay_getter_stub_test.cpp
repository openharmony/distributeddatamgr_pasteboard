/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ipc_skeleton.h"
#include "message_parcel_warp.h"
#include "pasteboard_hilog.h"
#include "pasteboard_delay_getter_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::MiscServices;
namespace OHOS {
class PasteboardDelayStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class PasteboardDelayGetterStubTest : public PasteboardDelayGetterStub {
public:
    PasteboardDelayGetterStubTest() = default;
    virtual ~PasteboardDelayGetterStubTest() = default;
    MOCK_METHOD(void, GetPasteData, (const std::string &type, PasteData &data), (override));
    MOCK_METHOD(void, GetUnifiedData, (const std::string &type, UDMF::UnifiedData &data), (override));
};

void PasteboardDelayStubTest::SetUpTestCase(void) {}

void PasteboardDelayStubTest::TearDownTestCase(void) {}

void PasteboardDelayStubTest::SetUp(void) {}

void PasteboardDelayStubTest::TearDown(void) {}

class PasteboardDelayStubInterface : public MessageParcel {
public:
    PasteboardDelayStubInterface(){};
    virtual ~PasteboardDelayStubInterface(){};
    virtual std::u16string ReadInterfaceToken() = 0;
    virtual bool Encode(std::vector<uint8_t> &buffer) const = 0;
    virtual bool WriteRawData(MessageParcel &parcelPata, const void *data, size_t size) = 0;
    virtual bool WriteInt64(int64_t value) = 0;
};

class PasteboardDelayStubInterfaceMock : public PasteboardDelayStubInterface {
public:
    PasteboardDelayStubInterfaceMock();
    ~PasteboardDelayStubInterfaceMock() override;
    MOCK_METHOD0(ReadInterfaceToken, std::u16string());
    MOCK_CONST_METHOD1(Encode, bool(std::vector<uint8_t> &buffer));
    MOCK_METHOD3(WriteRawData, bool(MessageParcel &parcelPata, const void *data, size_t size));
    MOCK_METHOD1(WriteInt64, bool(int64_t value));
};

static void *g_interface = nullptr;

PasteboardDelayStubInterfaceMock::PasteboardDelayStubInterfaceMock()
{
    g_interface = reinterpret_cast<void *>(this);
}

PasteboardDelayStubInterfaceMock::~PasteboardDelayStubInterfaceMock()
{
    g_interface = nullptr;
}

static PasteboardDelayStubInterface *GetPasteboardDelayStubInterface()
{
    return reinterpret_cast<PasteboardDelayStubInterface *>(g_interface);
}

extern "C" {
std::u16string MessageParcel::ReadInterfaceToken()
{
    PasteboardDelayStubInterface *interface = GetPasteboardDelayStubInterface();
    if (interface == nullptr) {
        return u"";
    }
    return interface->ReadInterfaceToken();
}

bool TLVWriteable::Encode(std::vector<uint8_t> &buffer) const
{
    PasteboardDelayStubInterface *interface = GetPasteboardDelayStubInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->Encode(buffer);
}

bool Parcel::WriteInt64(int64_t data)
{
    PasteboardDelayStubInterface *interface = GetPasteboardDelayStubInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->WriteInt64(data);
}
}

/* *
 * @tc.name: OnRemoteRequestTest001
 * @tc.desc: remoteDescriptor != localDescriptor
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDelayStubTest, OnRemoteRequestTest001, TestSize.Level0)
{
    auto tempDelayGetter = std::make_shared<PasteboardDelayGetterStubTest>();
    NiceMock<PasteboardDelayStubInterfaceMock> mock;
    EXPECT_CALL(mock, ReadInterfaceToken()).WillOnce(Return(u"testing"));
    uint32_t code = IPasteboardDelayGetter::TRANS_HEAD;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = tempDelayGetter->OnRemoteRequest(code, data, reply, option);
    ASSERT_EQ(ret, -1);
}

/* *
 * @tc.name: OnRemoteRequestTest002
 * @tc.desc: code >= TRANS_BUTT
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDelayStubTest, OnRemoteRequestTest002, TestSize.Level0)
{
    auto tempDelayGetter = std::make_shared<PasteboardDelayGetterStubTest>();
    uint32_t code = IPasteboardDelayGetter::TRANS_BUTT;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    int32_t ret = tempDelayGetter->OnRemoteRequest(code, data, reply, option);
    ASSERT_EQ(ret, -1);
}

/* *
 * @tc.name: OnGetPasteDataTest001
 * @tc.desc: pasteData.Encode(pasteDataTlv) return error
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDelayStubTest, OnGetPasteDataTest001, TestSize.Level0)
{
    auto tempDelayGetter = std::make_shared<PasteboardDelayGetterStubTest>();
    MessageParcel data;
    MessageParcel reply;
    NiceMock<PasteboardDelayStubInterfaceMock> mock;
    EXPECT_CALL(mock, Encode(testing::_)).WillOnce(Return(false));
    int32_t ret = tempDelayGetter->OnGetPasteData(data, reply);
    ASSERT_EQ(ret, ERR_INVALID_VALUE);
}

/* *
 * @tc.name: OnGetPasteDataTest002
 * @tc.desc: WriteInt64 return error
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDelayStubTest, OnGetPasteDataTest002, TestSize.Level0)
{
    auto tempDelayGetter = std::make_shared<PasteboardDelayGetterStubTest>();
    MessageParcel data;
    MessageParcel reply;
    NiceMock<PasteboardDelayStubInterfaceMock> mock;
    EXPECT_CALL(mock, Encode(testing::_)).WillOnce(Return(true));
    EXPECT_CALL(mock, WriteInt64(testing::_)).WillRepeatedly(Return(false));
    int32_t ret = tempDelayGetter->OnGetPasteData(data, reply);
    ASSERT_EQ(ret, ERR_INVALID_VALUE);
}

/* *
 * @tc.name: OnGetPasteDataTest003
 * @tc.desc: reply.WriteInt64 return error
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDelayStubTest, OnGetPasteDataTest003, TestSize.Level0)
{
    auto tempDelayGetter = std::make_shared<PasteboardDelayGetterStubTest>();
    MessageParcel data;
    MessageParcel reply;
    NiceMock<PasteboardDelayStubInterfaceMock> mock;
    EXPECT_CALL(mock, Encode(testing::_)).WillOnce(Return(true));
    EXPECT_CALL(mock, WriteInt64(testing::_)).WillRepeatedly(Return(true));
    EXPECT_CALL(mock, WriteRawData(testing::_, testing::_, testing::_)).WillRepeatedly(Return(false));
    int32_t ret = tempDelayGetter->OnGetPasteData(data, reply);
    ASSERT_EQ(ret, ERR_INVALID_VALUE);
}

/* *
 * @tc.name: OnGetUnifiedDataTest001
 * @tc.desc: OnGetUnifiedData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardDelayStubTest, OnGetUnifiedDataTest001, TestSize.Level0)
{
    auto tempDelayGetter = std::make_shared<PasteboardDelayGetterStubTest>();
    MessageParcel data;
    MessageParcel reply;
    int32_t ret = tempDelayGetter->OnGetUnifiedData(data, reply);
    ASSERT_EQ(ret, ERR_OK);
}
} // namespace OHOS