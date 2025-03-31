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

#include <fcntl.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "ipc_skeleton.h"
#include "pasteboard_error.h"
#include "pasteboard_service.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace std::chrono;
namespace OHOS {
namespace {
    const int32_t ACCOUNT_IDS_RANDOM = 1121;
    const uint32_t UINT32_ONE = 1;
    const int INT_ONE = 1;
    const uint8_t UINT8_ONE = 1;
    const int32_t INT32_NEGATIVE_NUMBER = -1;
    const int64_t INT64_NEGATIVE_NUMBER = -1;
    const uint32_t UINT32_EXCEPTION_APPID = 9999985;
    const int INT_THREETHREETHREE = 333;
    const uint32_t MAX_RECOGNITION_LENGTH = 1000;
    const int64_t MIN_ASHMEM_DATA_SIZE = 32 * 1024;
    const std::string RANDOM_STRING = "TEST_string_111";
    const std::string URI_STRING = "https://www.a.com/";
    const std::string HTML_STRING =
        R"(<!DOCTYPE html><html lang="en"><head><title>S</title></head><body><h1>H</h1></body></html>)";
}

class MyTestEntityRecognitionObserver : public IEntityRecognitionObserver {
    void OnRecognitionEvent(EntityType entityType, std::string &entity)
    {
        return;
    }
    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
};

class PasteboardServiceInterface {
public:
    PasteboardServiceInterface(){};
    virtual ~PasteboardServiceInterface(){};

    virtual OHOS::ErrCode QueryActiveOsAccountIds(std::vector<int32_t> &ids);
    virtual bool Decode(const std::vector<std::uint8_t> &buffer) = 0;
    virtual bool Encode(std::vector<uint8_t> &buffer) const = 0;
    virtual pid_t GetCallingUid() = 0;
    virtual uint32_t GetCallingTokenID() = 0;
    virtual uint64_t GetCallingFullTokenID() = 0;
    virtual pid_t GetCallingPid() = 0;
    virtual bool HasContent(const std::string &utdId) const = 0;
    virtual std::shared_ptr<PasteDataRecord> GetRecordById(uint32_t recordId) const = 0;
    virtual std::shared_ptr<PasteDataEntry> GetEntry(const std::string &utdType) = 0;
    virtual bool IsOn() const = 0;
};

class PasteboardServiceInterfaceMock : public PasteboardServiceInterface {
public:
    PasteboardServiceInterfaceMock();
    ~PasteboardServiceInterfaceMock() override;

    MOCK_METHOD1(QueryActiveOsAccountIds, OHOS::ErrCode(std::vector<int32_t> &ids));
    MOCK_METHOD1(Decode, bool(const std::vector<std::uint8_t> &buffer));
    MOCK_CONST_METHOD1(Encode, bool(std::vector<uint8_t> &buffer));
    MOCK_METHOD0(GetCallingUid, pid_t());
    MOCK_METHOD0(GetCallingTokenID, uint32_t());
    MOCK_METHOD0(GetCallingFullTokenID, uint64_t());
    MOCK_METHOD0(GetCallingPid, pid_t());
    MOCK_CONST_METHOD1(HasContent, bool(const std::string &utdId));
    MOCK_METHOD1(GetEntry, std::shared_ptr<PasteDataEntry>(const std::string &utdType));
    MOCK_CONST_METHOD0(IsOn, bool());
    MOCK_CONST_METHOD1(GetRecordById, std::shared_ptr<PasteDataRecord>(uint32_t recordId));
};

static void *g_interface = nullptr;
static bool g_accountIds = false;
static bool g_encodeInsert = true;
static int g_encodeInsertCount = 0;

PasteboardServiceInterfaceMock::PasteboardServiceInterfaceMock()
{
    g_interface = reinterpret_cast<void *>(this);
}

PasteboardServiceInterfaceMock::~PasteboardServiceInterfaceMock()
{
    g_interface = nullptr;
}

static PasteboardServiceInterface *GetPasteboardServiceInterface()
{
    return reinterpret_cast<PasteboardServiceInterface *>(g_interface);
}

extern "C" {
bool DistributedModuleConfig::IsOn()
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->IsOn();
}

std::shared_ptr<PasteDataEntry> PasteDataRecord::GetEntry(const std::string &utdType)
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return nullptr;
    }
    return interface->GetEntry(utdType);
}

std::shared_ptr<PasteDataRecord> PasteData::GetRecordById(uint32_t recordId) const
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return nullptr;
    }
    return interface->GetRecordById(recordId);
}

bool PasteDataEntry::HasContent(const std::string &utdId) const
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->HasContent(utdId);
}

bool TLVWriteable::Encode(std::vector<uint8_t> &buffer) const
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->Encode(buffer);
}

bool TLVReadable::Decode(const std::vector<uint8_t> &buffer)
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->Decode(buffer);
}

pid_t IPCSkeleton::GetCallingUid()
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->GetCallingUid();
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->GetCallingTokenID();
}

uint64_t IPCSkeleton::GetCallingFullTokenID()
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->GetCallingFullTokenID();
}

pid_t IPCSkeleton::GetCallingPid()
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->GetCallingPid();
}
}

namespace AccountSA {
OHOS::ErrCode OsAccountManager::QueryActiveOsAccountIds(std::vector<int32_t> &ids)
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return -1;
    }
    ids.clear();
    if (g_accountIds) {
        ids.push_back(ACCOUNT_IDS_RANDOM);
    }
    return interface->QueryActiveOsAccountIds(ids);
}
}

class PasteboardServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardServiceTest::SetUpTestCase(void) { }

