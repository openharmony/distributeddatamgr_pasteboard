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
#include <variant>
#include "accesstoken_kit.h"
#include "dev_profile.h"
#include "input_method_controller.h"
#include "input_method_property.h"
#include "ipc_skeleton.h"
#include "pasteboard_error.h"
#include "pasteboard_service.h"
#include "remote_file_share.h"
#include "eventcenter/pasteboard_event.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace std::chrono;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AppFileService::ModuleRemoteFileShare;
namespace OHOS {
namespace {
    const int32_t INT32_TEN = 10;
    const int32_t INT32_ZERO = 0;
    const int32_t INT32_TWO = 2;
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
    const int32_t CONTROL_TYPE_ALLOW_SEND_RECEIVE = 1;
    const int32_t DEVICE_COLLABORATION_UID = 5521;
    const std::string RANDOM_STRING = "TEST_string_111";
    const std::string URI_STRING = "https://www.a.com/";
    const std::string HTML_STRING =
        R"(<!DOCTYPE html><html lang="en"><head><title>S</title></head><body><h1>H</h1></body></html>)";
    const char *MIMETYPE_TEXT_HTML = "text/html";
    const std::u16string RANDOM_U16STRING = u"TEST_string_111";
    using TestEvent = ClipPlugin::GlobalEvent;
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

static int g_recordValueByType = static_cast<int32_t>(PasteboardError::E_OK);
class PasteboardEntryGetterImpl : public IPasteboardEntryGetter {
    public:
        PasteboardEntryGetterImpl() {};
        ~PasteboardEntryGetterImpl() {};
        int32_t GetRecordValueByType(uint32_t recordId, PasteDataEntry &value)
        {
            return g_recordValueByType;
        };
        sptr<IRemoteObject> AsObject()
        {
            return nullptr;
        };
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
    virtual std::vector<std::string> GetMimeTypes() = 0;
    virtual std::string GetNetworkId() = 0;
    virtual int GetIntParameter(const std::string &key, int def, int min, int max) = 0;
    virtual uint32_t GetRemoteDeviceMinVersion() = 0;
    virtual int32_t GetDfsUriFromLocal(
        const std::string &uriStr, const int32_t &userId, struct HmdfsUriInfo &hui) = 0;
    virtual std::vector<std::shared_ptr<PasteDataRecord>> AllRecords() = 0;
    virtual bool Dump(int fd, const std::vector<std::string> &args) const = 0;
    virtual sptr<InputMethodController> GetInstance() = 0;
    virtual void RegisterCommand(std::shared_ptr<Command> &cmd) = 0;
    virtual int32_t GetDefaultInputMethod(std::shared_ptr<MiscServices::Property> &property) = 0;
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
    MOCK_METHOD0(GetMimeTypes, std::vector<std::string>());
    MOCK_METHOD1(RegisterCommand, void(std::shared_ptr<Command> &cmd));
    MOCK_METHOD0(GetNetworkId, std::string());
    MOCK_METHOD4(GetIntParameter, int(const std::string &key, int def, int min, int max));
    MOCK_METHOD0(GetRemoteDeviceMinVersion, uint32_t());
    MOCK_METHOD3(GetDfsUriFromLocal, int32_t(
        const std::string &uriStr, const int32_t &userId, struct HmdfsUriInfo &hui));
    MOCK_METHOD0(AllRecords, std::vector<std::shared_ptr<PasteDataRecord>>());
    MOCK_METHOD0(GetInstance, sptr<InputMethodController>());
    MOCK_CONST_METHOD2(Dump, bool(int fd, const std::vector<std::string> &args));
    MOCK_METHOD1(GetDefaultInputMethod, int32_t(std::shared_ptr<MiscServices::Property> &property));
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
int32_t InputMethodController::GetDefaultInputMethod(std::shared_ptr<Property> &property)
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return 0;
    }
    if (g_encodeInsert) {
        property = std::make_shared<Property>();
    }
    return interface->GetDefaultInputMethod(property);
}

PasteboardDumpHelper &PasteboardDumpHelper::GetInstance()
{
    static PasteboardDumpHelper instance;
    return instance;
}

void PasteboardDumpHelper::RegisterCommand(std::shared_ptr<Command> &cmd)
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return;
    }
    return interface->RegisterCommand(cmd);
}

sptr<InputMethodController> InputMethodController::GetInstance()
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return nullptr;
    }
    return interface->GetInstance();
}

bool PasteboardDumpHelper::Dump(int fd, const std::vector<std::string> &args) const
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return false;
    }
    return interface->Dump(fd, args);
}

std::vector<std::string> PasteData::GetMimeTypes()
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return std::vector<std::string>();
    }
    return interface->GetMimeTypes();
}

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

bool TLVWriteable::Encode(std::vector<uint8_t> &buffer, bool isRemote) const
{
    (void)isRemote;
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

int GetIntParameter(const std::string &key, int def, int min, int max)
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return INT32_NEGATIVE_NUMBER;
    }
    return interface->GetIntParameter(key, def, min, max);
}

uint32_t DistributedModuleConfig::GetRemoteDeviceMinVersion()
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return UINT_MAX;
    }
    return interface->GetRemoteDeviceMinVersion();
}

int32_t RemoteFileShare::GetDfsUriFromLocal(
    const std::string &uriStr, const int32_t &userId, struct HmdfsUriInfo &hui)
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return UINT_MAX;
    }
    return interface->GetDfsUriFromLocal(uriStr, userId, hui);
}

