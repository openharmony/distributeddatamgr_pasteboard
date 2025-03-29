/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "message_parcel.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_service_proxy.h"
#include "ipc_object_stub.h"
#include "tlv_readable.h"
#include "tlv_writeable.h"
#include "iremote_object.h"
#include "message_parcel_warp.h"
#include "pasteboard_client.h"
#include "iremote_broker.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MiscServices;
namespace OHOS {
constexpr uint8_t TEST_DATA = 0x02;

class PasteboardServiceProxyInterface {
public:
    PasteboardServiceProxyInterface() {};
    virtual ~PasteboardServiceProxyInterface() {};

    virtual bool WriteInterfaceToken(std::u16string name) = 0;
    virtual bool WriteUint32(uint32_t value) = 0;
    virtual bool Encode(std::vector<uint8_t> &buffer) = 0;
    virtual bool Decode(const std::vector<std::uint8_t> &buffer) = 0;
    virtual bool WriteInt64(int64_t value) = 0;
    virtual bool WriteString(const std::string &value) = 0;
    virtual int32_t ReadInt32() = 0;
    virtual bool WriteUnpadBuffer(const void *data, size_t size) = 0;
    virtual int64_t ReadInt64() = 0;
    virtual bool WriteRemoteObject(const Parcelable *object) = 0;
    virtual uint8_t *ReadUnpadBuffer(size_t length) = 0;
};

class PasteboardServiceProxyMock : public PasteboardServiceProxyInterface {
public:
    PasteboardServiceProxyMock();
    ~PasteboardServiceProxyMock() override;

    MOCK_METHOD1(WriteInterfaceToken, bool(std::u16string name));
    MOCK_METHOD1(WriteUint32, bool(uint32_t value));
    MOCK_METHOD1(Encode, bool(std::vector<uint8_t> &buffer));
    MOCK_METHOD1(Decode, bool(const std::vector<std::uint8_t> &buffer));
    MOCK_METHOD1(WriteInt64, bool(int64_t value));
    MOCK_METHOD1(WriteString, bool(const std::string &value));
    MOCK_METHOD0(ReadInt32, int32_t());
    MOCK_METHOD2(WriteUnpadBuffer, bool(const void *data, size_t size));
    MOCK_METHOD0(ReadInt64, int64_t());
    MOCK_METHOD1(WriteRemoteObject, bool(const Parcelable *object));
    MOCK_METHOD1(ReadUnpadBuffer, uint8_t *(size_t length));
};

static void *g_interface = nullptr;

PasteboardServiceProxyMock::PasteboardServiceProxyMock()
{
    g_interface = reinterpret_cast<void *>(this);
}

PasteboardServiceProxyMock::~PasteboardServiceProxyMock()
{
    g_interface = nullptr;
}

static PasteboardServiceProxyInterface *GetPasteboardServiceProxyInterface()
{
    return reinterpret_cast<PasteboardServiceProxyInterface *>(g_interface);
}

extern "C" {
    bool MessageParcel::WriteInterfaceToken(std::u16string name)
    {
        if (GetPasteboardServiceProxyInterface() == nullptr ) {
            return false;
        }
        return GetPasteboardServiceProxyInterface()->WriteInterfaceToken(name);
    }
    bool TLVWriteable::Encode(std::vector<uint8_t> &buffer) const
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return false;
        }
        if (g_interface) {
            buffer.push_back(TEST_DATA);
        }
        return GetPasteboardServiceProxyInterface()->Encode(buffer);
    }
    bool TLVReadable::Decode(const std::vector<std::uint8_t> &buffer)
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return false;
        }
        return GetPasteboardServiceProxyInterface()->Decode(buffer);
    }
    bool Parcel::WriteUint32(uint32_t value)
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return false;
        }
        return GetPasteboardServiceProxyInterface()->WriteUint32(value);
    }
    bool Parcel::WriteInt64(int64_t value)
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return false;
        }
        return GetPasteboardServiceProxyInterface()->WriteInt64(value);
    }
    int32_t Parcel::ReadInt32()
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return 0;
        }
        return GetPasteboardServiceProxyInterface()->ReadInt32();
    }
    bool Parcel::WriteString(const std::string &value)
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return false;
        }
        return GetPasteboardServiceProxyInterface()->WriteString(value);
    }
    int64_t Parcel::ReadInt64()
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return 0;
        }
        return GetPasteboardServiceProxyInterface()->ReadInt64();
    }
    bool Parcel::WriteUnpadBuffer(const void *data, size_t size)
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return false;
        }
        return GetPasteboardServiceProxyInterface()->WriteUnpadBuffer(data, size);
    }
    bool Parcel::WriteRemoteObject(const Parcelable *object)
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return false;
        }
        return GetPasteboardServiceProxyInterface()->WriteRemoteObject(object);
    }
    const uint8_t *Parcel::ReadUnpadBuffer(size_t length)
    {
        if (GetPasteboardServiceProxyInterface() == nullptr) {
            return nullptr;
        }
        return GetPasteboardServiceProxyInterface()->ReadUnpadBuffer(length);
    }
};
namespace MiscServices {
static int g_sendRequestCode = 0;
class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() : IRemoteObject(u"test_descripter") {};
    ~MockIRemoteObject() = default;
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override {
        return g_sendRequestCode;
    }
    int32_t GetObjectRefCount() override {
        return 0;
    }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override {
        return false;
    }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override {
        return false;
    }
    int Dump(int fd, const std::vector<std::u16string> &args) {
        return 0;
    }
};

