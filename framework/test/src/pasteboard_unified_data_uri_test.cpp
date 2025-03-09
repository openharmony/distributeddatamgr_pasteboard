/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "audio.h"
#include "folder.h"
#include "image.h"
#include "pasteboard_client.h"
#include "pasteboard_hilog.h"
#include "video.h"
namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::Media;
class PasteboardUnifiedDataUriTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    UDMF::UnifiedData InitFileData001();
    UDMF::UnifiedData InitImageData001();
    UDMF::UnifiedData InitVideoData001();
    UDMF::UnifiedData InitAudioData001();
    UDMF::UnifiedData InitFolderData001();
    UDMF::UnifiedData InitFileData002();
    UDMF::UnifiedData InitImageData002();
    UDMF::UnifiedData InitVideoData002();
    UDMF::UnifiedData InitAudioData002();
    UDMF::UnifiedData InitFolderData002();
    UDMF::UnifiedData InitFileData003();
    UDMF::UnifiedData InitImageData003();
    UDMF::UnifiedData InitVideoData003();
    UDMF::UnifiedData InitAudioData003();
    UDMF::UnifiedData InitFolderData003();

protected:
    Details details_;
    std::string uri_;
};

void PasteboardUnifiedDataUriTest::SetUpTestCase(void) { }

void PasteboardUnifiedDataUriTest::TearDownTestCase(void) { }

void PasteboardUnifiedDataUriTest::SetUp(void) { }

void PasteboardUnifiedDataUriTest::TearDown(void) { }

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitFileData001()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE);
    uri_ = "file://uri";
    std::shared_ptr<UDMF::File> fileRecord = std::make_shared<UDMF::File>(uri_);
    fileRecord->SetDetails(details_);
    data.AddRecord(fileRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitFileData002()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE);
    uri_ = "file:///uri";
    std::shared_ptr<UDMF::File> fileRecord = std::make_shared<UDMF::File>(uri_);
    fileRecord->SetDetails(details_);
    data.AddRecord(fileRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitFileData003()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE);
    uri_ = "file:////uri";
    std::shared_ptr<UDMF::File> fileRecord = std::make_shared<UDMF::File>(uri_);
    fileRecord->SetDetails(details_);
    data.AddRecord(fileRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitImageData001()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::IMAGE);
    uri_ = "file://image";
    std::shared_ptr<UDMF::Image> imageRecord = std::make_shared<UDMF::Image>(uri_);
    imageRecord->SetDetails(details_);
    data.AddRecord(imageRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitImageData002()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::IMAGE);
    uri_ = "file:///image";
    std::shared_ptr<UDMF::Image> imageRecord = std::make_shared<UDMF::Image>(uri_);
    imageRecord->SetDetails(details_);
    data.AddRecord(imageRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitImageData003()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::IMAGE);
    uri_ = "file:////image";
    std::shared_ptr<UDMF::Image> imageRecord = std::make_shared<UDMF::Image>(uri_);
    imageRecord->SetDetails(details_);
    data.AddRecord(imageRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitVideoData001()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::VIDEO);
    uri_ = "file://Video";
    std::shared_ptr<UDMF::Video> videoRecord = std::make_shared<UDMF::Video>(uri_);
    videoRecord->SetDetails(details_);
    data.AddRecord(videoRecord);

    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitVideoData002()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::VIDEO);
    uri_ = "file:///Video";
    std::shared_ptr<UDMF::Video> videoRecord = std::make_shared<UDMF::Video>(uri_);
    videoRecord->SetDetails(details_);
    data.AddRecord(videoRecord);

    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitVideoData003()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::VIDEO);
    uri_ = "file:////Video";
    std::shared_ptr<UDMF::Video> videoRecord = std::make_shared<UDMF::Video>(uri_);
    videoRecord->SetDetails(details_);
    data.AddRecord(videoRecord);

    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitAudioData001()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::AUDIO);
    uri_ = "file://Audio";
    std::shared_ptr<UDMF::Audio> audioRecord = std::make_shared<UDMF::Audio>(uri_);
    audioRecord->SetDetails(details_);
    data.AddRecord(audioRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitAudioData002()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::AUDIO);
    uri_ = "file:///Audio";
    std::shared_ptr<UDMF::Audio> audioRecord = std::make_shared<UDMF::Audio>(uri_);
    audioRecord->SetDetails(details_);
    data.AddRecord(audioRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitAudioData003()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::AUDIO);
    uri_ = "file:////Audio";
    std::shared_ptr<UDMF::Audio> audioRecord = std::make_shared<UDMF::Audio>(uri_);
    audioRecord->SetDetails(details_);
    data.AddRecord(audioRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitFolderData001()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FOLDER);
    uri_ = "file://Folder";
    std::shared_ptr<UDMF::Folder> folderRecord = std::make_shared<UDMF::Folder>(uri_);
    folderRecord->SetDetails(details_);
    data.AddRecord(folderRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitFolderData002()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FOLDER);
    uri_ = "file:///Folder";
    std::shared_ptr<UDMF::Folder> folderRecord = std::make_shared<UDMF::Folder>(uri_);
    folderRecord->SetDetails(details_);
    data.AddRecord(folderRecord);
    return data;
}

UDMF::UnifiedData PasteboardUnifiedDataUriTest::InitFolderData003()
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FOLDER);
    uri_ = "file:////Folder";
    std::shared_ptr<UDMF::Folder> folderRecord = std::make_shared<UDMF::Folder>(uri_);
    folderRecord->SetDetails(details_);
    data.AddRecord(folderRecord);
    return data;
}