void PasteboardServiceTest::TearDownTestCase(void) { }

void PasteboardServiceTest::SetUp(void) { }

void PasteboardServiceTest::TearDown(void) { }

namespace MiscServices {
bool WriteRawData(const void *data, int64_t size, int &serFd);
/**
 * @tc.name: IncreaseChangeCountTest001
 * @tc.desc: IncreaseChangeCount should reset to 0 after reach maximum limit.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IncreaseChangeCountTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t userId = 10;
    tempPasteboard->clipChangeCount_.Compute(userId, [](auto, uint32_t &changeCount) {
        changeCount = UINT32_MAX;
        return true;
    });
    uint32_t testCount = 0;
    auto it = tempPasteboard->clipChangeCount_.Find(userId);
    if (it.first) {
        testCount = it.second;
    }
    ASSERT_EQ(testCount, UINT32_MAX);
    tempPasteboard->IncreaseChangeCount(userId);
    it = tempPasteboard->clipChangeCount_.Find(userId);
    if (it.first) {
        testCount = it.second;
    }
    ASSERT_EQ(testCount, 0);
}

/**
 * @tc.name: IncreaseChangeCountTest002
 * @tc.desc: IncreaseChangeCount should reset to 0 after switch to a new user.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IncreaseChangeCountTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    uint32_t testCount = 0;
    tempPasteboard->GetChangeCount(testCount);
    ASSERT_EQ(testCount, 0);
    tempPasteboard->currentUserId_ = 10;
    auto userId = tempPasteboard->GetCurrentAccountId();
    tempPasteboard->IncreaseChangeCount(userId);
    tempPasteboard->GetChangeCount(testCount);
    ASSERT_EQ(testCount, 1);
    tempPasteboard->currentUserId_ = 100;
    tempPasteboard->GetChangeCount(testCount);
    ASSERT_EQ(testCount, 0);
}

/**
 * @tc.name: IsDisallowDistributedTest
 * @tc.desc: IsDisallowDistributed Check CallingUID contral collaboration.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsDisallowDistributedTest, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    ASSERT_EQ(tempPasteboard->IsDisallowDistributed(), false);
}

/**
 * @tc.name: ClearTest001
 * @tc.desc: test Func Clear
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ClearTest001, TestSize.Level0)
{
    PasteboardService service;
    service.currentUserId_ = ACCOUNT_IDS_RANDOM;
    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, std::make_shared<PasteData>());
    int32_t result = service.Clear();
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: ClearTest002
 * @tc.desc: test Func Clear
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ClearTest002, TestSize.Level0)
{
    PasteboardService service;
    service.currentUserId_ = ERROR_USERID;
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, QueryActiveOsAccountIds(testing::_)).WillOnce(Return(INT_ONE));
    int32_t result = service.Clear();
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_USERID_ERROR));
}

/**
 * @tc.name: GetChangeCountTest001
 * @tc.desc: test Func GetChangeCount
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetChangeCountTest001, TestSize.Level0)
{
    PasteboardService service;
    uint32_t changeCount = 0;
    int32_t result = service.GetChangeCount(changeCount);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SubscribeEntityObserverTest001
 * @tc.desc: test Func SubscribeEntityObserver
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SubscribeEntityObserverTest001, TestSize.Level0)
{
    PasteboardService service;
    EntityType entityType = EntityType::ADDRESS;
    uint32_t expectedDataLength = MAX_RECOGNITION_LENGTH;
    const sptr<IEntityRecognitionObserver> observer = new MyTestEntityRecognitionObserver();

    int32_t result = service.SubscribeEntityObserver(entityType, expectedDataLength, observer);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnsubscribeEntityObserverTest001
 * @tc.desc: test Func UnsubscribeEntityObserver
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeEntityObserverTest001, TestSize.Level0)
{
    PasteboardService service;
    EntityType entityType = EntityType::ADDRESS;
    uint32_t expectedDataLength = MAX_RECOGNITION_LENGTH;
    const sptr<IEntityRecognitionObserver> observer = new MyTestEntityRecognitionObserver();

    int32_t result = service.UnsubscribeEntityObserver(entityType, expectedDataLength, observer);
    EXPECT_EQ(result, ERR_OK);
}


/**
 * @tc.name: GetRecordValueByTypeTest001
 * @tc.desc: test Func GetRecordValueByType
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest001, TestSize.Level0)
{
    PasteboardService service;
    uint32_t dataId = 0;
    uint32_t recordId = 0;
    PasteDataEntry value;
    value.SetUtdId(RANDOM_STRING);
    service.currentUserId_ = ACCOUNT_IDS_RANDOM;
    auto data = std::make_shared<PasteData>();
    auto record = PasteDataRecord::NewPlainTextRecord(RANDOM_STRING);
    auto entry = std::make_shared<PasteDataEntry>();
    data->SetDataId(service.delayDataId_);
    data->AddRecord(record);
    data->SetRemote(true);
    entry->SetMimeType(MIMETYPE_TEXT_HTML);
    entry->SetValue(HTML_STRING);

    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, data);
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetCallingTokenID()).WillOnce(Return(service.delayTokenId_.load()));
    EXPECT_CALL(mock, HasContent(testing::_)).WillOnce(Return(false));
    EXPECT_CALL(mock, GetRecordById(testing::_)).WillOnce(Return(record));
    EXPECT_CALL(mock, GetEntry(testing::_)).WillOnce(Return(entry));

    int32_t result = service.GetRecordValueByType(service.delayDataId_, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::PLUGIN_IS_NULL));
}

/**
 * @tc.name: GetRecordValueByTypeTest002
 * @tc.desc: test Func GetRecordValueByType when paste data is html
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest002, TestSize.Level0)
{
    PasteboardService service;
    uint32_t dataId = 0;
    uint32_t recordId = 0;
    PasteDataEntry value;
    value.SetUtdId(RANDOM_STRING);
    service.currentUserId_ = ACCOUNT_IDS_RANDOM;
    auto data = std::make_shared<PasteData>();
    auto record = PasteDataRecord::NewPlainTextRecord(RANDOM_STRING);
    auto entry = std::make_shared<PasteDataEntry>();
    data->SetDataId(service.delayDataId_);
    data->AddRecord(record);
    data->SetRemote(false);
    entry->SetMimeType(MIMETYPE_TEXT_HTML);
    entry->SetValue(HTML_STRING);

    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, data);
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetCallingTokenID()).WillOnce(Return(service.delayTokenId_.load()));
    EXPECT_CALL(mock, HasContent(testing::_)).WillOnce(Return(true));
    EXPECT_CALL(mock, GetRecordById(testing::_)).WillOnce(Return(record));
    EXPECT_CALL(mock, GetEntry(testing::_)).WillOnce(Return(entry)).WillOnce(Return(entry));

    int32_t result = service.GetRecordValueByType(service.delayDataId_, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::E_OK));
}

/**
 * @tc.name: GetRecordValueByTypeTest003
 * @tc.desc: test Func GetRecordValueByType when paste data is url
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest003, TestSize.Level0)
{
    PasteboardService service;
    uint32_t dataId = 0;
    uint32_t recordId = 0;
    PasteDataEntry value;
    value.SetUtdId(RANDOM_STRING);
    service.currentUserId_ = ACCOUNT_IDS_RANDOM;
    auto data = std::make_shared<PasteData>();
    auto record = PasteDataRecord::NewPlainTextRecord(RANDOM_STRING);
    auto entry = std::make_shared<PasteDataEntry>();
    data->SetDataId(service.delayDataId_);
    data->AddRecord(record);
    data->SetRemote(false);
    entry->SetMimeType(MIMETYPE_TEXT_URI);
    

    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, data);
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetCallingTokenID()).WillOnce(Return(service.delayTokenId_.load()));
    EXPECT_CALL(mock, HasContent(testing::_)).WillOnce(Return(true));
    EXPECT_CALL(mock, GetRecordById(testing::_)).WillOnce(Return(record));
    EXPECT_CALL(mock, GetEntry(testing::_)).WillOnce(Return(entry)).WillOnce(Return(entry));

    int32_t result = service.GetRecordValueByType(service.delayDataId_, recordId, value);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::E_OK));
}

/**
 * @tc.name: GetRecordValueByTypeTest004
 * @tc.desc: test Func GetRecordValueByType when paste Encode return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest004, TestSize.Level0)
{
    PasteboardService service;
    int64_t rawDataSize = 0;
    std::vector<uint8_t> buffer;
    int32_t fd = 0;
    PasteDataEntry entryValue;

    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, Encode(testing::_)).WillOnce(Return(false));

    int32_t result = service.GetRecordValueByType(rawDataSize, buffer, fd, entryValue);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest005
 * @tc.desc: test Func GetRecordValueByType when paste rawDataSize < MIN_ASHMEM_DATA_SIZE
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest005, TestSize.Level0)
{
    PasteboardService service;
    int64_t rawDataSize = 0;
    std::vector<uint8_t> buffer;
    int32_t fd = 0;
    PasteDataEntry entryValue;
    g_encodeInsert = true;
    g_encodeInsertCount = MIN_ASHMEM_DATA_SIZE + 1;
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, Encode(testing::_)).WillOnce(Return(true));

    int32_t result = service.GetRecordValueByType(rawDataSize, buffer, fd, entryValue);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetRecordValueByTypeTest006
 * @tc.desc: test Func GetRecordValueByType when paste Encode return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest006, TestSize.Level0)
{
    PasteboardService service;
    int64_t rawDataSize = 0;
    std::vector<uint8_t> buffer;
    int32_t fd = 0;
    PasteDataEntry entryValue;
    g_encodeInsert = true;
    g_encodeInsertCount = INT_THREETHREETHREE;
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, Encode(testing::_)).WillOnce(Return(true));

    int32_t result = service.GetRecordValueByType(rawDataSize, buffer, fd, entryValue);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetRecordValueByTypeTest007
 * @tc.desc: test Func GetRecordValueByType rawDataSize == 0
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest007, TestSize.Level0)
{
    PasteboardService service;
    uint32_t dataId = 0;
    uint32_t recordId = 0;
    int64_t rawDataSize = INT64_NEGATIVE_NUMBER;
    std::vector<uint8_t> buffer;
    int fd = 0;

    int32_t result = service.GetRecordValueByType(dataId, recordId, rawDataSize, buffer, fd);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest008
 * @tc.desc: test Func GetRecordValueByType rawDataSize > DEFAULT_MAX_RAW_DATA_SIZE
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest008, TestSize.Level0)
{
    PasteboardService service;
    uint32_t dataId = 0;
    uint32_t recordId = 0;
    int64_t rawDataSize = INT64_MAX;
    std::vector<uint8_t> buffer;
    int fd = 0;

    int32_t result = service.GetRecordValueByType(dataId, recordId, rawDataSize, buffer, fd);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest009
 * @tc.desc: test Func GetRecordValueByType rawDataSize > MIN_ASHMEM_DATA_SIZE
 *      Decode return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest009, TestSize.Level0)
{
    PasteboardService service;
    uint32_t dataId = 0;
    uint32_t recordId = 0;
    int64_t rawDataSize = MIN_ASHMEM_DATA_SIZE + 1;
    std::vector<uint8_t> buffer;
    service.currentUserId_ = ACCOUNT_IDS_RANDOM;
    int fd = open("/dev/zero", O_RDWR);
    std::vector<char> data(rawDataSize, 'a');
    auto pasteData = std::make_shared<PasteData>();
    auto record = PasteDataRecord::NewPlainTextRecord(RANDOM_STRING);
    auto entry = std::make_shared<PasteDataEntry>();
    pasteData->SetDataId(service.delayDataId_);
    pasteData->AddRecord(record);
    pasteData->SetRemote(false);
    entry->SetMimeType(MIMETYPE_TEXT_URI);
    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, pasteData);
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, Decode(testing::_)).WillRepeatedly(Return(false));
    int32_t result = service.GetRecordValueByType(dataId, recordId, rawDataSize, buffer, fd);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR));
    close(fd);
}

/**
 * @tc.name: GetRecordValueByTypeTest010
 * @tc.desc: test Func GetRecordValueByType Decode return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest010, TestSize.Level0)
{
    PasteboardService service;
    uint32_t dataId = 0;
    uint32_t recordId = 0;
    int64_t rawDataSize = 1024;
    std::vector<uint8_t> buffer;
    int fd;
    buffer.insert(buffer.end(), rawDataSize, UINT8_ONE);
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, Decode(testing::_)).WillRepeatedly(Return(false));
    int32_t result = service.GetRecordValueByType(dataId, recordId, rawDataSize, buffer, fd);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR));
}

/**
 * @tc.name: GetRecordValueByTypeTest011
 * @tc.desc: test Func GetRecordValueByType second child GetRecordValueByType return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRecordValueByTypeTest011, TestSize.Level0)
{
    PasteboardService service;
    uint32_t dataId = 0;
    uint32_t recordId = 0;
    int64_t rawDataSize = 1024;
    std::vector<uint8_t> buffer;
    int fd;
    buffer.insert(buffer.end(), rawDataSize, UINT8_ONE);
    NiceMock<PasteboardServiceInterfaceMock> mock;

    PasteDataEntry value;
    value.SetUtdId(RANDOM_STRING);
    service.currentUserId_ = ACCOUNT_IDS_RANDOM;
    auto data = std::make_shared<PasteData>();
    auto record = PasteDataRecord::NewPlainTextRecord(RANDOM_STRING);
    auto entry = std::make_shared<PasteDataEntry>();
    data->SetDataId(service.delayDataId_);
    data->AddRecord(record);
    data->SetRemote(false);
    entry->SetMimeType(MIMETYPE_TEXT_URI);
    
    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, data);
    EXPECT_CALL(mock, Decode(testing::_)).WillRepeatedly(Return(true));
    EXPECT_CALL(mock, GetCallingTokenID()).WillOnce(Return(service.delayTokenId_.load()));
    EXPECT_CALL(mock, HasContent(testing::_)).WillOnce(Return(true));
    EXPECT_CALL(mock, GetRecordById(testing::_)).WillOnce(Return(record));
    EXPECT_CALL(mock, GetEntry(testing::_)).WillOnce(Return(entry)).WillOnce(Return(entry));

    int32_t result = service.GetRecordValueByType(dataId, recordId, rawDataSize, buffer, fd);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR));
}

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress HasPasteData return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ShowProgressTest001, TestSize.Level0)
{
    PasteboardService service;
    service.currentUserId_ = ERROR_USERID;
    const std::string progressKey;
    sptr<IRemoteObject> observer;
    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, std::make_shared<PasteData>());
    int32_t result = service.ShowProgress(progressKey, observer);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}

/**
 * @tc.name: ShowProgressTest002
 * @tc.desc: test Func ShowProgress IsFocusedApp return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ShowProgressTest002, TestSize.Level0)
{
    PasteboardService service;
    const std::string progressKey;
    sptr<IRemoteObject> observer;
    service.currentUserId_ = ACCOUNT_IDS_RANDOM;
    auto data = std::make_shared<PasteData>();
    g_accountIds = true;
    service.currentScreenStatus = ScreenEvent::ScreenUnlocked;
    service.copyTime_.InsertOrAssign(ACCOUNT_IDS_RANDOM,
        static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()));
    auto tokenId = IPCSkeleton::GetCallingTokenID();

    data->SetScreenStatus(ScreenEvent::ScreenUnlocked);
    data->SetShareOption(ShareOption::LocalDevice);
    data->SetTokenId(IPCSkeleton::GetCallingTokenID());
    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, data);
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, QueryActiveOsAccountIds(testing::_)).WillRepeatedly(Return(ERR_OK));
    EXPECT_CALL(mock, GetCallingTokenID()).WillRepeatedly(Return(tokenId + UINT32_EXCEPTION_APPID));

    int32_t result = service.ShowProgress(progressKey, observer);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR));
}

/**
 * @tc.name: ShowProgressTest003
 * @tc.desc: test Func ShowProgress
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ShowProgressTest003, TestSize.Level0)
{
    PasteboardService service;
    const std::string progressKey;
    sptr<IRemoteObject> observer;
    service.currentUserId_ = ACCOUNT_IDS_RANDOM;
    auto data = std::make_shared<PasteData>();
    g_accountIds = true;
    service.currentScreenStatus = ScreenEvent::ScreenUnlocked;
    service.copyTime_.InsertOrAssign(ACCOUNT_IDS_RANDOM,
        static_cast<uint64_t>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()));

    data->SetScreenStatus(ScreenEvent::ScreenUnlocked);
    data->SetShareOption(ShareOption::LocalDevice);
    data->SetTokenId(IPCSkeleton::GetCallingTokenID());
    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, data);
    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, QueryActiveOsAccountIds(testing::_)).WillRepeatedly(Return(ERR_OK));

    int32_t result = service.ShowProgress(progressKey, observer);
    EXPECT_EQ(result, static_cast<int32_t>(ERR_OK));
}

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, WriteRawDataTest001, TestSize.Level0)
{
    PasteboardService service;
    char rawData[] = "testData";
    int32_t fd = INT32_NEGATIVE_NUMBER;
    bool result = service.WriteRawData(rawData, sizeof(rawData), fd);
    EXPECT_EQ(result, true);
}

int32_t PasteStart(const std::string &pasteId);

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteStart001, TestSize.Level0)
{
    PasteboardService service;
    service.ffrtTimer_ = nullptr;
    std::string pasteId;
    int32_t result = service.PasteStart(pasteId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteStart002, TestSize.Level0)
{
    PasteboardService service;
    service.ffrtTimer_ = std::make_shared<FFRTTimer>();
    std::string pasteId;
    int32_t result = service.PasteStart(pasteId);
    EXPECT_EQ(result, ERR_OK);
}

int32_t PasteComplete(const std::string &deviceId, const std::string &pasteId);

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteComplete001, TestSize.Level0)
{
    std::string deviceId = "testId";
    std::string pasteId;
    PasteboardService service;
    int32_t result = service.PasteComplete(deviceId, pasteId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteComplete002, TestSize.Level0)
{
    std::string deviceId;
    std::string pasteId;
    PasteboardService service;
    int32_t result = service.PasteComplete(deviceId, pasteId);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetMimeTypesTest001, TestSize.Level0)
{
    PasteboardService service;
    service.currentScreenStatus = ScreenEvent::ScreenUnlocked;
    std::vector<std::string> funcResult;
    int32_t result = service.GetMimeTypes(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetMimeTypesTest002, TestSize.Level0)
{
    PasteboardService service;
    service.currentUserId_ = ERROR_USERID;
    std::vector<std::string> funcResult;
    int32_t result = service.GetMimeTypes(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

int32_t GetRemoteDeviceName(std::string &deviceName, bool &isRemote);

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRemoteDeviceName001, TestSize.Level0)
{
    PasteboardService service;
    service.currentUserId_ = ERROR_USERID;
    std::string deviceName;
    bool isRemote = false;
    int32_t result = service.GetRemoteDeviceName(deviceName, isRemote);
    EXPECT_EQ(result, ERR_OK);
}
int32_t HasPasteData(bool &funcResult);

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasPasteDataTest002, TestSize.Level0)
{
    PasteboardService service;
    service.currentScreenStatus = ScreenEvent::ScreenUnlocked;
    bool flag = false;
    int32_t result = service.HasPasteData(flag);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(flag, false);
}

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasPasteDataTest003, TestSize.Level0)
{
    PasteboardService service;
    service.clips_.Clear();
    std::shared_ptr<PasteData> dataPtr(nullptr);
    int32_t userId = INT_ONE;
    service.clips_.InsertOrAssign(userId, dataPtr);
    bool flag = false;
    int32_t result = service.HasPasteData(flag);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: ShowProgressTest001
 * @tc.desc: test Func ShowProgress 
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasPasteDataTest004, TestSize.Level0)
{
    PasteboardService service;
    service.clips_.Clear();
    std::shared_ptr<PasteData> dataPtr = std::make_shared<PasteData>();
    int32_t userId = INT_ONE;
    service.clips_.InsertOrAssign(userId, dataPtr);
    bool flag = false;
    int32_t result = service.HasPasteData(flag);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: HasDataTypeTest001
 * @tc.desc: currentScreenStatus is default.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasDataTypeTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    bool funcResult;
    int32_t res = tempPasteboard->HasDataType(MIMETYPE_TEXT_PLAIN, funcResult);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: HasDataTypeTest002
 * @tc.desc: currentScreenStatus is ScreenEvent::ScreenUnlocked.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasDataTypeTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    tempPasteboard->currentScreenStatus = ScreenEvent::ScreenUnlocked;
    tempPasteboard->currentUserId_ = 1;
    tempPasteboard->clipPlugin_ = nullptr;
    bool funcResult;
    int32_t res = tempPasteboard->HasDataType(MIMETYPE_TEXT_PLAIN, funcResult);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: HasDataTypeTest003
 * @tc.desc: currentScreenStatus is ScreenEvent::ScreenUnlocked.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasDataTypeTest003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    tempPasteboard->currentScreenStatus = ScreenEvent::ScreenUnlocked;
    tempPasteboard->currentUserId_ = 1;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));
    bool funcResult;
    int32_t res = tempPasteboard->HasDataType(MIMETYPE_TEXT_PLAIN, funcResult);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: HasDataTypeTest004
 * @tc.desc: clipPlugin_ is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasDataTypeTest004, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    tempPasteboard->currentScreenStatus = ScreenEvent::ScreenUnlocked;
    tempPasteboard->currentUserId_ = 1;
    std::string PLUGIN_NAME_VAL = "distributed_clip";
    auto release = [&PLUGIN_NAME_VAL, this](ClipPlugin *plugin) {
        ClipPlugin::DestroyPlugin(PLUGIN_NAME_VAL, plugin);
    };
    tempPasteboard->clipPlugin_ = std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME_VAL), release);
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(false));
    bool funcResult;
    int32_t res = tempPasteboard->HasDataType(MIMETYPE_TEXT_PLAIN, funcResult);
    EXPECT_EQ(res, ERR_OK);
}

/**
 * @tc.name: DetectPatternsTest001
 * @tc.desc: DetectPatterns return PasteboardError::NO_DATA_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DetectPatternsTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    std::vector<Pattern> patternsToCheck;
    std::vector<Pattern> funcResult;
    EXPECT_EQ(tempPasteboard->DetectPatterns(patternsToCheck, funcResult),
        static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}

/**
 * @tc.name: DetectPatternsTest002
 * @tc.desc: DetectPatterns return PasteboardError::NO_DATA_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DetectPatternsTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    int32_t userId = INT32_MAX;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->AddTextRecord("hello");
    tempPasteboard->clips_.InsertOrAssign(userId, pasteData);
    std::vector<Pattern> patternsToCheck;
    std::vector<Pattern> funcResult;
    EXPECT_EQ(tempPasteboard->DetectPatterns(patternsToCheck, funcResult),
        static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}

/**
 * @tc.name: DetectPatternsTest003
 * @tc.desc: DetectPatterns test MIMETYPE_TEXT_PLAIN
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DetectPatternsTest003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    int32_t userId = tempPasteboard->currentUserId_ = 1;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->AddTextRecord("hello");
    tempPasteboard->clips_.InsertOrAssign(userId, pasteData);
    std::vector<Pattern> patternsToCheck;
    std::vector<Pattern> funcResult;
    EXPECT_EQ(tempPasteboard->DetectPatterns(patternsToCheck, funcResult),
        static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}

/**
 * @tc.name: DetectPatternsTest004
 * @tc.desc: DetectPatterns test MIMETYPE_TEXT_HTML
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DetectPatternsTest004, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    int32_t userId = tempPasteboard->currentUserId_ = 1;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->AddHtmlRecord("<div class='disable'>helloWorld</div>");
    tempPasteboard->clips_.InsertOrAssign(userId, pasteData);
    std::vector<Pattern> patternsToCheck;
    std::vector<Pattern> funcResult;
    EXPECT_EQ(tempPasteboard->DetectPatterns(patternsToCheck, funcResult),
        static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}

/**
 * @tc.name: DetectPatternsTest005
 * @tc.desc: DetectPatterns test MIMETYPE_TEXT_URI
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DetectPatternsTest005, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);
    int32_t userId = tempPasteboard->currentUserId_ = 1;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    OHOS::Uri uri("/");
    pasteData->AddUriRecord(uri);
    tempPasteboard->clips_.InsertOrAssign(userId, pasteData);
    std::vector<Pattern> patternsToCheck;
    std::vector<Pattern> funcResult;
    EXPECT_EQ(tempPasteboard->DetectPatterns(patternsToCheck, funcResult),
        static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}

/**
 * @tc.name: SetPasteDataDelayDataTest001
 * @tc.desc: SetPasteData return INVALID_PARAM_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataDelayDataTest001, TestSize.Level0)
{
    int fd = -1;
    int64_t rawDataSize = 0;
    std::vector<uint8_t> buffer = {'h', 'e', 'l', 'l', 'o'};
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t result = tempPasteboard->SetPasteDataDelayData(fd, rawDataSize, buffer, delayGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR));
}

/**
 * @tc.name: SetPasteDataDelayDataTest002
 * @tc.desc: SetPasteData return INVALID_PARAM_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataDelayDataTest002, TestSize.Level0)
{
    int fd = -1;
    int64_t rawDataSize = INT64_MAX;
    std::vector<uint8_t> buffer = {'h', 'e', 'l', 'l', 'o'};
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t result = tempPasteboard->SetPasteDataDelayData(fd, rawDataSize, buffer, delayGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR));
}

/**
 * @tc.name: SetPasteDataDelayDataTest003
 * @tc.desc: fd is error, map failed, SetPasteData return INVALID_DATA_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataDelayDataTest003, TestSize.Level0)
{
    int fd = -1;
    int64_t rawDataSize = USHRT_MAX;
    std::vector<uint8_t> buffer = {'h', 'e', 'l', 'l', 'o'};
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t result = tempPasteboard->SetPasteDataDelayData(fd, rawDataSize, buffer, delayGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR));
}

/**
 * @tc.name: SetPasteDataDelayDataTest004
 * @tc.desc: fd is right, but buffer is empty, return NO_DATA_ERROR;
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataDelayDataTest004, TestSize.Level0)
{
    int fd = 0XF;
    int64_t rawDataSize = USHRT_MAX;
    std::vector<uint8_t> buffer;
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t result = tempPasteboard->SetPasteDataDelayData(fd, rawDataSize, buffer, delayGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_DATA_ERROR));
}

/**
 * @tc.name: SetPasteDataDelayDataTest005
 * @tc.desc: NOT goto map and buffer is empty
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataDelayDataTest005, TestSize.Level0)
{
    int fd = -1;
    int64_t rawDataSize = UINT8_MAX;
    std::vector<uint8_t> buffer;
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t result = tempPasteboard->SetPasteDataDelayData(fd, rawDataSize, buffer, delayGetter);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}

/**
 * @tc.name: SetPasteDataDelayDataTest006
 * @tc.desc: NOT goto map and buffer is NOT empty.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataDelayDataTest006, TestSize.Level0)
{
    int fd = -1;
    int64_t rawDataSize = UINT8_MAX;
    std::vector<uint8_t> buffer {'h', 'e', 'l', 'l', 'o'};
    sptr<IPasteboardDelayGetter> delayGetter = nullptr;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t result = tempPasteboard->SetPasteDataDelayData(fd, rawDataSize, buffer, delayGetter);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: SetPasteDataEntryDataTest001
 * @tc.desc: NOT goto map and buffer is NOT empty.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataEntryDataTest001, TestSize.Level0)
{
    int fd = -1;
    int64_t rawDataSize = UINT8_MAX;
    std::vector<uint8_t> buffer {'h', 'e', 'l', 'l', 'o'};
    sptr<IPasteboardEntryGetter> entryGetter = nullptr;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t result = tempPasteboard->SetPasteDataEntryData(fd, rawDataSize, buffer, entryGetter);
    EXPECT_NE(result, ERR_OK);
}

/**
 * @tc.name: IsRemoteDataTest001
 * @tc.desc: test Func IsRemoteData, funcResult is false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsRemoteDataTest001, TestSize.Level0)
{
    PasteboardService service;
    service.currentUserId_ = ERROR_USERID;
    bool funcResult;
    int32_t result = service.IsRemoteData(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: IsRemoteDataTest002
 * @tc.desc: test Func IsRemoteData, currentUserId_ is INT32_MAX.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsRemoteDataTest002, TestSize.Level0)
{
    PasteboardService service;
    service.currentUserId_ = INT32_MAX;
    bool funcResult;
    int32_t result = service.IsRemoteData(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: IsRemoteDataTest003
 * @tc.desc: test Func IsRemoteData, currentUserId_ is 0XF
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsRemoteDataTest003, TestSize.Level0)
{
    PasteboardService service;
    int32_t userId = service.currentUserId_ = 0XF;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->AddTextRecord("hello");
    service.clips_.InsertOrAssign(userId, pasteData);
    bool funcResult;
    int32_t result = service.IsRemoteData(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SetPasteDataOnlyTest001
 * @tc.desc: test Func SetPasteDataOnly, it will be return INVALID_PARAM_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataOnlyTest001, TestSize.Level0)
{
    PasteboardService service;
    int64_t rawDataSize = 0;
    std::vector<uint8_t> buffer;
    int fd = 3;

    int32_t result = service.SetPasteDataOnly(fd, rawDataSize, buffer);
    ASSERT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR));
}

/**
 * @tc.name: SetAppShareOptionsTest001
 * @tc.desc: test Func SetAppShareOptions, it will be return INVALID_PARAM_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetAppShareOptionsTest001, TestSize.Level0)
{
    PasteboardService service;
    int32_t shareOptions = 1;

    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetCallingFullTokenID).WillOnce(testing::Return(123));
    int32_t result = service.SetAppShareOptions(shareOptions);
    ASSERT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR));
}

/**
 * @tc.name: SetAppShareOptionsTest002
 * @tc.desc: test Func SetAppShareOptions, it will be return PERMISSION_VERIFICATION_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetAppShareOptionsTest002, TestSize.Level0)
{
    PasteboardService service;
    int32_t shareOptions = 0;

    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetCallingFullTokenID).WillOnce(testing::Return(123));
    EXPECT_CALL(mock, GetCallingTokenID).WillOnce(testing::Return(1000));
    int32_t result = service.SetAppShareOptions(shareOptions);
    ASSERT_EQ(result, static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR));
}

/**
 * @tc.name: SetAppShareOptionsTest003
 * @tc.desc: test Func SetAppShareOptions, it will be return PERMISSION_VERIFICATION_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetAppShareOptionsTest003, TestSize.Level0)
{
    PasteboardService service;
    int32_t shareOptions = 0;

    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingFullTokenID).WillOnce(testing::Return(4294967295));
    EXPECT_CALL(ipcMock, GetCallingTokenID).WillOnce(testing::Return(1000));
    int32_t result = service.SetAppShareOptions(shareOptions);
    ASSERT_EQ(result, static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR));
}

/**
 * @tc.name: GetGlobalShareOptionTest001
 * @tc.desc: test Func GetGlobalShareOption, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetGlobalShareOptionTest001, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(3057));

    std::vector<uint32_t> tokenIds;
    std::unordered_map<uint32_t, int32_t> funcResult;
    int32_t result = service.GetGlobalShareOption(tokenIds, funcResult);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetGlobalShareOptionTest002
 * @tc.desc: test Func GetGlobalShareOption, it will be return PERMISSION_VERIFICATION_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetGlobalShareOptionTest002, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(0));

    std::vector<uint32_t> tokenIds;
    std::unordered_map<uint32_t, int32_t> funcResult;
    int32_t result = service.GetGlobalShareOption(tokenIds, funcResult);
    ASSERT_EQ(result, static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR));
}

/**
 * @tc.name: GetGlobalShareOptionTest003
 * @tc.desc: test Func GetGlobalShareOption, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetGlobalShareOptionTest003, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(3057));

    std::vector<uint32_t> tokenIds = {1, 2, 3};
    std::unordered_map<uint32_t, int32_t> funcResult;
    int32_t result = service.GetGlobalShareOption(tokenIds, funcResult);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SetGlobalShareOptionTest001
 * @tc.desc: test Func SetGlobalShareOption, it will be return PERMISSION_VERIFICATION_ERROR.
 * @tc.type: FUNC
 */
 HWTEST_F(PasteboardServiceTest, SetGlobalShareOptionTest001, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(0));