class MockIPasteboardDelayGetter : public IPasteboardDelayGetter {
public:
    MockIPasteboardDelayGetter() = default;
    ~MockIPasteboardDelayGetter() = default;
    MOCK_METHOD(void, GetPasteData, (const std::string &type, PasteData &data), (override));
    MOCK_METHOD(void, GetUnifiedData, (const std::string &type, UDMF::UnifiedData &data), (override));
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, (), (override));
};

class MockIPasteboardEntryGetter : public IPasteboardEntryGetter {
public:
    MockIPasteboardEntryGetter() = default;
    ~MockIPasteboardEntryGetter() = default;
    MOCK_METHOD(int32_t, GetRecordValueByType, (uint32_t recordId, PasteDataEntry &value), (override));
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, (), (override));
};

class PasteboardServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardServiceProxyTest::SetUpTestCase(void) { }

void PasteboardServiceProxyTest::TearDownTestCase(void) { }

void PasteboardServiceProxyTest::SetUp(void) { }

void PasteboardServiceProxyTest::TearDown(void) { }

/**
 * @tc.name: GetRecordValueByTypeTest001
 * @tc.desc: Verify the GetRecordValueByType function when WriteInterfaceToken function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetRecordValueByTypeTest001, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    uint32_t dataId = 1;
    uint32_t recordId = 1;
    PasteDataEntry value;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(false));

    int32_t result = pasteboardServiceProxy->GetRecordValueByType(dataId, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest002
 * @tc.desc: Verify the GetRecordValueByType function when WriteUint32 function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetRecordValueByTypeTest002, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    uint32_t dataId = 1;
    uint32_t recordId = 1;
    PasteDataEntry value;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteUint32).WillOnce(testing::Return(false)).WillRepeatedly(testing::Return(true));

    int32_t result = pasteboardServiceProxy->GetRecordValueByType(dataId, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest003
 * @tc.desc: Verify the GetRecordValueByType function when WriteUint32 function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetRecordValueByTypeTest003, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    uint32_t dataId = 1;
    uint32_t recordId = 1;
    PasteDataEntry value;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteUint32).WillOnce(testing::Return(true)).WillOnce(testing::Return(false));

    int32_t result = pasteboardServiceProxy->GetRecordValueByType(dataId, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest004
 * @tc.desc: Verify the GetRecordValueByType function when Encode function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetRecordValueByTypeTest004, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    uint32_t dataId = 1;
    uint32_t recordId = 1;
    PasteDataEntry value;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteUint32).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(false));

    int32_t result = pasteboardServiceProxy->GetRecordValueByType(dataId, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest005
 * @tc.desc: Verify the GetRecordValueByType function when WriteInt64 function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetRecordValueByTypeTest005, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    uint32_t dataId = 1;
    uint32_t recordId = 1;
    PasteDataEntry value;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteUint32).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillOnce(testing::Return(false));

    int32_t result = pasteboardServiceProxy->GetRecordValueByType(dataId, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest006
 * @tc.desc: Verify the GetRecordValueByType function when WriteInt64 function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetRecordValueByTypeTest006, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    uint32_t dataId = 1;
    uint32_t recordId = 1;
    PasteDataEntry value;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteUint32).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillOnce(testing::Return(true)).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(false));

    int32_t result = pasteboardServiceProxy->GetRecordValueByType(dataId, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest007
 * @tc.desc: Verify the GetRecordValueByType function when Decode function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetRecordValueByTypeTest007, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new MockIRemoteObject();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    uint32_t dataId = 1;
    uint32_t recordId = 1;
    PasteDataEntry value;
    uint32_t code = 0;
    g_sendRequestCode = code;
    std::vector<uint8_t> mockData = {0x01, 0x02, 0x03};
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteUint32).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, ReadInt64).WillRepeatedly(testing::Return(3));
    EXPECT_CALL(mock, ReadUnpadBuffer).WillRepeatedly(testing::Return(mockData.data()));
    EXPECT_CALL(mock, Decode).WillRepeatedly(testing::Return(false));

    int32_t result = pasteboardServiceProxy->GetRecordValueByType(dataId, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest008
 * @tc.desc: Verify the GetRecordValueByType function when ReadInt32 function return 1
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetRecordValueByTypeTest008, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new MockIRemoteObject();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    uint32_t dataId = 1;
    uint32_t recordId = 1;
    PasteDataEntry value;
    uint32_t code = 0;
    g_sendRequestCode = code;
    std::vector<uint8_t> mockData = {0x01, 0x02, 0x03};
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteUint32).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, ReadInt64).WillRepeatedly(testing::Return(3));
    EXPECT_CALL(mock, ReadUnpadBuffer).WillRepeatedly(testing::Return(mockData.data()));
    EXPECT_CALL(mock, Decode).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, ReadInt32).WillRepeatedly(testing::Return(1));

    int32_t result = pasteboardServiceProxy->GetRecordValueByType(dataId, recordId, value);
    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: GetPasteDataTest001
 * @tc.desc: Verify the GetPasteData function when WriteInterfaceToken function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetPasteDataTest001, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    int32_t syncTime = 1;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(false));

    int32_t result = pasteboardServiceProxy->GetPasteData(pasteData, syncTime);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: GetPasteDataTest002
 * @tc.desc: Verify the GetPasteData function when WriteString function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetPasteDataTest002, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    int32_t syncTime = 1;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteString).WillOnce(testing::Return(false));

    int32_t result = pasteboardServiceProxy->GetPasteData(pasteData, syncTime);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: GetPasteDataTest003
 * @tc.desc: Verify the GetPasteData function when SendRequest function return 1
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetPasteDataTest003, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new MockIRemoteObject();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    int32_t syncTime = 1;
    NiceMock<PasteboardServiceProxyMock> mock;
    constexpr int MAX_TRANSACTION_ID = 0x00ffffff;
    uint32_t code = 1;
    g_sendRequestCode = code;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteString).WillOnce(testing::Return(true));

    int32_t result = pasteboardServiceProxy->GetPasteData(pasteData, syncTime);
    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: GetPasteDataTest004
 * @tc.desc: Verify the GetPasteData function when rawDataSize is 0
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetPasteDataTest004, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new MockIRemoteObject();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    int32_t syncTime = 1;
    NiceMock<PasteboardServiceProxyMock> mock;
    uint32_t code = 0;
    g_sendRequestCode = code;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteString).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, ReadInt64).WillOnce(testing::Return(0));

    int32_t result = pasteboardServiceProxy->GetPasteData(pasteData, syncTime);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR));
}

/**
 * @tc.name: GetPasteDataTest005
 * @tc.desc: Verify the GetPasteData function when Decode function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetPasteDataTest005, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new MockIRemoteObject();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    int32_t syncTime = 1;
    NiceMock<PasteboardServiceProxyMock> mock;
    uint32_t code = 0;
    g_sendRequestCode = code;
    std::vector<uint8_t> mockData = {0x01, 0x02, 0x03};

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteString).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, ReadInt64).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(mock, WriteInt64).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, ReadInt64).WillRepeatedly(testing::Return(3));
    EXPECT_CALL(mock, ReadUnpadBuffer).WillRepeatedly(testing::Return(mockData.data()));
    EXPECT_CALL(mock, Decode).WillRepeatedly(testing::Return(false));

    int32_t result = pasteboardServiceProxy->GetPasteData(pasteData, syncTime);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::DESERIALIZATION_ERROR));
}

/**
 * @tc.name: GetPasteDataTest006
 * @tc.desc: Verify the GetPasteData function when ReadInt32 function return 1
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, GetPasteDataTest006, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new MockIRemoteObject();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    int32_t syncTime = 1;
    NiceMock<PasteboardServiceProxyMock> mock;
    uint32_t code = 0;
    g_sendRequestCode = code;
    std::vector<uint8_t> mockData = {0x01, 0x02, 0x03};

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteString).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, ReadInt64).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(mock, WriteInt64).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, ReadInt64).WillRepeatedly(testing::Return(3));
    EXPECT_CALL(mock, ReadUnpadBuffer).WillRepeatedly(testing::Return(mockData.data()));
    EXPECT_CALL(mock, Decode).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, ReadInt32).WillRepeatedly(testing::Return(1));

    int32_t result = pasteboardServiceProxy->GetPasteData(pasteData, syncTime);
    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: SetPasteDataTest001
 * @tc.desc: Verify the SetPasteData function when WriteInterfaceToken function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, SetPasteDataTest001, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    sptr<IPasteboardEntryGetter> entryGetter = nullptr;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(false));

    int32_t result = pasteboardServiceProxy->SetPasteData(pasteData, delayGetter, entryGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: SetPasteDataTest002
 * @tc.desc: Verify the SetPasteData function when Encode function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, SetPasteDataTest002, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    pasteData.SetDelayData(true);
    pasteData.SetDelayRecord(true);
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    sptr<IPasteboardEntryGetter> entryGetter = nullptr;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(false));

    int32_t result = pasteboardServiceProxy->SetPasteData(pasteData, delayGetter, entryGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: SetPasteDataTest003
 * @tc.desc: Verify the SetPasteData function when WriteInt64 function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, SetPasteDataTest003, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    pasteData.SetDelayData(true);
    pasteData.SetDelayRecord(true);
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    sptr<IPasteboardEntryGetter> entryGetter = nullptr;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillOnce(testing::Return(false));

    int32_t result = pasteboardServiceProxy->SetPasteData(pasteData, delayGetter, entryGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: SetPasteDataTest004
 * @tc.desc: Verify the SetPasteData function when WriteRawData function return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, SetPasteDataTest004, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    pasteData.SetDelayData(true);
    pasteData.SetDelayRecord(true);
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    sptr<IPasteboardEntryGetter> entryGetter = nullptr;
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(false));

    int32_t result = pasteboardServiceProxy->SetPasteData(pasteData, delayGetter, entryGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: SetPasteDataTest005
 * @tc.desc: Verify the SetPasteData function when pasteData.IsDelayData() is true
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, SetPasteDataTest005, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    pasteData.SetDelayData(true);
    pasteData.SetDelayRecord(true);
    sptr<IPasteboardDelayGetter> delayGetter = new MockIPasteboardDelayGetter();
    sptr<IPasteboardEntryGetter> entryGetter = new MockIPasteboardEntryGetter();
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteRemoteObject).WillRepeatedly(testing::Return(false));

    int32_t result = pasteboardServiceProxy->SetPasteData(pasteData, delayGetter, entryGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: SetPasteDataTest006
 * @tc.desc: Verify the SetPasteData function when pasteData.IsDelayRecord() is true
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, SetPasteDataTest006, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new IPCObjectStub();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    pasteData.SetDelayData(false);
    pasteData.SetDelayRecord(true);
    sptr<IPasteboardDelayGetter> delayGetter = new MockIPasteboardDelayGetter();
    sptr<IPasteboardEntryGetter> entryGetter = new MockIPasteboardEntryGetter();
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteRemoteObject).WillRepeatedly(testing::Return(false));

    int32_t result = pasteboardServiceProxy->SetPasteData(pasteData, delayGetter, entryGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR));
}

/**
 * @tc.name: SetPasteDataTest007
 * @tc.desc: Verify the SetPasteData function when SendRequest function return 1
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, SetPasteDataTest007, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new MockIRemoteObject();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    pasteData.SetDelayData(false);
    pasteData.SetDelayRecord(false);
    uint32_t code = 1;
    g_sendRequestCode = code;
    sptr<IPasteboardDelayGetter> delayGetter = new MockIPasteboardDelayGetter();
    sptr<IPasteboardEntryGetter> entryGetter = new MockIPasteboardEntryGetter();
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteRemoteObject).WillRepeatedly(testing::Return(true));

    int32_t result = pasteboardServiceProxy->SetPasteData(pasteData, delayGetter, entryGetter);
    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: SetPasteDataTest008
 * @tc.desc: Verify the SetPasteData function when ReadInt32 function return 1
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceProxyTest, SetPasteDataTest008, TestSize.Level0)
{
    sptr<IRemoteObject> remoteObject = new MockIRemoteObject();
    sptr<PasteboardServiceProxy> pasteboardServiceProxy = new PasteboardServiceProxy(remoteObject);
    PasteData pasteData;
    pasteData.SetDelayData(false);
    pasteData.SetDelayRecord(false);
    uint32_t code = 0;
    g_sendRequestCode = code;
    sptr<IPasteboardDelayGetter> delayGetter = new MockIPasteboardDelayGetter();
    sptr<IPasteboardEntryGetter> entryGetter = new MockIPasteboardEntryGetter();
    NiceMock<PasteboardServiceProxyMock> mock;

    EXPECT_CALL(mock, WriteInterfaceToken).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, Encode).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, WriteInt64).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteUnpadBuffer).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, WriteRemoteObject).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, ReadInt32).WillRepeatedly(testing::Return(1));

    int32_t result = pasteboardServiceProxy->SetPasteData(pasteData, delayGetter, entryGetter);
    EXPECT_EQ(result, 1);
}
}
} // namespace OHOS::MiscServices