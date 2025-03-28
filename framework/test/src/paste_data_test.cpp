/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <gtest/gtest.h>

#include "clip/clip_plugin.h"
#include "clip_factory.h"
#include "common/block_object.h"
#include "int_wrapper.h"
#include "pasteboard_client.h"
#include "pasteboard_service_loader.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::AAFwk;
using namespace OHOS::Media;
constexpr const int32_t INVALID_FD = -1;
constexpr const char *FILE_URI = "/data/test/resource/pasteboardTest.txt";
class PasteDataTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteDataTest::SetUpTestCase(void) { }

void PasteDataTest::TearDownTestCase(void) { }

void PasteDataTest::SetUp(void) { }

void PasteDataTest::TearDown(void) { }

ClipFactory::ClipFactory()
{
    ClipPlugin::RegCreator("distributed_clip", this);
}

/**
 * @tc.name: AddRecord001
 * @tc.desc: PasteDataRecord AddRecord
 * @tc.type: FUNC
 */
HWTEST_F(PasteDataTest, AddRecord001, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = FILE_URI;
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();
    EXPECT_TRUE(record != nullptr);
    data.AddRecord(nullptr);
    auto count = data.GetRecordCount();
    EXPECT_TRUE(count == 0);
}

/**
 * @tc.name: AddRecord002
 * @tc.desc: PasteDataRecord AddRecord
 * @tc.type: FUNC
 */
HWTEST_F(PasteDataTest, AddRecord002, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = FILE_URI;
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();
    data.AddRecord(*record);
    auto count = data.GetRecordCount();
    EXPECT_TRUE(count == 1);
}

/**
 * @tc.name: Marshalling001
 * @tc.desc: PasteData Marshalling
 * @tc.type: FUNC
 */
HWTEST_F(PasteDataTest, Marshalling001, TestSize.Level0)
{
    PasteData data1;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = FILE_URI;
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    builder.SetUri(uri);
    auto record = builder.Build();
    data1.AddRecord(*record);
    auto count = data1.GetRecordCount();
    EXPECT_TRUE(count == 1);
    Parcel parcel;
    data1.Marshalling(parcel);

    auto data2 = PasteData::Unmarshalling(parcel);
    auto count2 = data2->GetRecordCount();
    EXPECT_TRUE(count == count2);
    std::shared_ptr<OHOS::Uri> uri2 = data2->GetPrimaryUri();
    std::string uriStr2 = uri2->ToString();
    EXPECT_TRUE(uriStr == uriStr2);
}

/**
 * @tc.name: MaxLength001
 * @tc.desc: PasteDataRecord: maxLength NewHtmlRecord
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, MaxLength001, TestSize.Level0)
{
    int maxLength = 100 * 1024 * 1024 + 1;
    std::string res = "hello";
    std::string temp = "world";
    for (int i = 0; i < maxLength; i++) {
        res += temp;
    }
    std::string htmlText = "<div class='disabled'>" + res + "</div>";
    auto record = PasteboardClient::GetInstance()->CreateHtmlTextRecord(htmlText);
    ASSERT_TRUE(record == nullptr);
}

/**
 * @tc.name: MaxLength002
 * @tc.desc: PasteDataRecord: maxLength NewPlainTextRecord
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, MaxLength002, TestSize.Level0)
{
    int maxLength = 100 * 1024 * 1024 + 1;
    std::string plainText = "hello";
    std::string temp = "world";
    for (int i = 0; i < maxLength; i++) {
        plainText += temp;
    }
    auto record = PasteboardClient::GetInstance()->CreatePlainTextRecord(plainText);
    ASSERT_TRUE(record == nullptr);
}

/**
 * @tc.name: ConvertToText001
 * @tc.desc: PasteDataRecord: ConvertToText htmlText
 * @tc.type: FUNC
 * @tc.require: AR000HEECD
 * @tc.author: chenyu
 */
HWTEST_F(PasteDataTest, ConvertToText001, TestSize.Level0)
{
    std::string htmlText = "<div class='disabled item tip user-programs'>";
    auto record = PasteboardClient::GetInstance()->CreateHtmlTextRecord(htmlText);
    ASSERT_TRUE(record != nullptr);
    auto text = record->ConvertToText();
    EXPECT_EQ(text, htmlText);
}