    std::unordered_map<uint32_t, int32_t> globalShareOptions = {{1, static_cast<int32_t>(InApp)}};
    int32_t result = service.SetGlobalShareOption(globalShareOptions);
    ASSERT_EQ(result, static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR));
}

/**
 * @tc.name: SetGlobalShareOptionTest002
 * @tc.desc: test Func SetGlobalShareOption, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetGlobalShareOptionTest002, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(3057));

    std::unordered_map<uint32_t, int32_t> globalShareOptions =
	    {{1, static_cast<int32_t>(InApp)}, {2, static_cast<int32_t>(CrossDevice)}};
    int32_t result = service.SetGlobalShareOption(globalShareOptions);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SetGlobalShareOptionTest003
 * @tc.desc: test Func SetGlobalShareOption, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetGlobalShareOptionTest003, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(3057));

    std::unordered_map<uint32_t, int32_t> globalShareOptions = {{1, static_cast<int32_t>(InApp)}, {2, -1}};
    int32_t result = service.SetGlobalShareOption(globalShareOptions);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnsubscribeAllObserverTest001
 * @tc.desc: test Func UnsubscribeAllObserver, it will be return 0.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeAllObserverTest001, TestSize.Level0)
{
    PasteboardService service;
    PasteboardObserverType type = PasteboardObserverType::OBSERVER_EVENT;
    int32_t result = service.UnsubscribeAllObserver(type);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: UnsubscribeAllObserverTest002
 * @tc.desc: test Func UnsubscribeAllObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeAllObserverTest002, TestSize.Level0)
{
    PasteboardService service;
    PasteboardObserverType type = PasteboardObserverType::OBSERVER_LOCAL;
    int32_t result = service.UnsubscribeAllObserver(type);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnsubscribeAllObserverTest003
 * @tc.desc: test Func UnsubscribeAllObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeAllObserverTest003, TestSize.Level0)
{
    PasteboardService service;
    PasteboardObserverType type = PasteboardObserverType::OBSERVER_REMOTE;

    int32_t result = service.UnsubscribeAllObserver(type);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnsubscribeAllObserverTest004
 * @tc.desc: test Func UnsubscribeAllObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeAllObserverTest004, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(3057));
    PasteboardObserverType type = PasteboardObserverType::OBSERVER_EVENT;

    int32_t result = service.UnsubscribeAllObserver(type);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnsubscribeObserverTest001
 * @tc.desc: test Func UnsubscribeObserver, it will be return 0.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeObserverTest001, TestSize.Level0)
{
    PasteboardService service;

    PasteboardObserverType type = PasteboardObserverType::OBSERVER_EVENT;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.UnsubscribeObserver(type, observer);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: UnsubscribeObserverTest002
 * @tc.desc: test Func UnsubscribeObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeObserverTest002, TestSize.Level0)
{
    PasteboardService service;
    PasteboardObserverType type = PasteboardObserverType::OBSERVER_LOCAL;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.UnsubscribeObserver(type, observer);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnsubscribeObserverTest003
 * @tc.desc: test Func UnsubscribeObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeObserverTest003, TestSize.Level0)
{
    PasteboardService service;
    PasteboardObserverType type = PasteboardObserverType::OBSERVER_REMOTE;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.UnsubscribeObserver(type, observer);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnsubscribeObserverTest004
 * @tc.desc: test Func UnsubscribeObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeObserverTest004, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(3057));

    PasteboardObserverType type = PasteboardObserverType::OBSERVER_EVENT;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.UnsubscribeObserver(type, observer);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnsubscribeObserverTest005
 * @tc.desc: test Func UnsubscribeObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeObserverTest005, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(0));

    PasteboardObserverType type = PasteboardObserverType::OBSERVER_EVENT;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.UnsubscribeObserver(type, observer);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SubscribeObserverTest001
 * @tc.desc: test Func SubscribeObserver, it will be return 0.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SubscribeObserverTest001, TestSize.Level0)
{
    PasteboardService service;

    PasteboardObserverType type = PasteboardObserverType::OBSERVER_EVENT;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.SubscribeObserver(type, observer);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: SubscribeObserverTest002
 * @tc.desc: test Func SubscribeObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SubscribeObserverTest002, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingPid()).WillOnce(testing::Return(1234));

    PasteboardObserverType type = PasteboardObserverType::OBSERVER_LOCAL;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.SubscribeObserver(type, observer);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SubscribeObserverTest003
 * @tc.desc: test Func SubscribeObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SubscribeObserverTest003, TestSize.Level0)
{
    PasteboardService service;
    PasteboardObserverType type = PasteboardObserverType::OBSERVER_REMOTE;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.SubscribeObserver(type, observer);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SubscribeObserverTest004
 * @tc.desc: test Func SubscribeObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SubscribeObserverTest004, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(3057));
    EXPECT_CALL(ipcMock, GetCallingPid()).WillOnce(testing::Return(1234));

    PasteboardObserverType type = PasteboardObserverType::OBSERVER_EVENT;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.SubscribeObserver(type, observer);
    ASSERT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SubscribeObserverTest005
 * @tc.desc: test Func SubscribeObserver, it will be return ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SubscribeObserverTest005, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(0));
    EXPECT_CALL(ipcMock, GetCallingPid()).WillOnce(testing::Return(1234));

    PasteboardObserverType type = PasteboardObserverType::OBSERVER_EVENT;
    sptr<IPasteboardChangedObserver> observer = nullptr;
    int32_t result = service.SubscribeObserver(type, observer);
    ASSERT_EQ(result, ERR_OK);
}
}
} // namespace OHOS::MiscServices