std::vector<std::shared_ptr<PasteDataRecord>> PasteData::AllRecords() const
{
    PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
    if (interface == nullptr) {
        return std::vector<std::shared_ptr<PasteDataRecord>>();
    }
    return interface->AllRecords();
}
}

namespace MiscServices {
    int32_t InputMethodController::SendPrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
    {
        return 0;
    }
    
    int32_t InputMethodController::ReceivePrivateCommand(const std::unordered_map<std::string, PrivateDataValue> &privateCommand)
    {
        return 0;
    }
    
    InputMethodController::InputMethodController() {}
    
    InputMethodController::~InputMethodController() {}
    
    std::string PasteboardEvent::GetNetworkId() const
    {
        PasteboardServiceInterface *interface = GetPasteboardServiceInterface();
        if (interface == nullptr) {
            return "";
        }
        return interface->GetNetworkId();
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
    tempPasteboard->clipPlugin_ =
        std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME_VAL), release);
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

/**
 * @tc.name: CloseDistributedStoreTest002
 * @tc.desc: test Func CloseDistributedStore
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CloseDistributedStoreTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int32_t user = 1;
    bool isNeedClear = true;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, IsOn()).WillRepeatedly(testing::Return(true));
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    EXPECT_EQ(tempPasteboard->GetClipPlugin(), nullptr);
    tempPasteboard->CloseDistributedStore(user, isNeedClear);
}

/**
 * @tc.name: CloseDistributedStoreTest003
 * @tc.desc: test Func CloseDistributedStore
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CloseDistributedStoreTest003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int32_t user = 1;
    bool isNeedClear = true;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, IsOn()).WillRepeatedly(testing::Return(true));
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL3;
    tempPasteboard->clipPlugin_ = nullptr;
    EXPECT_NE(tempPasteboard->GetClipPlugin(), nullptr);
    EXPECT_NE(tempPasteboard->GetClipPlugin(), nullptr);
    tempPasteboard->CloseDistributedStore(user, isNeedClear);
}

/**
 * @tc.name: CloseDistributedStoreTest004
 * @tc.desc: test Func CloseDistributedStore
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CloseDistributedStoreTest004, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int32_t user = 1;
    bool isNeedClear = true;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, IsOn()).WillRepeatedly(testing::Return(false));
    EXPECT_EQ(tempPasteboard->GetClipPlugin(), nullptr);
    tempPasteboard->CloseDistributedStore(user, isNeedClear);
}

/**
 * @tc.name: OnConfigChangeTest001
 * @tc.desc: test Func OnConfigChange
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnConfigChangeTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->OnConfigChange(false);
    EXPECT_EQ(tempPasteboard->GetCurrentAccountId(), 1);
}

/**
 * @tc.name: OnConfigChangeTest003
 * @tc.desc: test Func OnConfigChange
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnConfigChangeTest003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
    tempPasteboard->OnConfigChange(true);
}

/**
 * @tc.name: OnConfigChangeTest004
 * @tc.desc: test Func OnConfigChange
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnConfigChangeTest004, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL4;
    tempPasteboard->OnConfigChange(true);
}

/**
 * @tc.name: OnConfigChangeTest005
 * @tc.desc: test Func OnConfigChange
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnConfigChangeTest005, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL4;
    tempPasteboard->OnConfigChange(true);
}

/**
 * @tc.name: PasteboardEventSubscriberTest001
 * @tc.desc: test Func PasteboardEventSubscriber
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteboardEventSubscriberTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetNetworkId()).WillRepeatedly(testing::Return(""));
    tempPasteboard->PasteboardEventSubscriber();
}

/**
 * @tc.name: PasteboardEventSubscriberTest002
 * @tc.desc: test Func PasteboardEventSubscriber
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteboardEventSubscriberTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    std::string networkId = "networkId1";
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetNetworkId()).WillRepeatedly(testing::Return(networkId));
    int32_t pid = 1234;
    std::string key = "key1";
    ConcurrentMap<std::string, int32_t> p2pMap;
    p2pMap.Insert(key, pid);
    tempPasteboard->p2pMap_.Insert(networkId, p2pMap);
    tempPasteboard->PasteboardEventSubscriber();
    EXPECT_NE(tempPasteboard->p2pMap_.Size(), 0);
}

/**
 * @tc.name: PasteboardEventSubscriberTest003
 * @tc.desc: test Func PasteboardEventSubscriber
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteboardEventSubscriberTest003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetNetworkId()).WillRepeatedly(testing::Return("networkId1"));
    int32_t pid = 1234;
    std::string key = "key1";
    ConcurrentMap<std::string, int32_t> p2pMap;
    p2pMap.Insert(key, pid);
    tempPasteboard->p2pMap_.Insert("networkId2", p2pMap);
    tempPasteboard->PasteboardEventSubscriber();
    EXPECT_NE(tempPasteboard->p2pMap_.Size(), 0);
}

/**
 * @tc.name: SetDistributedDataTest001
 * @tc.desc: test Func SetDistributedData, IsAllowSendData is false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetDistributedDataTest001, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetIntParameter(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(INT32_NEGATIVE_NUMBER));

    PasteData pasteData;
    int32_t result = service.SetDistributedData(ACCOUNT_IDS_RANDOM, pasteData);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SetDistributedDataTest002
 * @tc.desc: test Func SetDistributedData, IsDisallowDistributed is true
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetDistributedDataTest002, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetIntParameter(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(CONTROL_TYPE_ALLOW_SEND_RECEIVE));
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(DEVICE_COLLABORATION_UID));

    PasteData pasteData;
    int32_t result = service.SetDistributedData(ACCOUNT_IDS_RANDOM, pasteData);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SetDistributedDataTest003
 * @tc.desc: test Func SetDistributedData, clipPlugin == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetDistributedDataTest003, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetIntParameter(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(CONTROL_TYPE_ALLOW_SEND_RECEIVE));
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(1234));

    service.securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    PasteData pasteData;
    int32_t result = service.SetDistributedData(ACCOUNT_IDS_RANDOM, pasteData);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SetDistributedDataTest004
 * @tc.desc: test Func SetDistributedData, return true
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetDistributedDataTest004, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetIntParameter(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(CONTROL_TYPE_ALLOW_SEND_RECEIVE));
    EXPECT_CALL(ipcMock, GetCallingUid()).WillOnce(testing::Return(1234));

    PasteData pasteData;
    int32_t result = service.SetDistributedData(ACCOUNT_IDS_RANDOM, pasteData);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SetCurrentDataTest001
 * @tc.desc: test Func SetCurrentData, clipPlugin == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetCurrentDataTest001, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;

    service.securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    TestEvent event;
    PasteData data;
    bool result = service.SetCurrentData(event, data);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SetCurrentDataTest002
 * @tc.desc: test Func SetCurrentData, needFull is true, encode is false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetCurrentDataTest002, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> mock;
    service.securityLevel_.securityLevel_ = DATA_SEC_LEVEL3;
    std::string PLUGIN_NAME_VAL = "mock_clip";
    auto release = [&PLUGIN_NAME_VAL, this](ClipPlugin *plugin) {
        ClipPlugin::DestroyPlugin(PLUGIN_NAME_VAL, plugin);
    };
    std::shared_ptr<ClipPlugin> clipPlugin =
        std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME_VAL), release);
    service.clipPlugin_ = clipPlugin;
    TestEvent event;
    PasteData data;
    data.AddTextRecord(RANDOM_STRING);
    data.SetDelayRecord(true);
    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, GetRemoteDeviceMinVersion()).WillOnce(testing::Return(DevProfile::FIRST_VERSION));

    bool result = service.SetCurrentData(event, data);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SetCurrentDataTest003
 * @tc.desc: test Func SetCurrentData, data.IsDelayRecord() && !needFull
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetCurrentDataTest003, TestSize.Level0)
{
    PasteboardService service;
    NiceMock<PasteboardServiceInterfaceMock> mock;
    service.securityLevel_.securityLevel_ = DATA_SEC_LEVEL3;
    std::string PLUGIN_NAME_VAL = "mock_clip";
    auto release = [&PLUGIN_NAME_VAL, this](ClipPlugin *plugin) {
        ClipPlugin::DestroyPlugin(PLUGIN_NAME_VAL, plugin);
    };
    std::shared_ptr<ClipPlugin> clipPlugin =
        std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME_VAL), release);
    service.clipPlugin_ = clipPlugin;
    TestEvent event;
    PasteData data;
    data.AddTextRecord(RANDOM_STRING);
    data.SetDelayRecord(true);
    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));
    EXPECT_CALL(mock, GetRemoteDeviceMinVersion()).WillOnce(testing::Return(DevProfile::FIRST_VERSION + 1));
    EXPECT_CALL(mock, Encode(testing::_)).WillOnce(Return(true));
    bool result = service.SetCurrentData(event, data);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetDistributedDelayEntryTest001
 * @tc.desc: test Func GetDistributedDelayEntry
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDistributedDelayEntryTest001, TestSize.Level0)
{
    PasteboardService service;
    TestEvent event;
    event.user = ACCOUNT_IDS_RANDOM;
    auto data = std::make_shared<PasteData>();
    EXPECT_NE(data, nullptr);
    auto record = PasteDataRecord::NewPlainTextRecord(RANDOM_STRING);
    data->SetDataId(service.delayDataId_);
    data->AddRecord(record);
    data->SetRemote(true);
    data->records_.emplace_back(record);
    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, data);
    event.dataId == data->GetDataId();
    std::vector<uint8_t> rawData;
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetMimeType(MIMETYPE_TEXT_PLAIN);

    sptr<PasteboardEntryGetterImpl> pasteboardEntryGetterImpl = sptr<PasteboardEntryGetterImpl>::MakeSptr();
    sptr<PasteboardService::EntryGetterDeathRecipient> deathRecipient =
        sptr<PasteboardService::EntryGetterDeathRecipient>::MakeSptr(ACCOUNT_IDS_RANDOM, service);
    service.entryGetters_.InsertOrAssign(
        ACCOUNT_IDS_RANDOM, std::make_pair(pasteboardEntryGetterImpl, deathRecipient));

    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetRecordById(testing::_)).WillOnce(Return(record));
    EXPECT_CALL(mock, GetEntry(testing::_)).WillOnce(Return(entry));
    EXPECT_CALL(mock, Encode(testing::_)).WillOnce(Return(true));
    service.GetDistributedDelayEntry(event, data->GetRecordId(), RANDOM_STRING, rawData);
}

/**
 * @tc.name: GetDistributedDelayEntryTest002
 * @tc.desc: test Func GetDistributedDelayEntry
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDistributedDelayEntryTest002, TestSize.Level0)
{
    PasteboardService service;
    TestEvent event;
    event.user = ACCOUNT_IDS_RANDOM;
    auto data = std::make_shared<PasteData>();
    EXPECT_NE(data, nullptr);
    OHOS::Uri uri("uri");
    auto record = PasteDataRecord::NewUriRecord(uri);
    data->SetDataId(service.delayDataId_);
    data->AddRecord(record);
    data->SetRemote(true);
    data->records_.emplace_back(record);
    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, data);
    event.dataId == data->GetDataId();
    std::vector<uint8_t> rawData;
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetMimeType(MIMETYPE_TEXT_URI);

    sptr<PasteboardEntryGetterImpl> pasteboardEntryGetterImpl = sptr<PasteboardEntryGetterImpl>::MakeSptr();
    sptr<PasteboardService::EntryGetterDeathRecipient> deathRecipient =
        sptr<PasteboardService::EntryGetterDeathRecipient>::MakeSptr(ACCOUNT_IDS_RANDOM, service);
    service.entryGetters_.InsertOrAssign(
        ACCOUNT_IDS_RANDOM, std::make_pair(pasteboardEntryGetterImpl, deathRecipient));

    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetRecordById(testing::_)).WillOnce(Return(record));
    EXPECT_CALL(mock, GetEntry(testing::_)).WillOnce(Return(entry));
    service.GetDistributedDelayEntry(event, data->GetRecordId(), RANDOM_STRING, rawData);
}

/**
 * @tc.name: GetDistributedDelayEntryTest003
 * @tc.desc: test Func GetDistributedDelayEntry
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDistributedDelayEntryTest003, TestSize.Level0)
{
    PasteboardService service;
    TestEvent event;
    event.user = ACCOUNT_IDS_RANDOM;
    auto data = std::make_shared<PasteData>();
    EXPECT_NE(data, nullptr);
    auto record = PasteDataRecord::NewHtmlRecord(HTML_STRING);
    data->SetDataId(service.delayDataId_);
    data->AddRecord(record);
    data->SetRemote(true);
    data->records_.emplace_back(record);
    service.clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, data);
    event.dataId == data->GetDataId();
    std::vector<uint8_t> rawData;
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetMimeType(MIMETYPE_TEXT_HTML);

    sptr<PasteboardEntryGetterImpl> pasteboardEntryGetterImpl = sptr<PasteboardEntryGetterImpl>::MakeSptr();
    sptr<PasteboardService::EntryGetterDeathRecipient> deathRecipient =
        sptr<PasteboardService::EntryGetterDeathRecipient>::MakeSptr(ACCOUNT_IDS_RANDOM, service);
    service.entryGetters_.InsertOrAssign(
        ACCOUNT_IDS_RANDOM, std::make_pair(pasteboardEntryGetterImpl, deathRecipient));

    NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetRecordById(testing::_)).WillOnce(Return(record));
    EXPECT_CALL(mock, GetEntry(testing::_)).WillOnce(Return(entry));
    service.GetDistributedDelayEntry(event, data->GetRecordId(), RANDOM_STRING, rawData);
}

/**
 * @tc.name: ProcessDistributedDelayUriTest003
 * @tc.desc: test Func ProcessDistributedDelayUri
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ProcessDistributedDelayUriTest003, TestSize.Level0)
{
    PasteboardService service;

    PasteData data;
    OHOS::Uri uri("/");
    auto record = PasteDataRecord::NewUriRecord(uri);
    data.AddRecord(record);

    auto entry = record->GetEntryByMimeType(MIMETYPE_TEXT_URI);
    auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, entry->GetMimeType());
    entry->SetUtdId(utdId);
    std::vector<uint8_t> rawData;

    NiceMock<PasteboardServiceInterfaceMock> mock;
    struct HmdfsUriInfo dfsUri;
    EXPECT_CALL(mock, GetDfsUriFromLocal(testing::_, testing::_, testing::_))
        .WillOnce([](auto, auto, struct HmdfsUriInfo &hui) {
            hui.uriStr = "uri";
            hui.fileSize = 0;
            return 0;
        });
    EXPECT_CALL(mock, Encode(testing::_)).WillOnce(Return(true));
    int32_t ret = service.ProcessDistributedDelayUri(ACCOUNT_IDS_RANDOM, data, *entry, rawData);
    EXPECT_EQ(ret, static_cast<int32_t>(PasteboardError::E_OK));
}

/**
 * @tc.name: GetClipPluginTest001
 * @tc.desc: test Func GetClipPlugin
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetClipPluginTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;

    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));

    auto result = tempPasteboard->GetClipPlugin();
    EXPECT_EQ(result, nullptr);
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
}

/**
 * @tc.name: GetClipPluginTest002
 * @tc.desc: test Func GetClipPlugin
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetClipPluginTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL3;
    tempPasteboard->clipPlugin_ = nullptr;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;

    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));

    auto result = tempPasteboard->GetClipPlugin();
    EXPECT_NE(result, nullptr);
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
}

/**
 * @tc.name: GetClipPluginTest003
 * @tc.desc: test Func GetClipPlugin
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetClipPluginTest003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL3;
    tempPasteboard->clipPlugin_ = nullptr;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;

    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(false));

    auto result = tempPasteboard->GetClipPlugin();
    EXPECT_EQ(result, nullptr);
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
}

/**
 * @tc.name: GetClipPluginTest004
 * @tc.desc: test Func GetClipPlugin
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetClipPluginTest004, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL3;
    std::string PLUGIN_NAME_VAL = "distributed_clip";
    auto release = [&PLUGIN_NAME_VAL, this](ClipPlugin *plugin) {
        ClipPlugin::DestroyPlugin(PLUGIN_NAME_VAL, plugin);
    };
    std::shared_ptr<ClipPlugin> clipPlugin =
        std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME_VAL), release);
    tempPasteboard->clipPlugin_ = clipPlugin;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;

    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));

    auto result = tempPasteboard->GetClipPlugin();
    EXPECT_EQ(result, clipPlugin);
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
}

/**
 * @tc.name: CleanDistributedDataTest001
 * @tc.desc: test Func CleanDistributedData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CleanDistributedDataTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    tempPasteboard->clipPlugin_ = nullptr;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;

    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));

    tempPasteboard->CleanDistributedData(ACCOUNT_IDS_RANDOM);
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
}

/**
 * @tc.name: ChangeStoreStatusTest001
 * @tc.desc: test Func ChangeStoreStatus
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ChangeStoreStatusTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    tempPasteboard->clipPlugin_ = nullptr;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;

    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));

    tempPasteboard->ChangeStoreStatus(ACCOUNT_IDS_RANDOM);
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
}


/**
 * @tc.name: GetFullDelayPasteDataTest001
 * @tc.desc: test Func GetFullDelayPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetFullDelayPasteDataTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    tempPasteboard->clipPlugin_ = nullptr;

    sptr<PasteboardEntryGetterImpl> pasteboardEntryGetterImpl = sptr<PasteboardEntryGetterImpl>::MakeSptr();
    sptr<PasteboardService::EntryGetterDeathRecipient> deathRecipient =
        sptr<PasteboardService::EntryGetterDeathRecipient>::MakeSptr(ACCOUNT_IDS_RANDOM, *tempPasteboard);
    tempPasteboard->entryGetters_.InsertOrAssign(
        ACCOUNT_IDS_RANDOM, std::make_pair(pasteboardEntryGetterImpl, deathRecipient));

    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    PasteData data;
    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    record->SetDelayRecordFlag(false);
    record->SetRecordId(11);
    record->mimeType_ = MIMETYPE_TEXT_HTML;
    std::vector<std::shared_ptr<PasteDataRecord>> records_;
    records_.push_back(record);

    EXPECT_CALL(mock, IsOn()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, AllRecords()).WillRepeatedly(testing::Return(records_));

    auto result = tempPasteboard->GetFullDelayPasteData(ACCOUNT_IDS_RANDOM, data);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::E_OK));
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
}

/**
 * @tc.name: GetFullDelayPasteDataTest002
 * @tc.desc: test Func GetFullDelayPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetFullDelayPasteDataTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    tempPasteboard->clipPlugin_ = nullptr;

    sptr<PasteboardEntryGetterImpl> pasteboardEntryGetterImpl = sptr<PasteboardEntryGetterImpl>::MakeSptr();
    sptr<PasteboardService::EntryGetterDeathRecipient> deathRecipient =
        sptr<PasteboardService::EntryGetterDeathRecipient>::MakeSptr(ACCOUNT_IDS_RANDOM, *tempPasteboard);
    tempPasteboard->entryGetters_.InsertOrAssign(
        ACCOUNT_IDS_RANDOM, std::make_pair(pasteboardEntryGetterImpl, deathRecipient));

    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    PasteData data;
    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    record->SetDelayRecordFlag(true);
    record->SetRecordId(11);
    record->mimeType_ = MIMETYPE_TEXT_HTML;
    std::vector<std::shared_ptr<PasteDataEntry>> entries;
    std::shared_ptr<PasteDataEntry> entry = std::make_shared<PasteDataEntry>();
    auto newObject = std::make_shared<Object>();
    entry->SetValue(newObject);
    entries.emplace_back(entry);
    record->entries_.push_back(entry);
    std::vector<std::shared_ptr<PasteDataRecord>> records_;
    records_.push_back(record);

    EXPECT_CALL(mock, IsOn()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, AllRecords()).WillRepeatedly(testing::Return(records_));

    auto result = tempPasteboard->GetFullDelayPasteData(ACCOUNT_IDS_RANDOM, data);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::E_OK));
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
}

/**
 * @tc.name: GetFullDelayPasteDataTest003
 * @tc.desc: test Func GetFullDelayPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetFullDelayPasteDataTest003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    tempPasteboard->clipPlugin_ = nullptr;
    g_recordValueByType = static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);

    sptr<PasteboardEntryGetterImpl> pasteboardEntryGetterImpl = sptr<PasteboardEntryGetterImpl>::MakeSptr();
    sptr<PasteboardService::EntryGetterDeathRecipient> deathRecipient =
        sptr<PasteboardService::EntryGetterDeathRecipient>::MakeSptr(ACCOUNT_IDS_RANDOM, *tempPasteboard);
    tempPasteboard->entryGetters_.InsertOrAssign(
        ACCOUNT_IDS_RANDOM, std::make_pair(pasteboardEntryGetterImpl, deathRecipient));

    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    PasteData data;
    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    record->SetDelayRecordFlag(true);
    record->SetRecordId(11);
    record->mimeType_ = MIMETYPE_TEXT_HTML;
    std::vector<std::shared_ptr<PasteDataEntry>> entries;
    std::shared_ptr<PasteDataEntry> entry = std::make_shared<PasteDataEntry>();
    EntryValue var;
    entry->SetValue(var);
    entries.emplace_back(entry);
    record->entries_.push_back(entry);
    std::vector<std::shared_ptr<PasteDataRecord>> records_;
    records_.push_back(record);

    EXPECT_CALL(mock, IsOn()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, AllRecords()).WillRepeatedly(testing::Return(records_));

    auto result = tempPasteboard->GetFullDelayPasteData(ACCOUNT_IDS_RANDOM, data);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::E_OK));
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
    g_recordValueByType = static_cast<int32_t>(PasteboardError::E_OK);
}

/**
 * @tc.name: GetFullDelayPasteDataTest004
 * @tc.desc: test Func GetFullDelayPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetFullDelayPasteDataTest004, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    ASSERT_NE(tempPasteboard, nullptr);

    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    tempPasteboard->clipPlugin_ = nullptr;
    g_recordValueByType = static_cast<int32_t>(PasteboardError::E_OK);

    sptr<PasteboardEntryGetterImpl> pasteboardEntryGetterImpl = sptr<PasteboardEntryGetterImpl>::MakeSptr();
    sptr<PasteboardService::EntryGetterDeathRecipient> deathRecipient =
        sptr<PasteboardService::EntryGetterDeathRecipient>::MakeSptr(ACCOUNT_IDS_RANDOM, *tempPasteboard);
    tempPasteboard->entryGetters_.InsertOrAssign(
        ACCOUNT_IDS_RANDOM, std::make_pair(pasteboardEntryGetterImpl, deathRecipient));

    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    PasteData data;
    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    record->SetDelayRecordFlag(true);
    record->SetRecordId(11);
    record->mimeType_ = MIMETYPE_TEXT_HTML;
    std::vector<std::shared_ptr<PasteDataEntry>> entries;
    std::shared_ptr<PasteDataEntry> entry = std::make_shared<PasteDataEntry>();
    EntryValue var;
    entry->SetValue(var);
    entry->SetMimeType(MIMETYPE_TEXT_HTML);
    entries.emplace_back(entry);
    record->entries_.push_back(entry);
    std::vector<std::shared_ptr<PasteDataRecord>> records_;
    records_.push_back(record);

    EXPECT_CALL(mock, IsOn()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(mock, AllRecords()).WillRepeatedly(testing::Return(records_));

    auto result = tempPasteboard->GetFullDelayPasteData(ACCOUNT_IDS_RANDOM, data);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::E_OK));
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL0;
    g_recordValueByType = static_cast<int32_t>(PasteboardError::E_OK);
}

/**
 * @tc.name: GenerateDataTypeTest002
 * @tc.desc: test Func GenerateDataType when GetMimeTypes is return empty
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GenerateDataTypeTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    PasteData pasteData;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    std::vector<std::string> mimeTypes;
    EXPECT_CALL(ipcMock, GetMimeTypes()).WillOnce(testing::Return(mimeTypes));
    
    uint8_t result = tempPasteboard->GenerateDataType(pasteData);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: GenerateDataTypeTest003
 * @tc.desc: test Func GenerateDataType when in tempPasteboard->typeMap_ not find mimeTypes data
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GenerateDataTypeTest003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    PasteData pasteData;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    std::vector<std::string> mimeTypes;
    mimeTypes.push_back(RANDOM_STRING);
    tempPasteboard->typeMap_.erase(RANDOM_STRING);
    EXPECT_CALL(ipcMock, GetMimeTypes()).WillOnce(testing::Return(mimeTypes));
    
    uint8_t result = tempPasteboard->GenerateDataType(pasteData);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: GenerateDataTypeTest004
 * @tc.desc: test Func GenerateDataType when find mimeTypes and is MIMETYPE_TEXT_HTML
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GenerateDataTypeTest004, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    PasteData pasteData;
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    std::vector<std::string> mimeTypes;
    pasteData.SetTag(PasteData::WEBVIEW_PASTEDATA_TAG);
    mimeTypes.push_back(MIMETYPE_TEXT_HTML);
    EXPECT_CALL(ipcMock, GetMimeTypes()).WillOnce(testing::Return(mimeTypes));
    
    uint8_t result = tempPasteboard->GenerateDataType(pasteData);
    EXPECT_EQ(result, INT32_TWO);
}

/**
 * @tc.name: GetDistributedData002
 * @tc.desc: test Func GetDistributedData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDistributedData002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    TestEvent event;
    int32_t user = 0;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL1;
    std::pair<std::shared_ptr<PasteData>, PasteDateResult> result = tempPasteboard->GetDistributedData(event, user);
    EXPECT_EQ(result.second.errorCode, static_cast<int32_t>(PasteboardError::REMOTE_TASK_ERROR));
}

/**
 * @tc.name: GetDistributedData003
 * @tc.desc: test Func GetDistributedData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDistributedData003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    TestEvent event;
    int32_t user = 0;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    auto release = [](ClipPlugin *plugin) {
        ClipPlugin::DestroyPlugin(RANDOM_STRING, plugin);
    };
    tempPasteboard->clipPlugin_ = std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(RANDOM_STRING), release);
    EXPECT_CALL(mock, IsOn()).WillOnce(testing::Return(true));
    tempPasteboard->securityLevel_.securityLevel_ = DATA_SEC_LEVEL4;
    std::pair<std::shared_ptr<PasteData>, PasteDateResult> result = tempPasteboard->GetDistributedData(event, user);
    EXPECT_EQ(result.second.errorCode, static_cast<int32_t>(PasteboardError::E_OK));
    tempPasteboard->clipPlugin_ = nullptr;
}

/**
 * @tc.name: GetPasteDataDot002
 * @tc.desc: test Func GetPasteDataDot
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetPasteDataDot002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    PasteData pasteData;
    pasteData.SetRemote(false);
    EXPECT_NO_FATAL_FAILURE(tempPasteboard->GetPasteDataDot(pasteData, RANDOM_STRING));
}

/**
 * @tc.name: GetPasteDataDot003
 * @tc.desc: test Func GetPasteDataDot
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetPasteDataDot003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    PasteData pasteData;
    pasteData.SetRemote(true);
    EXPECT_NO_FATAL_FAILURE(tempPasteboard->GetPasteDataDot(pasteData, RANDOM_STRING));
}

/**
 * @tc.name: DumpHistory002
 * @tc.desc: test Func DumpHistory
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DumpHistory002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->dataHistory_.clear();
    tempPasteboard->dataHistory_.push_back(RANDOM_STRING);
    std::string history = tempPasteboard->DumpHistory();
    EXPECT_TRUE(history.find("Access history last ten times: ") != std::string::npos);
    tempPasteboard->dataHistory_.clear();
}

/**
 * @tc.name: DumpHistory003
 * @tc.desc: test Func DumpHistory
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DumpHistory003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->dataHistory_.clear();
    std::string history = tempPasteboard->DumpHistory();
    EXPECT_TRUE(history.find("Access history fail! dataHistory_ no data.") != std::string::npos);
}

/**
 * @tc.name: Dump002
 * @tc.desc: test Func Dump when uid > maxUid
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, Dump002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int fd = INT_ONE;
    std::vector<std::u16string> args;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetCallingUid).WillOnce(testing::Return(UINT32_EXCEPTION_APPID));
    int result = tempPasteboard->Dump(fd, args);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: Dump003
 * @tc.desc: test Func Dump when PasteboardDumpHelper::GetInstance().Dump return false
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, Dump003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int fd = INT_ONE;
    std::vector<std::u16string> args;
    args.push_back(RANDOM_U16STRING);
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetCallingUid).WillOnce(testing::Return(INT_THREETHREETHREE));
    EXPECT_CALL(mock, Dump).WillOnce(testing::Return(false));
    int result = tempPasteboard->Dump(fd, args);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: Dump004
 * @tc.desc: test Func Dump when PasteboardDumpHelper::GetInstance().Dump return true
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, Dump004, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int fd = INT_ONE;
    std::vector<std::u16string> args;
    args.push_back(RANDOM_U16STRING);
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetCallingUid).WillOnce(testing::Return(INT_THREETHREETHREE));
    EXPECT_CALL(mock, Dump).WillOnce(testing::Return(true));
    int result = tempPasteboard->Dump(fd, args);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: IsNeedThaw002
 * @tc.desc: test Func IsNeedThaw when InputMethodController::GetInstance return nullptr
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsNeedThaw002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetInstance).WillOnce(testing::Return(nullptr));
    bool result = tempPasteboard->IsNeedThaw();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsNeedThaw003
 * @tc.desc: test Func IsNeedThaw when imc->GetDefaultInputMethod is error
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsNeedThaw003, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    sptr<InputMethodController> instance_ = new (std::nothrow) InputMethodController();
    g_encodeInsert = true;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetInstance).WillOnce(testing::Return(instance_));
    EXPECT_CALL(mock, GetDefaultInputMethod).WillOnce(testing::Return(INT32_NEGATIVE_NUMBER));
    bool result = tempPasteboard->IsNeedThaw();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsNeedThaw004
 * @tc.desc: test Func IsNeedThaw when property == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsNeedThaw004, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    sptr<InputMethodController> instance_ = new (std::nothrow) InputMethodController();
    g_encodeInsert = false;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetInstance).WillOnce(testing::Return(instance_));
    EXPECT_CALL(mock, GetDefaultInputMethod).WillOnce(testing::Return(0));
    bool result = tempPasteboard->IsNeedThaw();
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsNeedThaw005
 * @tc.desc: test Func IsNeedThaw when validity
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsNeedThaw005, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    sptr<InputMethodController> instance_ = new (std::nothrow) InputMethodController();
    g_encodeInsert = true;
    testing::NiceMock<PasteboardServiceInterfaceMock> mock;
    EXPECT_CALL(mock, GetInstance).WillOnce(testing::Return(instance_));
    EXPECT_CALL(mock, GetDefaultInputMethod).WillOnce(testing::Return(0));
    bool result = tempPasteboard->IsNeedThaw();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: SetAppShareOptionsTest004
 * @tc.desc: test Func SetAppShareOptions
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetAppShareOptionsTest004, TestSize.Level0)
{
    PasteboardService service;
    int32_t shareOptions = 0;

    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingFullTokenID).WillOnce(testing::Return(4294967296));
    EXPECT_CALL(ipcMock, GetCallingTokenID).WillOnce(testing::Return(1000));
    int32_t result = service.SetAppShareOptions(shareOptions);
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: SetAppShareOptionsTest005
 * @tc.desc: test Func SetAppShareOptions
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetAppShareOptionsTest005, TestSize.Level0)
{
    PasteboardService service;
    int32_t shareOptions = 0;

    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingFullTokenID).WillOnce(testing::Return(4294967296));
    EXPECT_CALL(ipcMock, GetCallingTokenID).WillOnce(testing::Return(1000));
    ShareOption testOption = InApp;
    PasteboardService::GlobalShareOption option = {.source = PasteboardService::GlobalShareOptionSource::APP,
        .shareOption = testOption};
    service.globalShareOptions_.InsertOrAssign(1000, option); 
    int32_t result = service.SetAppShareOptions(shareOptions);
    ASSERT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_OPERATION_ERROR));
}

/**
 * @tc.name: RemoveAppShareOptionsTest001
 * @tc.desc: test Func RemoveAppShareOptions
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemoveAppShareOptionsTest001, TestSize.Level0)
{
    PasteboardService service;

    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, VerifyAccessToken).WillOnce(Return(PermissionState::PERMISSION_DENIED));
    EXPECT_CALL(ipcMock, GetCallingFullTokenID).WillOnce(testing::Return(123));
    EXPECT_CALL(ipcMock, GetCallingTokenID).WillOnce(testing::Return(1000));
    int32_t result = service.RemoveAppShareOptions();
    ASSERT_EQ(result, static_cast<int32_t>(PasteboardError::PERMISSION_VERIFICATION_ERROR));
}

/**
 * @tc.name: RemoveAppShareOptionsTest002
 * @tc.desc: test Func RemoveAppShareOptions
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemoveAppShareOptionsTest002, TestSize.Level0)
{
    PasteboardService service;

    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingFullTokenID).WillOnce(testing::Return(4294967296));
    EXPECT_CALL(ipcMock, GetCallingTokenID).WillOnce(testing::Return(1000));
    int32_t result = service.RemoveAppShareOptions();
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: RemoveAppShareOptionsTest003
 * @tc.desc: test Func RemoveAppShareOptions
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemoveAppShareOptionsTest003, TestSize.Level0)
{
    PasteboardService service;

    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingFullTokenID).WillOnce(testing::Return(4294967296));
    EXPECT_CALL(ipcMock, GetCallingTokenID).WillOnce(testing::Return(1000));
    ShareOption testOption = InApp;
    PasteboardService::GlobalShareOption option = {.source = PasteboardService::GlobalShareOptionSource::APP,
        .shareOption = testOption};
    service.globalShareOptions_.InsertOrAssign(1000, option); 
    int32_t result = service.RemoveAppShareOptions();
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: RemoveAppShareOptionsTest004
 * @tc.desc: test Func RemoveAppShareOptions
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemoveAppShareOptionsTest004, TestSize.Level0)
{
    PasteboardService service;

    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    EXPECT_CALL(ipcMock, GetCallingFullTokenID).WillOnce(testing::Return(4294967296));
    EXPECT_CALL(ipcMock, GetCallingTokenID).WillOnce(testing::Return(1000));
    ShareOption testOption = InApp;
    PasteboardService::GlobalShareOption option = {.source = PasteboardService::GlobalShareOptionSource::MDM,
        .shareOption = testOption};
    service.globalShareOptions_.InsertOrAssign(1000, option); 
    int32_t result = service.RemoveAppShareOptions();
    ASSERT_EQ(result, 0);
}

/**
 * @tc.name: CheckMdmShareOptionTest001
 * @tc.desc: test Func CheckMdmShareOption
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CheckMdmShareOptionTest001, TestSize.Level0)
{
    PasteboardService service;
    PasteData pasteData;

    pasteData.SetTokenId(1000);
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    ShareOption testOption = InApp;
    PasteboardService::GlobalShareOption option = {.source = PasteboardService::GlobalShareOptionSource::MDM,
        .shareOption = testOption};
    service.globalShareOptions_.InsertOrAssign(1000, option);
    int32_t result = service.CheckMdmShareOption(pasteData);
    ASSERT_EQ(result, true);
}

/**
 * @tc.name: CheckMdmShareOptionTest002
 * @tc.desc: test Func CheckMdmShareOption
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CheckMdmShareOptionTest002, TestSize.Level0)
{
    PasteboardService service;
    PasteData pasteData;

    NiceMock<PasteboardServiceInterfaceMock> ipcMock;
    int32_t result = service.CheckMdmShareOption(pasteData);
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: SetPasteboardHistoryTest001
 * @tc.desc: test Func SetPasteboardHistory
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteboardHistoryTest001, TestSize.Level0)
{
    PasteboardService service;
    HistoryInfo info;
    for (int i = 1; i <= INT32_TEN; i++) {
        info.time = std::to_string(i);
        info.bundleName = "app" + std::to_string(i);
        info.state = "COPY";
        info.remote = "";
        service.SetPasteboardHistory(info);
    }

    ASSERT_EQ(service.dataHistory_.size(), INT32_TEN);
    info.time = "11";
    info.bundleName = "app11";
    NiceMock<PasteboardServiceInterfaceMock> ipcMock;

    int32_t result = service.SetPasteboardHistory(info);
    ASSERT_EQ(service.dataHistory_.size(), INT32_TEN);
    
    ASSERT_EQ(result, true);
    EXPECT_TRUE(service.dataHistory_.front().find("2 ") == 0);
    EXPECT_TRUE(service.dataHistory_.back().find("11 ") == 0);
}
}
} // namespace OHOS::MiscServices