/**
 * @tc.name: ConvertToText002
 * @tc.desc: PasteDataRecord: ConvertToText plainText
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ConvertToText002, TestSize.Level0)
{
    std::string plainText = "paste record test";
    auto record = PasteboardClient::GetInstance()->CreatePlainTextRecord(plainText);
    ASSERT_TRUE(record != nullptr);
    auto text = record->ConvertToText();
    EXPECT_EQ(text, plainText);
}

/**
 * @tc.name: ConvertToText003
 * @tc.desc: PasteDataRecord: ConvertToText uri
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ConvertToText003, TestSize.Level0)
{
    OHOS::Uri uri("uri");
    auto record = PasteboardClient::GetInstance()->CreateUriRecord(uri);
    ASSERT_TRUE(record != nullptr);
    auto text = record->ConvertToText();
    EXPECT_EQ(text, uri.ToString());
}

/**
 * @tc.name: ConvertToText004
 * @tc.desc: PasteDataRecord: ConvertToText uri
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ConvertToText004, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = {
        {5, 7},
        PixelFormat::ARGB_8888
    };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto record = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMapIn);
    ASSERT_TRUE(record != nullptr);
    auto text = record->ConvertToText();
    EXPECT_EQ(text, "");
}

/**
 * @tc.name: ConvertToText005
 * @tc.desc: PasteDataRecord: ConvertToText plain
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ConvertToText005, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = {
        {5, 7},
        PixelFormat::ARGB_8888
    };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto record = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMapIn);
    ASSERT_TRUE(record != nullptr);
    std::string plainText = "hello";
    auto plainUtdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, MIMETYPE_TEXT_PLAIN);
    record->AddEntryByMimeType(MIMETYPE_TEXT_PLAIN, std::make_shared<PasteDataEntry>(plainUtdId, plainText));
    auto text = record->ConvertToText();
    EXPECT_EQ(text, plainText);
}

/**
 * @tc.name: ConvertToText006
 * @tc.desc: PasteDataRecord: ConvertToText html
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ConvertToText006, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = {
        {5, 7},
        PixelFormat::ARGB_8888
    };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto record = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMapIn);
    ASSERT_TRUE(record != nullptr);
    std::string htmlText = "<div class='disabled item tip'>";
    auto htmlUtdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, MIMETYPE_TEXT_HTML);
    record->AddEntryByMimeType(MIMETYPE_TEXT_HTML, std::make_shared<PasteDataEntry>(htmlUtdId, htmlText));
    auto text = record->ConvertToText();
    EXPECT_EQ(text, htmlText);
}

/**
 * @tc.name: ConvertToText007
 * @tc.desc: PasteDataRecord: ConvertToText uri
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ConvertToText007, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = {
        {5, 7},
        PixelFormat::ARGB_8888
    };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, 100, opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto record = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMapIn);
    ASSERT_TRUE(record != nullptr);
    std::string uri = "file://123.txt";
    auto uriUtdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, MIMETYPE_TEXT_URI);
    record->AddEntryByMimeType(MIMETYPE_TEXT_URI, std::make_shared<PasteDataEntry>(uriUtdId, uri));
    auto text = record->ConvertToText();
    EXPECT_EQ(text, uri);
}

/**
 * @tc.name: GetPasteDataMsg001
 * @tc.desc: PasteData: GetPrimaryMimeType is nullptr and so on
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetPasteDataMsg001, TestSize.Level0)
{
    std::string plainText1 = "helloWorld";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText1);
    ASSERT_TRUE(pasteData != nullptr);
    auto newPrimaryPixelMap = pasteData->GetPrimaryPixelMap();
    ASSERT_TRUE(newPrimaryPixelMap == nullptr);
    auto newPrimaryMimeType = pasteData->GetPrimaryMimeType();
    ASSERT_TRUE(newPrimaryMimeType != nullptr);
    auto newPasteData = std::make_shared<PasteData>();
    auto newPrimaryMimeType2 = newPasteData->GetPrimaryMimeType();
    ASSERT_TRUE(newPrimaryMimeType2 == nullptr);
    std::string plainText2 = "plain text";
    auto record = PasteboardClient::GetInstance()->CreatePlainTextRecord(plainText2);
    ASSERT_TRUE(record != nullptr);
    ASSERT_FALSE(pasteData->ReplaceRecordAt(1000, record));
}

/**
 * @tc.name: GetPasteDataMsg002
 * @tc.desc: PasteData: GetPrimaryWant is nullptr and so on
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetPasteDataMsg002, TestSize.Level0)
{
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = {
        {5, 7},
        PixelFormat::ARGB_8888
    };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    auto newPasteData = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMapIn);
    ASSERT_TRUE(newPasteData != nullptr);
    auto pixMap = newPasteData->GetPrimaryPixelMap();
    ASSERT_TRUE(pixMap != nullptr);
    auto primaryWant = newPasteData->GetPrimaryWant();
    ASSERT_TRUE(primaryWant == nullptr);
    auto primaryText = newPasteData->GetPrimaryText();
    ASSERT_TRUE(primaryText == nullptr);
    auto primaryUri = newPasteData->GetPrimaryUri();
    ASSERT_TRUE(primaryUri == nullptr);
    auto record = newPasteData->GetRecordAt(1);
    ASSERT_TRUE(record == nullptr);
    auto res1 = newPasteData->RemoveRecordAt(1);
    ASSERT_FALSE(res1);
    std::string mimeType = "text/plain";
    ASSERT_FALSE(newPasteData->HasMimeType(mimeType));
}

/**
 * @tc.name: ShareOptionToString001
 * @tc.desc: PasteData: ShareOptionToString
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ShareOptionToString001, TestSize.Level0)
{
    std::string shareOption1;
    PasteData::ShareOptionToString(ShareOption::InApp, shareOption1);
    ASSERT_TRUE(shareOption1 == "InAPP");
    std::string shareOption2;
    PasteData::ShareOptionToString(ShareOption::LocalDevice, shareOption2);
    ASSERT_TRUE(shareOption2 == "LocalDevice");
    std::string shareOption3;
    PasteData::ShareOptionToString(ShareOption::CrossDevice, shareOption3);
    ASSERT_TRUE(shareOption3 == "CrossDevice");
}

/**
 * @tc.name: SetInvalid001
 * @tc.desc: PasteData: SetInvalid001
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetInvalid001, TestSize.Level0)
{
    bool result = true;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->SetInvalid();
    result = pasteData->IsValid();
    ASSERT_FALSE(result);
}

/**
 * @tc.name: SetLocalOnly001
 * @tc.desc: PasteData: SetLocalOnly
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetLocalOnly001, TestSize.Level0)
{
    bool result = false;
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->SetLocalOnly(true);
    result = pasteData->GetLocalOnly();
    ASSERT_TRUE(result);
}

/**
 * @tc.name: SetAddition001
 * @tc.desc: PasteData: SetAddition
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetAddition001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    int64_t fileSize = 0L;
    pasteData->SetFileSize(fileSize);
    AAFwk::WantParams additions;
    pasteData->SetAdditions(additions);
    ASSERT_TRUE(pasteData != nullptr);
}

/**
 * @tc.name: SetRemote001
 * @tc.desc: PasteData: SetRemote
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetRemote001, TestSize.Level0)
{
    bool isRemote = false;
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    isRemote = true;
    pasteData->SetRemote(isRemote);
    bool result = pasteData->IsRemote();
    ASSERT_TRUE(result);
}

/**
 * @tc.name: SetOrginAuthority001
 * @tc.desc: PasteData: SetOrginAuthority
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetOrginAuthority001, TestSize.Level0)
{
    std::string plainText = "plain text";
    std::string bundleName = "com.example.myapplication";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    pasteData->SetBundleName(bundleName);
    pasteData->SetOriginAuthority(bundleName);
    std::string getBundleName = pasteData->GetBundleName();
    std::string getOriginAuthority = pasteData->GetOriginAuthority();
    ASSERT_TRUE(getBundleName == bundleName);
    ASSERT_TRUE(getOriginAuthority == bundleName);
    std::string time = "2023-08-09";
    pasteData->SetTime(time);
    std::string getTime = pasteData->GetTime();
    ASSERT_TRUE(getTime == time);
}

/**
 * @tc.name: GetConvertUri001
 * @tc.desc: PasteDataRecord: GetConvertUri
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetConvertUri001, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    std::string convertUri_ = "/mnt/hmdfs/";
    pasteDataRecord->SetConvertUri(convertUri_);
    std::string result = pasteDataRecord->GetConvertUri();
    ASSERT_TRUE(result == convertUri_);
    std::string newUriStr = "/mnt/hmdfs/test";
    pasteDataRecord->SetUri(std::make_shared<OHOS::Uri>(newUriStr));
    std::shared_ptr<Uri> uri = pasteDataRecord->GetUri();
    ASSERT_TRUE(uri != nullptr);
    std::shared_ptr<Uri> getOriginUri = pasteDataRecord->GetOriginUri();
    ASSERT_TRUE(getOriginUri != nullptr);
}

/**
 * @tc.name: GetConvertUri002
 * @tc.desc: PasteDataRecord: GetConvertUri
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetConvertUri002, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    std::string convertUri_ = "";
    pasteDataRecord->SetConvertUri(convertUri_);
    std::string result = pasteDataRecord->GetConvertUri();
    ASSERT_TRUE(result == convertUri_);
}

/**
 * @tc.name: HasGrantUriPermission001
 * @tc.desc: PasteDataRecord: HasGrantUriPermission
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, HasGrantUriPermission001, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 1, 2, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteDataRecord != nullptr);
    pasteDataRecord->SetGrantUriPermission(true);
    auto hasGrantUriPermission_ = pasteDataRecord->HasGrantUriPermission();
    ASSERT_TRUE(hasGrantUriPermission_);
}

/**
 * @tc.name: LoadSystemAbilityFail001
 * @tc.desc: PasteDataRecord: LoadSystemAbilityFail
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, LoadSystemAbilityFail001, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    std::string mimeType = "image/jpg";
    arrayBuffer = { 1, 2, 3, 4, 6 };
    PasteboardServiceLoader::GetInstance().LoadSystemAbilityFail();
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteDataRecord != nullptr);
}

/**
 * @tc.name: LoadSystemAbilitySuccess001
 * @tc.desc: PasteDataRecord: LoadSystemAbilitySuccess
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, LoadSystemAbilitySuccess001, TestSize.Level0)
{
    std::vector<uint8_t> arrayBuffer(46);
    std::string mimeType = "image/jpg";
    arrayBuffer = { 1, 2, 3, 4, 6 };
    sptr<IRemoteObject> remoteObject = nullptr;
    PasteboardServiceLoader::GetInstance().LoadSystemAbilitySuccess(remoteObject);
    auto pasteDataRecord = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(pasteDataRecord != nullptr);
}

/**
 * @tc.name: SetInterval001
 * @tc.desc: BlockObject: SetInterval
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetInterval001, TestSize.Level0)
{
    uint32_t POPUP_INTERVAL = 1000;
    auto block = std::make_shared<BlockObject<std::shared_ptr<PasteData>>>(POPUP_INTERVAL);
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    block->SetValue(pasteData);
    block->SetInterval(POPUP_INTERVAL);
    auto value = block->GetValue();
    EXPECT_TRUE(value != nullptr);
}

/**
 * @tc.name: ClipPlugin001
 * @tc.desc: API_EXPORT: ClipPlugin
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ClipPlugin001, TestSize.Level0)
{
    std::string PLUGIN_NAME_VAL = "distributed_clip";
    auto release = [&PLUGIN_NAME_VAL, this](ClipPlugin *plugin) {
        ClipPlugin::DestroyPlugin(PLUGIN_NAME_VAL, plugin);
    };
    auto clipPlugin_ = std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME_VAL), release);
    ClipPlugin::Factory *factory = nullptr;
    auto result = ClipPlugin::RegCreator(PLUGIN_NAME_VAL, factory);
    EXPECT_FALSE(result);
    auto userId = 10000;
    auto events1 = clipPlugin_->GetTopEvents(1, userId);
    EXPECT_TRUE(events1.size() == 0);
    auto events2 = clipPlugin_->GetTopEvents(1);
    EXPECT_TRUE(events2.size() == 0);
    clipPlugin_->Clear();
    clipPlugin_->Clear(userId);
}

/**
 * @tc.name: ClipPlugin002
 * @tc.desc: API_EXPORT: ClipPlugin
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ClipPlugin002, TestSize.Level0)
{
    std::string PLUGIN_NAME_VAL = "distributed_clip";
    auto release = [&PLUGIN_NAME_VAL, this](ClipPlugin *plugin) {
        ClipPlugin::DestroyPlugin(PLUGIN_NAME_VAL, plugin);
    };
    auto clipPlugin_ = std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME_VAL), release);
    ClipPlugin::Factory *factory = new ClipFactory();
    auto result = ClipPlugin::RegCreator(PLUGIN_NAME_VAL, factory);
    EXPECT_FALSE(result);
    auto userId = 3701;
    auto events1 = clipPlugin_->GetTopEvents(1, userId);
    EXPECT_TRUE(events1.size() == 0);
    clipPlugin_->Clear(userId);
}

/**
 * @tc.name: ClipPlugin003
 * @tc.desc: API_EXPORT: ClipPlugin
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ClipPlugin003, TestSize.Level0)
{
    ClipPlugin::GlobalEvent event1;
    event1.seqId = 0;
    event1.deviceId = "test_device_id";
    event1.user = 0;
    ClipPlugin::GlobalEvent event2;
    event2.seqId = 0;
    event2.deviceId = "test_device_id";
    event2.user = 1;
    EXPECT_TRUE(event1 == event2);
}

/**
 * @tc.name: ClipPlugin004
 * @tc.desc: API_EXPORT: ClipPlugin
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ClipPlugin004, TestSize.Level0)
{
    ClipPlugin::GlobalEvent event1;
    event1.seqId = 0;
    event1.deviceId = "test_device_id";
    event1.user = 0;
    ClipPlugin::GlobalEvent event2;
    event2.seqId = 0;
    event2.deviceId = "test_device_id1";
    event2.user = 1;
    EXPECT_FALSE(event1 == event2);
}

/**
 * @tc.name: ClipPlugin005
 * @tc.desc: API_EXPORT: ClipPlugin
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ClipPlugin005, TestSize.Level0)
{
    ClipPlugin::GlobalEvent event1;
    event1.seqId = 0;
    event1.deviceId = "test_device_id";
    event1.user = 0;
    ClipPlugin::GlobalEvent event2;
    event2.seqId = 1;
    event2.deviceId = "test_device_id";
    event2.user = 1;
    EXPECT_FALSE(event1 == event2);
}

/**
 * @tc.name: PasteDataOperator001
 * @tc.desc: PasteData: operator
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, PasteDataOperator001, TestSize.Level0)
{
    PasteData data1;
    PasteDataRecord::Builder builder1(MIMETYPE_TEXT_URI);
    std::string uriStr1 = FILE_URI;
    auto uri1 = std::make_shared<OHOS::Uri>(uriStr1);
    builder1.SetUri(uri1);
    auto record1 = builder1.Build();
    data1.AddRecord(record1);
    std::string bundleName1 = "com.example.myapplication";
    data1.SetOriginAuthority(bundleName1);
    PasteData data2;
    PasteDataRecord::Builder builder2(MIMETYPE_TEXT_URI);
    std::string uriStr2 = FILE_URI;
    auto uri2 = std::make_shared<OHOS::Uri>(uriStr2);
    builder2.SetUri(uri2);
    auto record2 = builder2.Build();
    data2.AddRecord(record2);
    std::string bundleName2 = "com.example.myapplication";
    data2.SetOriginAuthority(bundleName2);
    ASSERT_TRUE(data1.GetBundleName() == data2.GetBundleName());
}

/**
 * @tc.name: GetShareOption001
 * @tc.desc: GetShareOption call
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetShareOption001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    ShareOption option = InApp;
    pasteData->SetShareOption(option);
    auto result = pasteData->GetShareOption();
    ASSERT_TRUE(result == InApp);
}

/**
 * @tc.name: AddKvRecord001
 * @tc.desc: AddKvRecord call
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, AddKvRecord001, TestSize.Level0)
{
    PasteData data;
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType = "image/jpg";
    data.AddKvRecord(mimeType, arrayBuffer);
    ASSERT_TRUE(data.GetRecordCount() > 0);
}

/**
 * @tc.name: GetProperty001
 * @tc.desc: GetProperty call
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetProperty001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    PasteDataProperty property = pasteData->GetProperty();
    ASSERT_TRUE(property.tokenId == 0);
}

/**
 * @tc.name: SetProperty001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetProperty001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    PasteDataProperty property;
    property.tokenId = 1;
    pasteData->SetProperty(property);
    PasteDataProperty pasteDataProperty = pasteData->GetProperty();
    ASSERT_TRUE(pasteDataProperty.tokenId == 1);
}

/**
 * @tc.name: SetShareOption001
 * @tc.desc: SetShareOption call
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetShareOption001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    ShareOption option = LocalDevice;
    pasteData->SetShareOption(option);
    auto result = pasteData->GetShareOption();
    ASSERT_TRUE(result == LocalDevice);
}

/**
 * @tc.name: SetTokenId001
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetTokenId001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    uint32_t tokenId = 1;
    pasteData->SetTokenId(tokenId);
    auto result = pasteData->GetTokenId();
    ASSERT_TRUE(result == 1);
}

/**
 * @tc.name: IsDraggedData001
 * @tc.desc: IsDraggedData call
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, IsDraggedData001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    bool isDraggedData = false;
    pasteData->SetDraggedDataFlag(isDraggedData);
    auto result = pasteData->IsDraggedData();
    ASSERT_FALSE(result);
}

/**
 * @tc.name: SetDraggedDataFlag001
 * @tc.desc: SetDraggedDataFlag call
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetDraggedDataFlag001, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ASSERT_TRUE(pasteData != nullptr);
    bool isDraggedData = true;
    pasteData->SetDraggedDataFlag(isDraggedData);
    auto result = pasteData->IsDraggedData();
    ASSERT_TRUE(result);
}

/**
 * @tc.name: SetScreenStatus
 * @tc.desc:
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, SetScreenStatus, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    ScreenEvent event = ScreenEvent::ScreenLocked;
    pasteData->SetScreenStatus(event);
    ScreenEvent ret = pasteData->GetScreenStatus();
    ASSERT_TRUE(ret == ScreenEvent::ScreenLocked);
}

/**
 * @tc.name: GetMimeTypes
 * @tc.desc: PasteData GetMimeTypes
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetMimeTypes, TestSize.Level0)
{
    PasteData data;
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_URI);
    std::string uriStr = FILE_URI;
    auto uri = std::make_shared<OHOS::Uri>(uriStr);
    auto record = builder.SetUri(uri).Build();
    data.AddRecord(*record);

    PasteDataRecord::Builder builder1(MIMETYPE_TEXT_PLAIN);
    std::string plainText = "plain text";
    auto text = std::make_shared<std::string>(plainText);
    auto record1 = builder1.SetPlainText(text).Build();
    data.AddRecord(*record1);

    auto mimeType = data.GetMimeTypes();
    EXPECT_TRUE((strcmp(MIMETYPE_TEXT_PLAIN, mimeType.at(0).c_str()) == 0 &&
                    strcmp(MIMETYPE_TEXT_URI, mimeType.at(1).c_str()) == 0) ||
        (strcmp(MIMETYPE_TEXT_PLAIN, mimeType.at(1).c_str()) == 0 &&
            strcmp(MIMETYPE_TEXT_URI, mimeType.at(0).c_str()) == 0));
}

/**
 * @tc.name: GetDeviceId
 * @tc.desc: Get DeviceId
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetDeviceId, TestSize.Level0)
{
    PasteData data;
    data.GetDeviceId();
    ASSERT_TRUE(true);
}

/**
 * @tc.name: GetRecordByIdTest001
 * @tc.desc: GetRecordByIdTest001
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetRecordByIdTest001, TestSize.Level0)
{
    //Arrange
    PasteData pasteData;
    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    record->SetRecordId(1);
    pasteData.AddRecord(record);

    //Act
    std::shared_ptr<PasteDataRecord> result = pasteData.GetRecordById(1);

    //Assert
    EXPECT_EQ(result, record);
}

/**
 * @tc.name: GetRecordByIdTest002
 * @tc.desc: GetRecordByIdTest002
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetRecordByIdTest002, TestSize.Level0)
{
    //Arrange
    PasteData pasteData;
    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    record->SetRecordId(1);
    pasteData.AddRecord(record);

    //Act
    std::shared_ptr<PasteDataRecord> result = pasteData.GetRecordById(2);

    //Assert
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetRecordByIdTest003
 * @tc.desc: GetRecordByIdTest003
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, GetRecordByIdTest003, TestSize.Level0)
{
    //Arrange
    PasteData pasteData;

    //Act
    std::shared_ptr<PasteDataRecord> result = pasteData.GetRecordById(1);

    //Assert
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: ReplaceRecordAtTest001
 * @tc.desc: ReplaceRecordAtTest001
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteDataTest, ReplaceRecordAtTest001, TestSize.Level0)
{
    //Arrange
    PasteData pasteData;

    //Act
    std::shared_ptr<PasteDataRecord> record = nullptr;

    //Assert
    EXPECT_FALSE(pasteData.ReplaceRecordAt(0, record));
}
} // namespace OHOS::MiscServices