/**
 * @tc.name: SetFile001
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetFile001, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFile001 start");
    auto data = InitFileData001();
    PasteboardClient::GetInstance()->SetUnifiedData(data);

    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::FILE);
    auto newFileRecord = static_cast<UDMF::File *>(newRecord.get());
    auto newDetails = newFileRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newFileRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::FILE);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFile001 end");
}

/**
 * @tc.name: SetFile002
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetFile002, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFile002 start");
    auto data = InitFileData002();
    PasteboardClient::GetInstance()->SetUnifiedData(data);

    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::FILE);
    auto newFileRecord = static_cast<UDMF::File *>(newRecord.get());
    auto newDetails = newFileRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newFileRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::FILE);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFile002 end");
}

/**
 * @tc.name: SetFile003
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetFile003, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFile003 start");
    auto data = InitFileData003();
    PasteboardClient::GetInstance()->SetUnifiedData(data);

    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::FILE);
    auto newFileRecord = static_cast<UDMF::File *>(newRecord.get());
    auto newDetails = newFileRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newFileRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::FILE);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFile003 end");
}

/**
 * @tc.name: SetImage001
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetImage001, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetImage001 start");
    auto data = InitImageData001();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::IMAGE);
    auto newImageRecord = static_cast<UDMF::Image *>(newRecord.get());
    auto newDetails = newImageRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newImageRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::IMAGE);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetImage001 end");
}

/**
 * @tc.name: SetImage002
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetImage002, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetImage002 start");
    auto data = InitImageData002();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::IMAGE);
    auto newImageRecord = static_cast<UDMF::Image *>(newRecord.get());
    auto newDetails = newImageRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newImageRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::IMAGE);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetImage002 end");
}

/**
 * @tc.name: SetImage003
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetImage003, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetImage003 start");
    auto data = InitImageData003();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::IMAGE);
    auto newImageRecord = static_cast<UDMF::Image *>(newRecord.get());
    auto newDetails = newImageRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newImageRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::IMAGE);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetImage003 end");
}

/**
 * @tc.name: SetAudio001
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetAudio001, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetAudio001 start");
    auto data = InitAudioData001();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::AUDIO);
    auto newAudioRecord = static_cast<UDMF::Audio *>(newRecord.get());
    auto newDetails = newAudioRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newAudioRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::AUDIO);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetAudio001 end");
}

/**
 * @tc.name: SetAudio002
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetAudio002, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetAudio002 start");
    auto data = InitAudioData002();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::AUDIO);
    auto newAudioRecord = static_cast<UDMF::Audio *>(newRecord.get());
    auto newDetails = newAudioRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newAudioRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::AUDIO);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetAudio002 end");
}

/**
 * @tc.name: SetAudio003
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetAudio003, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetAudio003 start");
    auto data = InitAudioData003();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::AUDIO);
    auto newAudioRecord = static_cast<UDMF::Audio *>(newRecord.get());
    auto newDetails = newAudioRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newAudioRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::AUDIO);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetAudio003 end");
}

/**
 * @tc.name: SetVideo001
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetVideo001, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetVideo001 start");
    auto data = InitVideoData001();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::VIDEO);
    auto newVideoRecord = static_cast<UDMF::Video *>(newRecord.get());
    auto newDetails = newVideoRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newVideoRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::VIDEO);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetVideo001 end");
}

/**
 * @tc.name: SetVideo002
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetVideo002, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetVideo002 start");
    auto data = InitVideoData002();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::VIDEO);
    auto newVideoRecord = static_cast<UDMF::Video *>(newRecord.get());
    auto newDetails = newVideoRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newVideoRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::VIDEO);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetVideo002 end");
}

/**
 * @tc.name: SetVideo003
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetVideo003, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetVideo003 start");
    auto data = InitVideoData003();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::VIDEO);
    auto newVideoRecord = static_cast<UDMF::Video *>(newRecord.get());
    auto newDetails = newVideoRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newVideoRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::VIDEO);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetVideo003 end");
}

/**
 * @tc.name: SetFolder001
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetFolder001, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFolder001 start");
    auto data = InitFolderData001();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::FOLDER);
    auto newFolderRecord = static_cast<UDMF::Folder *>(newRecord.get());
    auto newDetails = newFolderRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newFolderRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::FOLDER);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFolder001 end");
}

/**
 * @tc.name: SetFolder002
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetFolder002, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFolder002 start");
    auto data = InitFolderData002();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::FOLDER);
    auto newFolderRecord = static_cast<UDMF::Folder *>(newRecord.get());
    auto newDetails = newFolderRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newFolderRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::FOLDER);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFolder002 end");
}

/**
 * @tc.name: SetFolder003
 * @tc.desc: Get the source of the data.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(PasteboardUnifiedDataUriTest, SetFolder003, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFolder003 start");
    auto data = InitFolderData003();
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::FOLDER);
    auto newFolderRecord = static_cast<UDMF::Folder *>(newRecord.get());
    auto newDetails = newFolderRecord->GetDetails();
    ASSERT_EQ(newDetails, details_);
    auto uri2 = newFolderRecord->GetUri();
    ASSERT_EQ(uri2, uri_);

    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::FOLDER);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri_);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details_);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "PasteboardUnifiedDataUriTest SetFolder003 end");
}

} // namespace OHOS::MiscServices