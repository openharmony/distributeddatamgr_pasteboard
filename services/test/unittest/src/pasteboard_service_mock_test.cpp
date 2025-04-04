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
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "pasteboard_error.h"
#include "pasteboard_service.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace std::chrono;
using namespace OHOS::Security::AccessToken;
namespace OHOS {
namespace {
    const int32_t ACCOUNT_IDS_RANDOM = 1121;
    const uint32_t UINT32_ONE = 1;
    const int INT_ONE = 1;
    const uint8_t UINT8_ONE = 1;
    const int32_t INT32_NEGATIVE_NUMBER = -1;
    const pid_t TEST_SERVER_UID = 3500;
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
    virtual int VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName) = 0;
    virtual ATokenTypeEnum GetTokenTypeFlag(AccessTokenID tokenId) = 0;
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
    MOCK_METHOD2(VerifyAccessToken, int(AccessTokenID tokenID, const std::string &permissionName));
    MOCK_METHOD1(GetTokenTypeFlag, ATokenTypeEnum(AccessTokenID tokenId));
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

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->VerifyAccessToken(tokenID, permissionName);
}

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenId)
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return ATokenTypeEnum::TOKEN_INVALID;
    }
    return interface->GetTokenTypeFlag(tokenId);
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
    EXPECT_CALL(mock, GetTokenTypeFlag).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(mock, QueryActiveOsAccountIds(testing::_)).WillRepeatedly(Return(ERR_OK));

    int32_t result = service.ShowProgress(progressKey, observer);
    EXPECT_EQ(result, static_cast<int32_t>(ERR_OK));
}

/**
 * @tc.name: HasPasteDataTest001
 * @tc.desc: test Func HasPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasPasteDataTest001, TestSize.Level0)
{
    PasteboardService service;
    bool flag = false;
    int32_t result = service.HasPasteData(flag);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(flag, false);
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
 * @tc.name: GetPasteDataTest001
 * @tc.desc: test Func GetPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetPasteDataTest001, TestSize.Level0)
{
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetTokenTypeFlag).WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_NATIVE));
    EXPECT_CALL(mock, VerifyAccessToken).WillOnce(Return(PermissionState::PERMISSION_GRANTED))
        .WillOnce(Return(PermissionState::PERMISSION_GRANTED));
    PasteboardService service;
    int fd;
    int64_t size;
    std::vector<uint8_t> rawData;
    int32_t syncTime;
    int32_t result = service.GetPasteData(fd, size, rawData, "", syncTime);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}

/**
 * @tc.name: GetPasteDataTest002
 * @tc.desc: test Func GetPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetPasteDataTest002, TestSize.Level0)
{
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetTokenTypeFlag).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    PasteboardService service;
    service.setPasteDataUId_ = ERROR_USERID;
    int fd;
    int64_t size;
    std::vector<uint8_t> rawData;
    int32_t syncTime;
    int32_t result = service.GetPasteData(fd, size, rawData, "", syncTime);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR));
}

/**
 * @tc.name: GetPasteDataTest003
 * @tc.desc: test Func GetPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetPasteDataTest003, TestSize.Level0)
{
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetTokenTypeFlag).WillOnce(Return(ATokenTypeEnum::TOKEN_HAP))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_HAP))
        .WillOnce(Return(ATokenTypeEnum::TOKEN_HAP));
    PasteboardService service;
    service.setPasteDataUId_ = TEST_SERVER_UID;
    int fd;
    int64_t size;
    std::vector<uint8_t> rawData;
    int32_t syncTime;
    int32_t result = service.GetPasteData(fd, size, rawData, "", syncTime);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
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
    EXPECT_CALL(mock, VerifyAccessToken).WillOnce(Return(PermissionState::PERMISSION_DENIED));
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
    EXPECT_CALL(ipcMock, VerifyAccessToken).WillOnce(Return(PermissionState::PERMISSION_DENIED));
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