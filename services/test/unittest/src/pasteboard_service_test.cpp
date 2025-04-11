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

#include <gtest/gtest.h>

#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_service.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace std::chrono;
using namespace OHOS::Security::AccessToken;

namespace OHOS {
namespace {
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
    const int32_t ACCOUNT_IDS_RANDOM = 1121;
    const uint32_t UINT32_ONE = 1;
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

class PasteboardEntryGetterImpl : public IPasteboardEntryGetter {
public:
    PasteboardEntryGetterImpl() {};
    ~PasteboardEntryGetterImpl() {};
    int32_t GetRecordValueByType(uint32_t recordId, PasteDataEntry &value) { return 0; };
    sptr<IRemoteObject> AsObject() { return nullptr; };
};

class PasteboardDelayGetterImpl : public IPasteboardDelayGetter {
public:
    PasteboardDelayGetterImpl() {};
    ~PasteboardDelayGetterImpl() {};
    void GetPasteData(const std::string &type, PasteData &data) {};
    void GetUnifiedData(const std::string &type, UDMF::UnifiedData &data) {};
    sptr<IRemoteObject> AsObject() { return nullptr; };
};

class RemoteObjectTest : public IRemoteObject {
public:
    explicit RemoteObjectTest(std::u16string descriptor) : IRemoteObject(descriptor) {}
    ~RemoteObjectTest() {}

    int32_t GetObjectRefCount() { return 0; }
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
                    MessageOption &option) { return 0; }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; }
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; }
};

class PasteboardServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    using TestEvent = ClipPlugin::GlobalEvent;
};

void PasteboardServiceTest::SetUpTestCase(void) { }

void PasteboardServiceTest::TearDownTestCase(void) { }

void PasteboardServiceTest::SetUp(void) { }

void PasteboardServiceTest::TearDown(void) { }

namespace MiscServices {
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
    EXPECT_EQ(testCount, UINT32_MAX);
    tempPasteboard->IncreaseChangeCount(userId);
    it = tempPasteboard->clipChangeCount_.Find(userId);
    if (it.first) {
        testCount = it.second;
    }
    EXPECT_EQ(testCount, 0);
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
    EXPECT_EQ(testCount, 0);
    tempPasteboard->currentUserId_ = 10;
    auto userId = tempPasteboard->GetCurrentAccountId();
    tempPasteboard->IncreaseChangeCount(userId);
    tempPasteboard->GetChangeCount(testCount);
    EXPECT_EQ(testCount, 1);
    tempPasteboard->currentUserId_ = 100;
    tempPasteboard->GetChangeCount(testCount);
    EXPECT_EQ(testCount, 0);
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
    EXPECT_EQ(tempPasteboard->IsDisallowDistributed(), false);
}

