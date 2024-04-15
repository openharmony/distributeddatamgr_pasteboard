/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "pasteboard_client.h"

#include <gtest/gtest.h>

#include "application_defined_record.h"
#include "audio.h"
#include "file.h"
#include "folder.h"
#include "html.h"
#include "image.h"
#include "link.h"
#include "plain_text.h"
#include "system_defined_appitem.h"
#include "system_defined_form.h"
#include "system_defined_pixelmap.h"
#include "system_defined_record.h"
#include "video.h"
namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::Media;
class PasteboardUnifiedDataTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardUnifiedDataTest::SetUpTestCase(void) {}

void PasteboardUnifiedDataTest::TearDownTestCase(void) {}

void PasteboardUnifiedDataTest::SetUp(void) {}

void PasteboardUnifiedDataTest::TearDown(void) {}

/**
* @tc.name: SetText001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetText001, TestSize.Level0)
{
    UDMF::UDDetails details;
    details.insert({ "1", "1" });
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UD_TYPE_MAP.at(UDMF::TEXT);
    std::shared_ptr<UDMF::Text> textRecord = std::make_shared<UDMF::Text>();
    textRecord->SetDetails(details);
    data.AddRecord(textRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::TEXT);
    auto newPlainRecord = static_cast<UDMF::Text*>(newRecord.get());
    auto newDetails = newPlainRecord->GetDetails();
    ASSERT_EQ(newDetails, details);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, typeStr);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::TEXT);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details);
}

/**
* @tc.name: SetPlainText001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetPlainText001, TestSize.Level0)
{
    std::string text = "helloWorld_plainText";
    std::string abstract = "helloWorld_plainabstract";
    UDMF::UDDetails details;
    details.insert({ "1", "1" });
    UDMF::UnifiedData data;
    std::shared_ptr<UDMF::PlainText> plainTextRecord = std::make_shared<UDMF::PlainText>(text, abstract);
    plainTextRecord->SetDetails(details);
    data.AddRecord(plainTextRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::PLAIN_TEXT);
    auto newPlainRecord = static_cast<UDMF::PlainText*>(newRecord.get());
    auto newPlainText = newPlainRecord->GetContent();
    auto newAbstract = newPlainRecord->GetAbstract();
    auto newDetails = newPlainRecord->GetDetails();
    ASSERT_EQ(newPlainText, text);
    ASSERT_EQ(newAbstract, abstract);
    ASSERT_EQ(newDetails, details);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_PLAIN);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::PLAIN_TEXT);
    auto plain = record->GetPlainText();
    auto textContent = record->GetTextContent();
    ASSERT_EQ(*plain, text);
    ASSERT_EQ(textContent, abstract);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details);
}

/**
* @tc.name: SetLink001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetLink001, TestSize.Level0)
{
    std::string text = "https://www.huawei.com";
    std::string content = "https://www.huawei.com_content";
    UDMF::UDDetails details;
    details.insert({ "1", "1" });
    UDMF::UnifiedData data;
    std::shared_ptr<UDMF::Link> linkRecord = std::make_shared<UDMF::Link>(text, content);
    linkRecord->SetDetails(details);
    data.AddRecord(linkRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::HYPERLINK);
    auto newPlainRecord = static_cast<UDMF::Link*>(newRecord.get());
    auto newUrl = newPlainRecord->GetUrl();
    auto newDescription = newPlainRecord->GetDescription();
    auto newDetails = newPlainRecord->GetDetails();
    ASSERT_EQ(newUrl, text);
    ASSERT_EQ(newDescription, content);
    ASSERT_EQ(newDetails, details);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_PLAIN);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::HYPERLINK);
    auto plain = record->GetPlainText();
    auto textContent = record->GetTextContent();
    ASSERT_EQ(*plain, text);
    ASSERT_EQ(textContent, content);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details);
}

/**
* @tc.name: SetHtml001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
//HWTEST_F(PasteboardUnifiedDataTest, SetHtml001, TestSize.Level0)
//{
//    std::string text = "<div class='disable'>helloWorld</div>";
//    std::string content = "helloWorld_plainabstract";
//    UDMF::UDDetails details;
//    details.insert({ "1", "1" });
//    UDMF::UnifiedData data;
//    std::shared_ptr<UDMF::Html> htmlRecord = std::make_shared<UDMF::Html>(text, content);
//    htmlRecord->SetDetails(details);
//    data.AddRecord(htmlRecord);
//    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
//    uniProps->tag = "myTag";
//    data.SetProperties(uniProps);
//
//    // SetUnifiedData
//    PasteboardClient::GetInstance()->SetUnifiedData(data);
//    // GetUnifiedData
//    UDMF::UnifiedData newData;
//    PasteboardClient::GetInstance()->GetUnifiedData(newData);
//    ASSERT_EQ(1, newData.GetRecords().size());
//    auto newRecord = newData.GetRecordAt(0);
//    auto newType = newRecord->GetType();
//    ASSERT_EQ(newType, UDMF::HTML);
//    auto newPlainRecord = static_cast<UDMF::Html*>(newRecord.get());
//    auto newPlainText = newPlainRecord->GetHtmlContent();
//    auto newAbstract = newPlainRecord->GetPlainContent();
//    auto newDetails = newPlainRecord->GetDetails();
//    ASSERT_EQ(newPlainText, text);
//    ASSERT_EQ(newAbstract, content);
//    ASSERT_EQ(newDetails, details);
//
//    // GetPasteData
//    PasteData pasteData;
//    PasteboardClient::GetInstance()->GetPasteData(pasteData);
//    ASSERT_EQ(1, pasteData.GetRecordCount());
//    auto record = pasteData.GetRecordAt(0);
//    auto type = record->GetMimeType();
//    ASSERT_EQ(type, MIMETYPE_TEXT_HTML);
//    auto udType = record->GetUDType();
//    ASSERT_EQ(udType, UDMF::UDType::HTML);
//    auto plain = record->GetHtmlText();
//    auto textContent = record->GetTextContent();
//    ASSERT_EQ(*plain, text);
//    ASSERT_EQ(textContent, content);
//    auto details1 = record->GetDetails();
//    ASSERT_EQ(*details1, details);
//}

/**
* @tc.name: SetFile001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetFile001, TestSize.Level0)
{
    UDMF::UDDetails details;
    details.insert({ "1", "1" });
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UD_TYPE_MAP.at(UDMF::FILE);
    std::string uri = "file:/uri";
    std::shared_ptr<UDMF::File> fileRecord = std::make_shared<UDMF::File>(uri);
    fileRecord->SetDetails(details);
    data.AddRecord(fileRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::FILE);
    auto newFileRecord = static_cast<UDMF::File*>(newRecord.get());
    auto newDetails = newFileRecord->GetDetails();
    ASSERT_EQ(newDetails, details);
    auto uri2 = newFileRecord->GetUri();
    ASSERT_EQ(uri2, uri);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::FILE);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details);
}

/**
* @tc.name: SetImage001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetImage001, TestSize.Level0)
{
    UDMF::UDDetails details;
    details.insert({ "1", "1" });
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UD_TYPE_MAP.at(UDMF::IMAGE);
    std::string uri = "file:/image";
    std::shared_ptr<UDMF::Image> imageRecord = std::make_shared<UDMF::Image>(uri);
    imageRecord->SetDetails(details);
    data.AddRecord(imageRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::IMAGE);
    auto newImageRecord = static_cast<UDMF::Image*>(newRecord.get());
    auto newDetails = newImageRecord->GetDetails();
    ASSERT_EQ(newDetails, details);
    auto uri2 = newImageRecord->GetUri();
    ASSERT_EQ(uri2, uri);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::IMAGE);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details);
}

/**
* @tc.name: SetAudio001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetAudio001, TestSize.Level0)
{
    UDMF::UDDetails details;
    details.insert({ "1", "1" });
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UD_TYPE_MAP.at(UDMF::AUDIO);
    std::string uri = "file:/Audio";
    std::shared_ptr<UDMF::Audio> audioRecord = std::make_shared<UDMF::Audio>(uri);
    audioRecord->SetDetails(details);
    data.AddRecord(audioRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::AUDIO);
    auto newAudioRecord = static_cast<UDMF::Audio*>(newRecord.get());
    auto newDetails = newAudioRecord->GetDetails();
    ASSERT_EQ(newDetails, details);
    auto uri2 = newAudioRecord->GetUri();
    ASSERT_EQ(uri2, uri);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::AUDIO);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details);
}

/**
* @tc.name: SetVideo001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetVideo001, TestSize.Level0)
{
    UDMF::UDDetails details;
    details.insert({ "1", "1" });
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UD_TYPE_MAP.at(UDMF::VIDEO);
    std::string uri = "file:/Video";
    std::shared_ptr<UDMF::Video> videoRecord = std::make_shared<UDMF::Video>(uri);
    videoRecord->SetDetails(details);
    data.AddRecord(videoRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::VIDEO);
    auto newVideoRecord = static_cast<UDMF::Video*>(newRecord.get());
    auto newDetails = newVideoRecord->GetDetails();
    ASSERT_EQ(newDetails, details);
    auto uri2 = newVideoRecord->GetUri();
    ASSERT_EQ(uri2, uri);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::VIDEO);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details);
}

/**
* @tc.name: SetFolder001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetFolder001, TestSize.Level0)
{
    UDMF::UDDetails details;
    details.insert({ "1", "1" });
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UD_TYPE_MAP.at(UDMF::FOLDER);
    std::string uri = "file:/Folder";
    std::shared_ptr<UDMF::Folder> folderRecord = std::make_shared<UDMF::Folder>(uri);
    folderRecord->SetDetails(details);
    data.AddRecord(folderRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::FOLDER);
    auto newFolderRecord = static_cast<UDMF::Folder*>(newRecord.get());
    auto newDetails = newFolderRecord->GetDetails();
    ASSERT_EQ(newDetails, details);
    auto uri2 = newFolderRecord->GetUri();
    ASSERT_EQ(uri2, uri);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, MIMETYPE_TEXT_URI);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::UDType::FOLDER);
    auto uri1 = record->GetUri()->ToString();
    ASSERT_EQ(uri1, uri);
    auto details1 = record->GetDetails();
    ASSERT_EQ(*details1, details);
}

/**
* @tc.name: SetSystemDefined001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetSystemDefined001, TestSize.Level0)
{
    UDMF::UnifiedData data;
    std::shared_ptr<UDMF::SystemDefinedRecord> systemRecord = std::make_shared<UDMF::SystemDefinedRecord>();
    UDMF::UDDetails details1;
    details1.insert({ "1", "1" });
    systemRecord->SetDetails(details1);
    data.AddRecord(systemRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::SYSTEM_DEFINED_RECORD);
    auto newSystemRecord = static_cast<UDMF::SystemDefinedRecord*>(newRecord.get());
    ASSERT_EQ(newSystemRecord->GetDetails(), details1);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, UDMF::UD_TYPE_MAP.at(UDMF::SYSTEM_DEFINED_RECORD));
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::SYSTEM_DEFINED_RECORD);
    auto details = record->GetDetails();
    ASSERT_EQ(*details, details1);
}

/**
* @tc.name: SetAppItem001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetAppItem001, TestSize.Level0)
{
    UDMF::UnifiedData data;
    std::shared_ptr<UDMF::SystemDefinedAppItem> systemDefinedAppItem1 = std::make_shared<UDMF::SystemDefinedAppItem>();
    UDMF::UDDetails details1;
    details1.insert({ "1", "1" });
    std::string appId = "appId";
    std::string appIconId = "appIconId";
    std::string appName = "appName";
    std::string appLabelId = "appLabelId";
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    systemDefinedAppItem1->SetDetails(details1);
    systemDefinedAppItem1->SetAppId(appId);
    systemDefinedAppItem1->SetAppName(appName);
    systemDefinedAppItem1->SetAppIconId(appIconId);
    systemDefinedAppItem1->SetAppLabelId(appLabelId);
    systemDefinedAppItem1->SetBundleName(bundleName);
    systemDefinedAppItem1->SetAbilityName(abilityName);
    systemDefinedAppItem1->SetType(UDMF::SYSTEM_DEFINED_APP_ITEM);
    data.AddRecord(systemDefinedAppItem1);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::SYSTEM_DEFINED_APP_ITEM);
    auto newAppItem = static_cast<UDMF::SystemDefinedAppItem*>(newRecord.get());
    ASSERT_EQ(newAppItem->GetAppId(), appId);
    ASSERT_EQ(newAppItem->GetAppIconId(), appIconId);
    ASSERT_EQ(newAppItem->GetAppName(), appName);
    ASSERT_EQ(newAppItem->GetAppLabelId(), appLabelId);
    ASSERT_EQ(newAppItem->GetBundleName(), bundleName);
    ASSERT_EQ(newAppItem->GetAbilityName(), abilityName);
    ASSERT_EQ(newAppItem->GetDetails(), details1);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, UDMF::UD_TYPE_MAP.at(UDMF::SYSTEM_DEFINED_APP_ITEM));
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::SYSTEM_DEFINED_APP_ITEM);
    auto details = record->GetDetails();
    auto content = *(record->GetSystemDefinedContent());
    ASSERT_EQ(*details, details1);
    auto appIconId1 = std::get<std::string>(content["appIconId"]);
    auto appId1 = std::get<std::string>(content["appId"]);
    auto appName1 = std::get<std::string>(content["appName"]);
    auto appLabelId1 = std::get<std::string>(content["appLabelId"]);
    auto bundleName1 = std::get<std::string>(content["bundleName"]);
    auto abilityName1 = std::get<std::string>(content["abilityName"]);
    ASSERT_EQ(appId, appId1);
    ASSERT_EQ(appIconId, appIconId1);
    ASSERT_EQ(appName, appName1);
    ASSERT_EQ(appLabelId, appLabelId1);
    ASSERT_EQ(bundleName, bundleName1);
    ASSERT_EQ(abilityName, abilityName1);
}

/**
* @tc.name: SetForm001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetForm001, TestSize.Level0)
{
    UDMF::UnifiedData data;
    std::shared_ptr<UDMF::SystemDefinedForm> form = std::make_shared<UDMF::SystemDefinedForm>();
    UDMF::UDDetails details1;
    details1.insert({ "1", "1" });
    int32_t formId = 1;
    std::string formName = "formName";
    std::string module = "module";
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    form->SetDetails(details1);
    form->SetFormId(formId);
    form->SetFormName(formName);
    form->SetAbilityName(abilityName);
    form->SetBundleName(bundleName);
    form->SetModule(module);
    form->SetType(UDMF::SYSTEM_DEFINED_FORM);
    data.AddRecord(form);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::SYSTEM_DEFINED_FORM);
    auto newForm = static_cast<UDMF::SystemDefinedForm*>(newRecord.get());
    ASSERT_EQ(newForm->GetFormId(), formId);
    ASSERT_EQ(newForm->GetFormName(), formName);
    ASSERT_EQ(newForm->GetModule(), module);
    ASSERT_EQ(newForm->GetBundleName(), bundleName);
    ASSERT_EQ(newForm->GetAbilityName(), abilityName);
    ASSERT_EQ(newForm->GetDetails(), details1);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, UDMF::UD_TYPE_MAP.at(UDMF::SYSTEM_DEFINED_FORM));
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::SYSTEM_DEFINED_FORM);
    auto details = record->GetDetails();
    auto content = *(record->GetSystemDefinedContent());
    ASSERT_EQ(*details, details1);
    auto formId1 = std::get<std::int32_t>(content["formId"]);
    auto formName1 = std::get<std::string>(content["formName"]);
    auto module1 = std::get<std::string>(content["module"]);
    auto bundleName1 = std::get<std::string>(content["bundleName"]);
    auto abilityName1 = std::get<std::string>(content["abilityName"]);
    ASSERT_EQ(formId, formId1);
    ASSERT_EQ(formName, formName1);
    ASSERT_EQ(module, module1);
    ASSERT_EQ(bundleName, bundleName1);
    ASSERT_EQ(abilityName, abilityName1);
}

/**
* @tc.name: SetAppDefined001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetAppDefined001, TestSize.Level0)
{
    UDMF::UnifiedData data;
    auto typeStr = UDMF::UD_TYPE_MAP.at(UDMF::APPLICATION_DEFINED_RECORD);
    std::shared_ptr<UDMF::ApplicationDefinedRecord> appRecord = std::make_shared<UDMF::ApplicationDefinedRecord>();
    std::vector<uint8_t> rawData1(8, 1);
    std::map<std::string, std::vector<uint8_t>> customData;
    customData[typeStr] = rawData1;
    appRecord->SetApplicationDefinedType(typeStr);
    appRecord->SetRawData(rawData1);
    data.AddRecord(appRecord);
    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
    uniProps->tag = "myTag";
    data.SetProperties(uniProps);

    // SetUnifiedData
    PasteboardClient::GetInstance()->SetUnifiedData(data);
    // GetUnifiedData
    UDMF::UnifiedData newData;
    PasteboardClient::GetInstance()->GetUnifiedData(newData);
    ASSERT_EQ(1, newData.GetRecords().size());
    auto newRecord = newData.GetRecordAt(0);
    auto newType = newRecord->GetType();
    ASSERT_EQ(newType, UDMF::APPLICATION_DEFINED_RECORD);
    auto newSystemRecord = static_cast<UDMF::ApplicationDefinedRecord*>(newRecord.get());
    ASSERT_EQ(newSystemRecord->GetRawData(), rawData1);

    // GetPasteData
    PasteData pasteData;
    PasteboardClient::GetInstance()->GetPasteData(pasteData);
    ASSERT_EQ(1, pasteData.GetRecordCount());
    auto record = pasteData.GetRecordAt(0);
    auto type = record->GetMimeType();
    ASSERT_EQ(type, typeStr);
    auto udType = record->GetUDType();
    ASSERT_EQ(udType, UDMF::APPLICATION_DEFINED_RECORD);
    auto details = record->GetCustomData()->GetItemData();
    ASSERT_EQ(details, customData);
}

/**
* @tc.name: SetPixelMap001
* @tc.desc: Get the source of the data.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(PasteboardUnifiedDataTest, SetPixelMap001, TestSize.Level0)
{
//    UDMF::UnifiedData data;
//    std::shared_ptr<UDMF::UnifiedDataProperties> uniProps = std::make_shared<UDMF::UnifiedDataProperties>();
//    uniProps->tag = "myTag";
//    data.SetProperties(uniProps);
//    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
//    InitializationOptions opts = { { 5, 7 }, PixelFormat::ARGB_8888 };
//    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
//    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
//    auto rawData = PasteDataRecord::PixelMap2Vector(pixelMapIn);
//    auto pixelMapRecord = std::make_shared<UDMF::SystemDefinedPixelMap>(rawData);
//    data.AddRecord(pixelMapRecord);
//    PasteboardClient::GetInstance()->SetUnifiedData(data);
//
//    UDMF::UnifiedData newData;
//    PasteboardClient::GetInstance()->GetUnifiedData(newData);
//    auto record = newData.GetRecordAt(0);
//    auto type = record->GetType();
//    ASSERT_EQ(type, UDMF::SYSTEM_DEFINED_PIXEL_MAP);
//    auto content = static_cast<UDMF::SystemDefinedPixelMap*>(record.get())->GetRawData();
//    ASSERT_EQ(rawData, content);
}
}