/**
 * @tc.name: ClearTest001
 * @tc.desc: test Func Clear
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ClearTest001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->currentUserId_ = ACCOUNT_IDS_RANDOM;
    service->clips_.InsertOrAssign(ACCOUNT_IDS_RANDOM, std::make_shared<PasteData>());
    int32_t result = service->Clear();
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SubscribeEntityObserverTest001
 * @tc.desc: test Func SubscribeEntityObserver
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SubscribeEntityObserverTest001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    EntityType entityType = EntityType::ADDRESS;
    uint32_t expectedDataLength = MAX_RECOGNITION_LENGTH;
    const sptr<IEntityRecognitionObserver> observer = sptr<MyTestEntityRecognitionObserver>::MakeSptr();

    int32_t result = service->SubscribeEntityObserver(entityType, expectedDataLength, observer);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: UnsubscribeEntityObserverTest001
 * @tc.desc: test Func UnsubscribeEntityObserver
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeEntityObserverTest001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    EntityType entityType = EntityType::ADDRESS;
    uint32_t expectedDataLength = MAX_RECOGNITION_LENGTH;
    const sptr<IEntityRecognitionObserver> observer = sptr<MyTestEntityRecognitionObserver>::MakeSptr();

    int32_t result = service->UnsubscribeEntityObserver(entityType, expectedDataLength, observer);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetChangeCountTest001
 * @tc.desc: test Func GetChangeCount
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetChangeCountTest001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    uint32_t changeCount = 0;
    int32_t result = service->GetChangeCount(changeCount);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: InitScreenStatusTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, InitScreenStatusTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->InitScreenStatus();
}


/**
 * @tc.name: WriteRawDataTest001
 * @tc.desc: test Func WriteRawData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, WriteRawDataTest001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    char rawData[] = "testData";
    int32_t fd = INT32_NEGATIVE_NUMBER;
    bool result = service->WriteRawData(rawData, sizeof(rawData), fd);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: PasteStart001
 * @tc.desc: test Func PasteStart
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteStart001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->ffrtTimer_ = nullptr;
    std::string pasteId;
    int32_t result = service->PasteStart(pasteId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: PasteStartTest001
 * @tc.desc: test Func PasteStart
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteStart002, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->ffrtTimer_ = std::make_shared<FFRTTimer>();
    std::string pasteId;
    int32_t result = service->PasteStart(pasteId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: PasteCompleteTest001
 * @tc.desc: test Func PasteComplete
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteComplete001, TestSize.Level0)
{
    std::string deviceId = "testId";
    std::string pasteId;
    auto service = std::make_shared<PasteboardService>();
    int32_t result = service->PasteComplete(deviceId, pasteId);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: PasteCompleteTest001
 * @tc.desc: test Func PasteComplete
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteComplete002, TestSize.Level0)
{
    std::string deviceId;
    std::string pasteId;
    auto service = std::make_shared<PasteboardService>();
    int32_t result = service->PasteComplete(deviceId, pasteId);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::NO_DATA_ERROR));
}


/**
 * @tc.name: HasPasteDataTest001
 * @tc.desc: test Func HasPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasPasteDataTest002, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->currentScreenStatus = ScreenEvent::ScreenUnlocked;
    bool flag = false;
    int32_t result = service->HasPasteData(flag);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(flag, false);
}

/**
 * @tc.name: HasPasteDataTest001
 * @tc.desc: test Func HasPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasPasteDataTest003, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->clips_.Clear();
    std::shared_ptr<PasteData> dataPtr(nullptr);
    int32_t userId = INT_ONE;
    service->clips_.InsertOrAssign(userId, dataPtr);
    bool flag = false;
    int32_t result = service->HasPasteData(flag);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: HasPasteDataTest001
 * @tc.desc: test Func HasPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HasPasteDataTest004, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->clips_.Clear();
    std::shared_ptr<PasteData> dataPtr = std::make_shared<PasteData>();
    int32_t userId = INT_ONE;
    service->clips_.InsertOrAssign(userId, dataPtr);
    bool flag = false;
    int32_t result = service->HasPasteData(flag);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: DealDataTest001
 * @tc.desc: test Func DealData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DealDataTest001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    int fd = -1;
    int64_t size;
    std::vector<uint8_t> rawData;
    PasteData data;
    int32_t result = service->DealData(fd, size, rawData, data);
    EXPECT_EQ(result, ERR_OK);
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
    EXPECT_EQ(result, ERR_OK);
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
    auto service = std::make_shared<PasteboardService>();
    service->currentUserId_ = ERROR_USERID;
    bool funcResult;
    int32_t result = service->IsRemoteData(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: IsRemoteDataTest002
 * @tc.desc: test Func IsRemoteData, currentUserId_ is INT32_MAX.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsRemoteDataTest002, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->currentUserId_ = INT32_MAX;
    bool funcResult;
    int32_t result = service->IsRemoteData(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: IsRemoteDataTest003
 * @tc.desc: test Func IsRemoteData, currentUserId_ is 0XF
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsRemoteDataTest003, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    int32_t userId = service->currentUserId_ = 0XF;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->AddTextRecord("hello");
    service->clips_.InsertOrAssign(userId, pasteData);
    bool funcResult;
    int32_t result = service->IsRemoteData(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: SetPasteDataOnlyTest001
 * @tc.desc: test Func SetPasteDataOnly, it will be return INVALID_PARAM_ERROR.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataOnlyTest001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    int64_t rawDataSize = 0;
    std::vector<uint8_t> buffer;
    int fd = 3;

    int32_t result = service->SetPasteDataOnly(fd, rawDataSize, buffer);
    EXPECT_EQ(result, static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR));
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
 * @tc.name: GetMimeTypesTest001
 * @tc.desc: test Func GetMimeTypes
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetMimeTypesTest001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->currentScreenStatus = ScreenEvent::ScreenUnlocked;
    std::vector<std::string> funcResult;
    int32_t result = service->GetMimeTypes(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetMimeTypesTest001
 * @tc.desc: test Func GetMimeTypes
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetMimeTypesTest002, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->currentUserId_ = ERROR_USERID;
    std::vector<std::string> funcResult;
    int32_t result = service->GetMimeTypes(funcResult);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetRemoteDeviceNameTest001
 * @tc.desc: test Func GetRemoteDeviceName
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRemoteDeviceName001, TestSize.Level0)
{
    auto service = std::make_shared<PasteboardService>();
    service->currentUserId_ = ERROR_USERID;
    std::string deviceName;
    bool isRemote = false;
    int32_t result = service->GetRemoteDeviceName(deviceName, isRemote);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: OnStopTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnStopTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->OnStop();
}

/**
 * @tc.name: AddSysAbilityListenerTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, AddSysAbilityListenerTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->AddSysAbilityListener();
}

/**
 * @tc.name: OnAddSystemAbilityTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnAddSystemAbilityTest001, TestSize.Level0)
{
    int32_t systemAbilityId = 1;
    const std::string &deviceId = "test_string";
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->OnAddSystemAbility(systemAbilityId, deviceId);
}

/**
 * @tc.name: OnRemoteDiedTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnRemoteDiedTest001, TestSize.Level0)
{
    wptr<IRemoteObject> deadRemote;
    constexpr int32_t userId = 111;
    PasteboardService service;
    auto tempPasteboard = std::make_shared<PasteboardService::DelayGetterDeathRecipient>(userId, service);
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->OnRemoteDied(deadRemote);
}

/**
 * @tc.name: NotifyDelayGetterDiedTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, NotifyDelayGetterDiedTest001, TestSize.Level0)
{
    constexpr int32_t userId = 111;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->NotifyDelayGetterDied(userId);
}

/**
 * @tc.name: OnRemoteDiedTest002
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnRemoteDiedTest002, TestSize.Level0)
{
    constexpr int32_t userId = 111;
    PasteboardService service;
    wptr<IRemoteObject> deadRemote;
    auto tempPasteboard = std::make_shared<PasteboardService::EntryGetterDeathRecipient>(userId, service);
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->OnRemoteDied(deadRemote);
}

/**
 * @tc.name: NotifyEntryGetterDiedTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, NotifyEntryGetterDiedTest001, TestSize.Level0)
{
    constexpr int32_t userId = 111;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->NotifyEntryGetterDied(userId);
}

/**
 * @tc.name: DMAdapterInitTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DMAdapterInitTest001, TestSize.Level0)
{
    constexpr int32_t userId = 111;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->DMAdapterInit();
}

/**
 * @tc.name: NotifySaStatusTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, NotifySaStatusTest001, TestSize.Level0)
{
    constexpr int32_t userId = 111;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->NotifySaStatus();
}

/**
 * @tc.name: ReportUeCopyEventTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ReportUeCopyEventTest001, TestSize.Level0)
{
    constexpr int32_t result = 111;
    PasteData pasteData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->ReportUeCopyEvent(pasteData, result);
}

/**
 * @tc.name: InitServiceHandlerTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, InitServiceHandlerTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->InitServiceHandler();
}

/**
 * @tc.name: NotifyEntityObserversTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, NotifyEntityObserversTest001, TestSize.Level0)
{
    std::string entity = "hello";
    uint32_t dataLength = 1;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->NotifyEntityObservers(entity, EntityType::ADDRESS, dataLength);
}

/**
 * @tc.name: GetAllPrimaryTextTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetAllPrimaryTextTest001, TestSize.Level0)
{
    PasteData pasteData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    auto ret = tempPasteboard->GetAllPrimaryText(pasteData);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: GetAllEntryPlainTextTest001
 * @tc.desc:
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetAllEntryPlainTextTest001, TestSize.Level0)
{
    uint32_t dataId = 1;
    uint32_t recordId = 1;
    std::vector<std::shared_ptr<PasteDataEntry>> tempEntries;
    std::string primaryText = "hello";
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    auto ret = tempPasteboard->GetAllEntryPlainText(dataId, recordId, tempEntries, primaryText);
    EXPECT_EQ(ret, static_cast<int32_t>(PasteboardError::E_OK));
}

/**
 * @tc.name: ExtractEntity001
 * @tc.desc: ExtractEntity001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ExtractEntity001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    std::string entity = "{\"code\": \"0\"}";
    std::string location = "location";
    tempPasteboard->ExtractEntity(entity, location);
}

/**
 * @tc.name: RecognizePasteData001
 * @tc.desc: RecognizePasteData001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RecognizePasteData001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    PasteData pasteData;
    tempPasteboard->RecognizePasteData(pasteData);
}

/**
 * @tc.name: PostProcessDelayHtmlEntry001
 * @tc.desc: PostProcessDelayHtmlEntry001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PostProcessDelayHtmlEntry001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    PasteData pasteData;
    std::string targetBundle = "targetBundle";
    PasteDataEntry entry;
    tempPasteboard->PostProcessDelayHtmlEntry(pasteData, targetBundle, entry);
}

/**
 * @tc.name: SetLocalPasteFlag001
 * @tc.desc: SetLocalPasteFlag001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetLocalPasteFlag001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    bool isCrossPaste = false;
    uint32_t tokenId = 0x123456;
    PasteData pasteData;
    tempPasteboard->SetLocalPasteFlag(isCrossPaste, tokenId, pasteData);
}

/**
 * @tc.name: GetRemoteDataTask001
 * @tc.desc: GetRemoteDataTask001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRemoteDataTask001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService::RemoteDataTaskManager> remoteDataTaskManager =
        std::make_shared<PasteboardService::RemoteDataTaskManager>();
    EXPECT_NE(remoteDataTaskManager, nullptr);
    TestEvent event;
    remoteDataTaskManager->GetRemoteDataTask(event);
}

/**
 * @tc.name: Notify001
 * @tc.desc: Notify001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, Notify001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService::RemoteDataTaskManager> remoteDataTaskManager =
        std::make_shared<PasteboardService::RemoteDataTaskManager>();
    EXPECT_NE(remoteDataTaskManager, nullptr);
    TestEvent event;
    remoteDataTaskManager->GetRemoteDataTask(event);
}

/**
 * @tc.name: WaitRemoteData001
 * @tc.desc: WaitRemoteData001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, WaitRemoteData001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService::RemoteDataTaskManager> remoteDataTaskManager =
        std::make_shared<PasteboardService::RemoteDataTaskManager>();
    EXPECT_NE(remoteDataTaskManager, nullptr);
    TestEvent event;
    remoteDataTaskManager->WaitRemoteData(event);
}

/**
 * @tc.name: ClearRemoteDataTask001
 * @tc.desc: ClearRemoteDataTask001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ClearRemoteDataTask001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService::RemoteDataTaskManager> remoteDataTaskManager =
        std::make_shared<PasteboardService::RemoteDataTaskManager>();
    EXPECT_NE(remoteDataTaskManager, nullptr);
    TestEvent event;
    remoteDataTaskManager->ClearRemoteDataTask(event);
}

/**
 * @tc.name: GetRemoteData001
 * @tc.desc: GetRemoteData001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRemoteData001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int32_t userId = 0x123456;
    TestEvent event;
    PasteData data;
    int32_t syncTime = 1000;
    tempPasteboard->GetRemoteData(userId, event, data, syncTime);
}

/**
 * @tc.name: GetRemotePasteData001
 * @tc.desc: GetRemotePasteData001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRemotePasteData001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int32_t userId = 0x123456;
    TestEvent event;
    PasteData data;
    int32_t syncTime = 1000;
    tempPasteboard->GetRemotePasteData(userId, event, data, syncTime);
}

/**
 * @tc.name: GetDelayPasteData001
 * @tc.desc: GetDelayPasteData001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDelayPasteData001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int32_t userId = 0x123456;
    PasteData data;
    tempPasteboard->GetDelayPasteData(userId, data);
}

/**
 * @tc.name: GetDelayPasteRecord001
 * @tc.desc: GetDelayPasteRecord001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDelayPasteRecord001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int32_t userId = 0x123456;
    PasteData data;
    tempPasteboard->GetDelayPasteRecord(userId, data);
}

/**
 * @tc.name: EstablishP2PLink001
 * @tc.desc: EstablishP2PLink001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, EstablishP2PLink001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    std::string networkId = "networkId";
    std::string pasteId = "pasteId";
    tempPasteboard->EstablishP2PLink(networkId, pasteId);
}

/**
 * @tc.name: CloseP2PLink001
 * @tc.desc: CloseP2PLink001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CloseP2PLink001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    std::string networkId = "networkId";
    tempPasteboard->CloseP2PLink(networkId);
}

/**
 * @tc.name: IsBundleOwnUriPermission001
 * @tc.desc: IsBundleOwnUriPermission001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsBundleOwnUriPermission001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    std::string bundleName = "bundleName";
    std::shared_ptr<Uri> uri = std::make_shared<Uri>("text/html");
    tempPasteboard->IsBundleOwnUriPermission(bundleName, *uri);
}

/**
 * @tc.name: SaveData001
 * @tc.desc: SaveData001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SaveData001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    PasteData pasteData;
    sptr<PasteboardDelayGetterImpl> delayGetter = sptr<PasteboardDelayGetterImpl>::MakeSptr();
    EXPECT_NE(delayGetter, nullptr);
    sptr<PasteboardEntryGetterImpl> entryGetter = sptr<PasteboardEntryGetterImpl>::MakeSptr();
    EXPECT_NE(entryGetter, nullptr);
    tempPasteboard->SaveData(pasteData, delayGetter, entryGetter);
}

/**
 * @tc.name: HandleDelayDataAndRecord001
 * @tc.desc: HandleDelayDataAndRecord001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, HandleDelayDataAndRecord001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    PasteData pasteData;
    sptr<PasteboardDelayGetterImpl> delayGetter = sptr<PasteboardDelayGetterImpl>::MakeSptr();
    EXPECT_NE(delayGetter, nullptr);
    sptr<PasteboardEntryGetterImpl> entryGetter = sptr<PasteboardEntryGetterImpl>::MakeSptr();
    EXPECT_NE(entryGetter, nullptr);
    AppInfo appInfo;
    tempPasteboard->HandleDelayDataAndRecord(pasteData, delayGetter, entryGetter, appInfo);
}

/**
 * @tc.name: IsBasicType001
 * @tc.desc: IsBasicType001
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsBasicType001, TestSize.Level0)
{
    std::shared_ptr<PasteboardService> tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    std::string mimeType = "text/html";
    tempPasteboard->IsBasicType(mimeType);
}

/**
 * @tc.name: GetDataSourceTest001
 * @tc.desc: GetDataSource founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDataSourceTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    std::string bundlename = "test";
    tempPasteboard->GetDataSource(bundlename);
}

/**
 * @tc.name: GetDataSourceTest002
 * @tc.desc: GetDataSource function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDataSourceTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    std::string bundleName = "test_bundle_name";
    int32_t userId = 100;
    tempPasteboard->currentUserId_ = userId;
    PasteDataProperty props_;
    auto data = std::make_shared<PasteData>();
    data->props_.isRemote = false;
    data->props_.tokenId = 1;

    tempPasteboard->clips_.Insert(userId, data);
    auto result = tempPasteboard->GetDataSource(bundleName);
    EXPECT_EQ(result, ERR_OK);
    tempPasteboard->clips_.Erase(userId);
}

/**
 * @tc.name: RemovePasteDataTest001
 * @tc.desc: RemovePasteData founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemovePasteDataTest001, TestSize.Level0)
{
    AppInfo appInfo;
    appInfo.tokenId = 123;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->RemovePasteData(appInfo);
}

/**
 * @tc.name: IsCopyableTest001
 * @tc.desc: IsCopyable founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsCopyableTest001, TestSize.Level0)
{
    auto tokenId = 123;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->IsCopyable(tokenId);
}

/**
 * @tc.name: GetAllObserversSizeTest001
 * @tc.desc: GetAllObserversSize founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetAllObserversSizeTest001, TestSize.Level0)
{
    auto userId = 123;
    auto tokenId = 123;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GetAllObserversSize(userId, tokenId);
}

/**
 * @tc.name: RemoveGlobalShareOptionTest001
 * @tc.desc: RemoveGlobalShareOption founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemoveGlobalShareOptionTest001, TestSize.Level0)
{
    std::vector<uint32_t> tokenIds = {1001, 1002, 1003};
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->RemoveGlobalShareOption(tokenIds);
}

/**
 * @tc.name: RemoveAppShareOptionsTest001
 * @tc.desc: RemoveAppShareOptions founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemoveAppShareOptionsTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->RemoveAppShareOptions();
}

/**
 * @tc.name: UpdateShareOptionTest001
 * @tc.desc: UpdateShareOption founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UpdateShareOptionTest001, TestSize.Level0)
{
    PasteData pasteData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->UpdateShareOption(pasteData);
}

/**
 * @tc.name: CheckMdmShareOptionTest001
 * @tc.desc: CheckMdmShareOption founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CheckMdmShareOptionTest001, TestSize.Level0)
{
    PasteData pasteData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->CheckMdmShareOption(pasteData);
}

/**
 * @tc.name: ThawInputMethodTest001
 * @tc.desc: ThawInputMethod founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ThawInputMethodTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->ThawInputMethod();
}

/**
 * @tc.name: IsNeedThawTest001
 * @tc.desc: IsNeedThaw founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsNeedThawTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->IsNeedThaw();
}

/**
 * @tc.name: GetDataSizeTest001
 * @tc.desc: GetDataSize founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDataSizeTest001, TestSize.Level0)
{
    PasteData data;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GetDataSize(data);
}

/**
 * @tc.name: SetPasteboardHistoryTest001
 * @tc.desc: SetPasteboardHistory founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteboardHistoryTest001, TestSize.Level0)
{
    HistoryInfo info;
    info.time = "2023-01-01";
    info.bundleName = "com.example.app";
    info.state = "copied";
    info.remote = "false";
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->SetPasteboardHistory(info);
}

/**
 * @tc.name: DumpTest001
 * @tc.desc: Dump founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DumpTest001, TestSize.Level0)
{
    auto fd = 1;
    std::vector<std::u16string> args;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->Dump(fd, args);
}

/**
 * @tc.name: GetTimeTest001
 * @tc.desc: GetTime founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetTimeTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GetTime();
}

/**
 * @tc.name: DumpHistoryTest001
 * @tc.desc: DumpHistory founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DumpHistoryTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->DumpHistory();
}

/**
 * @tc.name: DumpDataTest001
 * @tc.desc: DumpData founction test
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, DumpDataTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->DumpData();
}

/**
 * @tc.name: GetFocusedAppInfoTest001
 * @tc.desc: test Func GetFocusedAppInfo
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetFocusedAppInfoTest005, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GetFocusedAppInfo();
}

/**
 * @tc.name: SetPasteDataDotTest001
 * @tc.desc: test Func SetPasteDataDot
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetPasteDataDotTest001, TestSize.Level0)
{
    PasteData pasteData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->SetPasteDataDot(pasteData);
}

/**
 * @tc.name: GetPasteDataDotTest001
 * @tc.desc: test Func GetPasteDataDot
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetPasteDataDotTest001, TestSize.Level0)
{
    PasteData pasteData;
    std::string bundleName;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GetPasteDataDot(pasteData, bundleName);
}

/**
 * @tc.name: GetDistributedDataTest001
 * @tc.desc: test Func GetDistributedData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDistributedDataTest001, TestSize.Level0)
{
    ClipPlugin::GlobalEvent event{};
    int32_t user = ACCOUNT_IDS_RANDOM;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GetDistributedData(event, user);
}

/**
 * @tc.name: IsAllowSendDataTest001
 * @tc.desc: test Func IsAllowSendData

 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsAllowSendDataTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->IsAllowSendData();
}

/**
 * @tc.name: SetDistributedDataTest001
 * @tc.desc: test Func SetDistributedData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetDistributedDataTest001, TestSize.Level0)
{
    int32_t user = ACCOUNT_IDS_RANDOM;
    PasteData pasteData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->SetDistributedData(user, pasteData);
}

/**
 * @tc.name: SetCurrentDistributedDataTest001
 * @tc.desc: test Func SetCurrentDistributedData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetCurrentDistributedDataTest001, TestSize.Level0)
{
    PasteData pasteData;
    ClipPlugin::GlobalEvent event{};
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->SetCurrentDistributedData(pasteData, event);
}

/**
 * @tc.name: SetCurrentDataTest001
 * @tc.desc: test Func SetCurrentData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetCurrentDataTest001, TestSize.Level0)
{
    PasteData pasteData;
    ClipPlugin::GlobalEvent event{};
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->SetCurrentData(event, pasteData);
}

/**
 * @tc.name: GetDistributedDelayEntryTest001
 * @tc.desc: test Func GetDistributedDelayEntry
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDistributedDelayEntryTest001, TestSize.Level0)
{
    ClipPlugin::GlobalEvent evt{};
    uint32_t recordId = UINT32_ONE;
    std::string utdId;
    std::vector<uint8_t> rawData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GetDistributedDelayEntry(evt, recordId, utdId, rawData);
}

/**
 * @tc.name: ProcessDistributedDelayUriTest001
 * @tc.desc: test Func ProcessDistributedDelayUri
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ProcessDistributedDelayUriTest001, TestSize.Level0)
{
    int32_t userId = ACCOUNT_IDS_RANDOM;
    PasteData data;
    PasteDataEntry entry;
    std::vector<uint8_t> rawData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->ProcessDistributedDelayUri(userId, data, entry, rawData);
}

/**
 * @tc.name: ProcessDistributedDelayHtmlTest001
 * @tc.desc: test Func ProcessDistributedDelayHtml
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ProcessDistributedDelayHtmlTest001, TestSize.Level0)
{
    PasteData data;
    PasteDataEntry entry;
    std::vector<uint8_t> rawData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->ProcessDistributedDelayHtml(data, entry, rawData);
}

/**
 * @tc.name: ProcessDistributedDelayEntryTest001
 * @tc.desc: test Func ProcessDistributedDelayEntry
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ProcessDistributedDelayEntryTest001, TestSize.Level0)
{
    PasteDataEntry pasteData;
    std::vector<uint8_t> rawData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->ProcessDistributedDelayEntry(pasteData, rawData);
}

/**
 * @tc.name: ProcessRemoteDelayHtmlTest001
 * @tc.desc: test Func ProcessRemoteDelayHtml
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ProcessRemoteDelayHtmlTest001, TestSize.Level0)
{
    std::string remoteDeviceId;
    const std::string bundleName;
    const std::vector<uint8_t> rawData;
    PasteData data;
    PasteDataRecord record;
    PasteDataEntry entry;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->ProcessRemoteDelayHtml(remoteDeviceId, bundleName, rawData, data, record, entry);
}

/**
 * @tc.name: GetFullDelayPasteDataTest001
 * @tc.desc: test Func GetFullDelayPasteData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetFullDelayPasteDataTest001, TestSize.Level0)
{
    int32_t userId = ACCOUNT_IDS_RANDOM;
    PasteData pasteData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GetFullDelayPasteData(userId, pasteData);
}

/**
 * @tc.name: GenerateDistributedUriTest001
 * @tc.desc: test Func GenerateDistributedUri
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GenerateDistributedUriTest001, TestSize.Level0)
{
    PasteData pasteData;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GenerateDistributedUri(pasteData);
}

/**
 * @tc.name: CloseDistributedStoreTest001
 * @tc.desc: test Func CloseDistributedStore
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CloseDistributedStoreTest001, TestSize.Level0)
{
    int32_t user = ACCOUNT_IDS_RANDOM;
    bool isNeedClear = false;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->CloseDistributedStore(user, isNeedClear);
}

/**
 * @tc.name: OnConfigChangeTest002
 * @tc.desc: test Func OnConfigChange
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnConfigChangeTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->OnConfigChange(true);
}

/**
 * @tc.name: GetAppLabelTest001
 * @tc.desc: test Func GetAppLabel
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetAppLabelTest001, TestSize.Level0)
{
    uint32_t tokenId = UINT32_ONE;
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->GetAppLabel(tokenId);
}

/**
 * @tc.name: GetAppBundleManagerTest001
 * @tc.desc: GetAppBundleManager function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetAppBundleManagerTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard->GetAppBundleManager(), nullptr);
}

/**
 * @tc.name: ChangeStoreStatusTest001
 * @tc.desc: ChangeStoreStatus function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ChangeStoreStatusTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    int32_t userId = 1;
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->ChangeStoreStatus(userId);
}

/**
 * @tc.name: OnReceiveEventTest001
 * @tc.desc: OnReceiveEvent function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnReceiveEventTest001, TestSize.Level0)
{
    EventFwk::CommonEventSubscribeInfo subscribeInfo;
    sptr<PasteboardService> service;
    auto tempPasteboard = std::make_shared<PasteBoardCommonEventSubscriber>(subscribeInfo, service);
    EXPECT_NE(tempPasteboard, nullptr);
    const EventFwk::CommonEventData data;
    tempPasteboard->OnReceiveEvent(data);
}

/**
 * @tc.name: OnStateChangedTest001
 * @tc.desc: OnStateChanged function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnStateChangedTest001, TestSize.Level0)
{
    AccountSA::OsAccountSubscribeInfo subscribeInfo;
    sptr<PasteboardService> service;
    auto tempPasteboard = std::make_shared<PasteBoardAccountStateSubscriber>(subscribeInfo, service);
    EXPECT_NE(tempPasteboard, nullptr);
    const AccountSA::OsAccountStateData data;
    tempPasteboard->OnStateChanged(data);
}

/**
 * @tc.name: SubscribeKeyboardEventTest001
 * @tc.desc: SubscribeKeyboardEvent function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SubscribeKeyboardEventTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    auto result = tempPasteboard->SubscribeKeyboardEvent();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: PasteboardEventSubscriberTest001
 * @tc.desc: PasteboardEventSubscriber function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, PasteboardEventSubscriberTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->PasteboardEventSubscriber();
}

/**
 * @tc.name: CommonEventSubscriberTest001
 * @tc.desc: CommonEventSubscriber function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CommonEventSubscriberTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->CommonEventSubscriber();
}

/**
 * @tc.name: RemoveObserverByPidTest001
 * @tc.desc: RemoveObserverByPid function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemoveObserverByPidTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    int32_t userId = 1;
    pid_t pid = 1;
    PasteboardService::ObserverMap observerMap;
    tempPasteboard->RemoveObserverByPid(userId, pid, observerMap);
}

/**
 * @tc.name: RegisterClientDeathObserverTest001
 * @tc.desc: RegisterClientDeathObserver function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RegisterClientDeathObserverTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    PasteboardService service;
    pid_t pid = 1;
    sptr<IRemoteObject> observer = sptr<RemoteObjectTest>::MakeSptr(u"test");
    auto result = tempPasteboard->RegisterClientDeathObserver(observer);
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: RemotePasteboardChangeTest001
 * @tc.desc: RemotePasteboardChange function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemotePasteboardChangeTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    auto result = tempPasteboard->RemotePasteboardChange();
    EXPECT_NE(result, nullptr);
}

/**
 * @tc.name: IsCtrlVProcessTest001
 * @tc.desc: IsCtrlVProcess function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsCtrlVProcessTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<InputEventCallback>();
    EXPECT_NE(tempPasteboard, nullptr);
    uint32_t callingPid = 1;
    bool isFocused = 1;
    auto result = tempPasteboard->IsCtrlVProcess(callingPid, isFocused);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: ClearTest002
 * @tc.desc: Clear function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ClearTest002, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<InputEventCallback>();
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->Clear();
}

/**
 * @tc.name: OnRemoteDiedTest003
 * @tc.desc: OnRemoteDied function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, OnRemoteDiedTest003, TestSize.Level0)
{
    PasteboardService service;
    pid_t pid = 2;
    auto tempPasteboard = std::make_shared<PasteboardService::PasteboardDeathRecipient>(service, nullptr, pid);
    EXPECT_NE(tempPasteboard, nullptr);
    tempPasteboard->OnRemoteDied(nullptr);
}

/**
 * @tc.name: AppExitTest001
 * @tc.desc: AppExit function.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, AppExitTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);
    pid_t pid = 3;
    tempPasteboard->AppExit(pid);
}

/**
 * @tc.name: CleanDistributedDataTest001
 * @tc.desc: test Func CleanDistributedData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, CleanDistributedDataTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    int32_t user = ACCOUNT_IDS_RANDOM;
    tempPasteboard->CleanDistributedData(user);
}

/**
 * @tc.name: GetRemoteEntryValueTest001
 * @tc.desc: test Func GetRemoteEntryValue
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetRemoteEntryValueTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    AppInfo appInfo;
    PasteData pasteData;
    PasteDataRecord record;
    PasteDataEntry entry;
    tempPasteboard->GetRemoteEntryValue(appInfo, pasteData, record, entry);
}

/**
 * @tc.name: GetLocalEntryValueTest001
 * @tc.desc: test Func GetLocalEntryValue
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetLocalEntryValueTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    int32_t userId = 1;
    PasteData pasteData;
    PasteDataRecord record;
    PasteDataEntry entry;
    tempPasteboard->GetLocalEntryValue(userId, pasteData, record, entry);
}

/**
 * @tc.name: GetDistributedDelayDataTest001
 * @tc.desc: test Func GetDistributedDelayData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetDistributedDelayDataTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    TestEvent event;
    std::vector<uint8_t> rawData;
    tempPasteboard->GetDistributedDelayData(event, 0, rawData);
}

/**
 * @tc.name: GenerateDataTypeTest001
 * @tc.desc: test Func GenerateDataType
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GenerateDataTypeTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    PasteData pasteData;
    tempPasteboard->GenerateDataType(pasteData);
}

/**
 * @tc.name: IsFocusedAppTest001
 * @tc.desc: test Func IsFocusedApp
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsFocusedAppTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    uint32_t tokenId = UINT32_ONE;
    tempPasteboard->IsFocusedApp(tokenId);
}

/**
 * @tc.name: SetAppShareOptionsTest001
 * @tc.desc: test Func SetAppShareOptions
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetAppShareOptionsTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    int32_t shareOptions = 0;
    tempPasteboard->SetAppShareOptions(shareOptions);
}

/**
 * @tc.name: IsSystemAppByFullTokenIDTest001
 * @tc.desc: test Func IsSystemAppByFullTokenID
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsSystemAppByFullTokenIDTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    uint32_t tokenId = UINT32_ONE;
    tempPasteboard->IsSystemAppByFullTokenID(tokenId);
}

/**
 * @tc.name: RemoveAllObserverTest001
 * @tc.desc: test Func RemoveAllObserver
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RemoveAllObserverTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    int32_t userId = 1;
    PasteboardService::ObserverMap observerMap;
    tempPasteboard->RemoveAllObserver(userId, observerMap);
}

/**
 * @tc.name: GetObserversSizeTest001
 * @tc.desc: test Func GetObserversSize
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetObserversSizeTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    int32_t userId = 1;
    pid_t pid = 1;
    PasteboardService::ObserverMap observerMap;
    tempPasteboard->GetObserversSize(userId, pid, observerMap);
}

/**
 * @tc.name: ClearInputMethodPidTest001
 * @tc.desc: test Func ClearInputMethodPid
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ClearInputMethodPidTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    tempPasteboard->ClearInputMethodPid();
}

/**
 * @tc.name: ClearInputMethodPidByPidTest001
 * @tc.desc: test Func ClearInputMethodPidByPid
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ClearInputMethodPidByPidTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    pid_t callPid = 1;
    tempPasteboard->ClearInputMethodPidByPid(callPid);
}

/**
 * @tc.name: SetInputMethodPidTest001
 * @tc.desc: test Func SetInputMethodPid
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, SetInputMethodPidTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    pid_t callPid = 1;
    tempPasteboard->SetInputMethodPid(callPid);
}

/**
 * @tc.name: GetCurrentScreenStatusTest001
 * @tc.desc: test Func GetCurrentScreenStatus
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetCurrentScreenStatusTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    tempPasteboard->GetCurrentScreenStatus();
}

/**
 * @tc.name: GetLocalMimeTypesTest001
 * @tc.desc: test Func GetLocalMimeTypes
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetLocalMimeTypesTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    tempPasteboard->GetLocalMimeTypes();
}

/**
 * @tc.name: ShowHintToastTest001
 * @tc.desc: test Func ShowHintToast
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, ShowHintToastTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    uint32_t tokenId = UINT32_ONE;
    uint32_t pid = 0;
    tempPasteboard->ShowHintToast(tokenId, pid);
}

/**
 * @tc.name: RevokeUriPermissionTest001
 * @tc.desc: test Func RevokeUriPermission
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, RevokeUriPermissionTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    tempPasteboard->RevokeUriPermission(pasteData);
}

/**
 * @tc.name: GetLocalDataTest001
 * @tc.desc: test Func GetLocalData
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetLocalDataTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    AppInfo appInfo;
    PasteData pasteData;
    tempPasteboard->GetLocalData(appInfo, pasteData);
}

/**
 * @tc.name: AddPermissionRecordTest001
 * @tc.desc: test Func AddPermissionRecord
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, AddPermissionRecordTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    uint32_t tokenId = 0x123456;
    tempPasteboard->AddPermissionRecord(tokenId, true, true);
}

/**
 * @tc.name: GetAppBundleNameTest001
 * @tc.desc: test Func GetAppBundleName
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetAppBundleNameTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    AppInfo appInfo;
    tempPasteboard->GetAppBundleName(appInfo);
}

/**
 * @tc.name: GetAppInfoTest001
 * @tc.desc: test Func GetAppInfo
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetAppInfoTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    uint32_t tokenId = 0x123456;
    tempPasteboard->GetAppInfo(tokenId);
}

/**
 * @tc.name: IsDataAgedTest001
 * @tc.desc: test Func IsDataAged
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsDataAgedTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    tempPasteboard->IsDataAged();
}

/**
 * @tc.name: GetSdkVersionTest001
 * @tc.desc: test Func GetSdkVersion
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, GetSdkVersionTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    uint32_t tokenId = 0x123456;
    tempPasteboard->GetSdkVersion(tokenId);
}

/**
 * @tc.name: IsDataVaildTest001
 * @tc.desc: test Func IsDataVaild
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, IsDataVaildTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    PasteData pasteData;
    uint32_t tokenId = 0x123456;
    tempPasteboard->IsDataVaild(pasteData, tokenId);
}

/**
 * @tc.name: VerifyPermissionTest001
 * @tc.desc: test Func VerifyPermission
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, VerifyPermissionTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    uint32_t tokenId = 0x123456;
    tempPasteboard->VerifyPermission(tokenId);
}

/**
 * @tc.name: UnsubscribeAllEntityObserverTest001
 * @tc.desc: test Func UnsubscribeAllEntityObserver
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardServiceTest, UnsubscribeAllEntityObserverTest001, TestSize.Level0)
{
    auto tempPasteboard = std::make_shared<PasteboardService>();
    EXPECT_NE(tempPasteboard, nullptr);

    tempPasteboard->UnsubscribeAllEntityObserver();
}
}
} // namespace OHOS::MiscServices