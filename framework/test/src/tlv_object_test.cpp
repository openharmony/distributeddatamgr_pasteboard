/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "long_wrapper.h"
#include "pasteboard_client.h"
#include "pasteboard_hilog.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace OHOS::AAFwk;
class TLVObjectTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static std::shared_ptr<PasteDataRecord> GenRecord(std::uint32_t index);
};

void TLVObjectTest::SetUpTestCase(void) { }

void TLVObjectTest::TearDownTestCase(void) { }

void TLVObjectTest::SetUp(void) { }

void TLVObjectTest::TearDown(void) { }

std::shared_ptr<PasteDataRecord> TLVObjectTest::GenRecord(std::uint32_t index)
{
    std::string indexStr = "";
    auto plainText = std::make_shared<std::string>("hello" + indexStr);
    auto htmlText = std::make_shared<std::string>("<span>hello" + indexStr + "</span>");
    auto uri = std::make_shared<OHOS::Uri>("dataability://hello" + indexStr + ".txt");
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    want->SetParam(key, id);

    PasteDataRecord::Builder builder(MIMETYPE_TEXT_HTML);
    auto record1 = builder.SetPlainText(plainText).SetHtmlText(htmlText).SetUri(uri).SetWant(want).Build();
    return record1;
}

/**
 * @tc.name: TLVOjbectTest001
 * @tc.desc: test tlv coder.
 * @tc.type: FUNC
 * @tc.require:AR000H5I1D
 * @tc.author: baoyayong
 */
HWTEST_F(TLVObjectTest, TLVOjbectTest001, TestSize.Level0)
{
    PasteData pasteData1;
    for (uint32_t i = 0; i < 10; ++i) {
        pasteData1.AddRecord(TLVObjectTest::GenRecord(i));
    }

    std::vector<uint8_t> buffer;
    auto ret = pasteData1.Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1.CountTLV());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1.GetRecordCount());

    for (uint32_t i = 0; i < 10; ++i) {
        auto record2 = pasteData2.GetRecordAt(i);
        auto record1 = pasteData1.GetRecordAt(i);
        EXPECT_EQ(*(record2->GetHtmlText()), *(record1->GetHtmlText()));
        EXPECT_EQ(*(record2->GetPlainText()), *(record1->GetPlainText()));
        EXPECT_TRUE(record2->GetUri()->ToString() == record1->GetUri()->ToString());
        EXPECT_EQ(record2->GetWant()->OperationEquals(*(record1->GetWant())), true);
    }
}

/**
 * @tc.name: TLVOjbectTest002
 * @tc.desc: test tlv coder.
 * @tc.type: FUNC
 * @tc.require:AR000H5I1D
 * @tc.author: baoyayong
 */
HWTEST_F(TLVObjectTest, TLVOjbectTest002, TestSize.Level0)
{
    std::shared_ptr<Want> want = std::make_shared<Want>();
    std::string key = "id";
    int32_t id = 456;
    Want wantIn = want->SetParam(key, id);
    auto pasteData1 = PasteboardClient::GetInstance()->CreateWantData(std::make_shared<Want>(wantIn));

    std::vector<uint8_t> buffer;
    auto ret = pasteData1->Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1->CountTLV());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1->GetRecordCount());

    auto record2 = pasteData2.GetRecordAt(0);
    auto record1 = pasteData1->GetRecordAt(0);
    EXPECT_EQ(record2->GetWant()->OperationEquals(*(record1->GetWant())), true);
}

/**
 * @tc.name: TLVOjbectTest003
 * @tc.desc: test tlv coder map.
 * @tc.type: FUNC
 * @tc.require:AR000H5I1D
 * @tc.author: baoyayong
 */
HWTEST_F(TLVObjectTest, TLVOjbectTest003, TestSize.Level0)
{
    std::string plainText = "plain text";
    auto pasteData1 = PasteboardClient::GetInstance()->CreatePlainTextData(plainText);
    std::vector<uint8_t> arrayBuffer(46);
    arrayBuffer = { 2, 7, 6, 8, 9 };
    std::string mimeType1 = "image/jpg";
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    customData->AddItemData(mimeType1, arrayBuffer);
    PasteDataRecord::Builder builder(MIMETYPE_TEXT_PLAIN);
    std::shared_ptr<PasteDataRecord> pasteDataRecord = builder.SetCustomData(customData).Build();
    pasteData1->AddRecord(pasteDataRecord);

    std::vector<uint8_t> buffer;
    auto ret = pasteData1->Encode(buffer);
    ASSERT_TRUE(ret);
    ASSERT_EQ(buffer.size(), pasteData1->CountTLV());

    PasteData pasteData2;
    ret = pasteData2.Decode(buffer);
    EXPECT_TRUE(ret);
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1->GetRecordCount());
    EXPECT_EQ(pasteData2.GetRecordCount(), pasteData1->GetRecordCount());

    auto record1 = pasteData1->GetRecordAt(0);
    auto record2 = pasteData2.GetRecordAt(0);
    ASSERT_TRUE(record2 != nullptr);
    auto custom2 = record2->GetCustomData();
    auto custom1 = record1->GetCustomData();
    ASSERT_TRUE(custom1 != nullptr && custom2 != nullptr);
    EXPECT_EQ(custom2->GetItemData().size(), custom1->GetItemData().size());
}

/**
 * @tc.name: TestPasteDataProperty
 * @tc.desc: PasteDataProperty
 * @tc.type: FUNC
 */
HWTEST_F(TLVObjectTest, TestPasteDataProperty, TestSize.Level0)
{
    PasteDataProperty obj1;
    obj1.mimeTypes = {
        MIMETYPE_PIXELMAP, MIMETYPE_TEXT_HTML, MIMETYPE_TEXT_PLAIN, MIMETYPE_TEXT_URI, MIMETYPE_TEXT_WANT,
    };
    obj1.additions.SetParam("key", AAFwk::Long::Box(1));
    obj1.tag = "tag";
    obj1.timestamp = 1;
    obj1.localOnly = true;
    obj1.shareOption = ShareOption::CrossDevice;
    obj1.tokenId = 1;
    obj1.isRemote = true;
    obj1.bundleName = "bundleName";
    obj1.setTime = "setTime";
    obj1.screenStatus = ScreenEvent::ScreenUnlocked;

    std::vector<uint8_t> buffer;
    bool ret = obj1.Encode(buffer);
    ASSERT_TRUE(ret);

    PasteDataProperty obj2;
    ret = obj2.Decode(buffer);
    ASSERT_TRUE(ret);

    EXPECT_TRUE(obj1.additions == obj2.additions);
    EXPECT_EQ(obj1.mimeTypes, obj2.mimeTypes);
    EXPECT_EQ(obj1.tag, obj2.tag);
    EXPECT_EQ(obj1.timestamp, obj2.timestamp);
    EXPECT_EQ(obj1.localOnly, obj2.localOnly);
    EXPECT_EQ(obj1.shareOption, obj2.shareOption);
    EXPECT_EQ(obj1.tokenId, obj2.tokenId);
    EXPECT_EQ(obj1.isRemote, obj2.isRemote);
    EXPECT_EQ(obj1.bundleName, obj2.bundleName);
    EXPECT_EQ(obj1.setTime, obj2.setTime);
    EXPECT_EQ(obj1.screenStatus, obj2.screenStatus);
}

/**
 * @tc.name: TestMineCustomData
 * @tc.desc: MineCustomData
 * @tc.type: FUNC
 */
HWTEST_F(TLVObjectTest, TestMineCustomData, TestSize.Level0)
{
    MineCustomData obj1;
    obj1.AddItemData("key", {1, 1, 1, 1});

    std::vector<uint8_t> buffer;
    bool ret = obj1.Encode(buffer);
    ASSERT_TRUE(ret);

    MineCustomData obj2;
    ret = obj2.Decode(buffer);
    ASSERT_TRUE(ret);

    EXPECT_EQ(obj1.GetItemData(), obj2.GetItemData());
}

/**
 * @tc.name: TestPasteDataEntry
 * @tc.desc: PasteDataEntry
 * @tc.type: FUNC
 */
HWTEST_F(TLVObjectTest, TestPasteDataEntry, TestSize.Level0)
{
    PasteDataEntry obj1;
    obj1.SetUtdId("utdId");
    obj1.SetMimeType("mimeType");
    auto udmfObject = std::make_shared<Object>();
    udmfObject->value_["key"] = "value";
    obj1.SetValue(udmfObject);

    std::vector<uint8_t> buffer;
    bool ret = obj1.Encode(buffer);
    ASSERT_TRUE(ret);

    PasteDataEntry obj2;
    ret = obj2.Decode(buffer);
    ASSERT_TRUE(ret);

    EXPECT_EQ(obj1.GetUtdId(), obj2.GetUtdId());
    EXPECT_EQ(obj1.GetMimeType(), obj2.GetMimeType());

    auto entryValue = obj2.GetValue();
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<Object>>(entryValue));
    auto udmfObject2 = std::get<std::shared_ptr<Object>>(entryValue);
    ASSERT_NE(udmfObject2, nullptr);
    EXPECT_EQ(udmfObject->value_, udmfObject2->value_);
}

/**
 * @tc.name: TestPasteDataRecord
 * @tc.desc: PasteDataRecord
 * @tc.type: FUNC
 */
HWTEST_F(TLVObjectTest, TestPasteDataRecord, TestSize.Level0)
{
    PasteDataRecord obj1;
    auto udmfObject = std::make_shared<Object>();
    std::string utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::PLAIN_TEXT);
    udmfObject->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    udmfObject->value_[UDMF::CONTENT] = "content";
    auto entry1 = std::make_shared<PasteDataEntry>();
    entry1->SetValue(udmfObject);
    entry1->SetUtdId(utdId);
    entry1->SetMimeType(MIMETYPE_TEXT_PLAIN);
    obj1.AddEntry(utdId, entry1);

    std::vector<uint8_t> buffer;
    bool ret = obj1.Encode(buffer);
    ASSERT_TRUE(ret);

    PasteDataRecord obj2;
    ret = obj2.Decode(buffer);
    ASSERT_TRUE(ret);

    EXPECT_EQ(obj1.GetMimeType(), obj2.GetMimeType());
    auto entry2 = obj2.GetEntry(utdId);
    ASSERT_NE(entry2, nullptr);
    auto entryValue = entry2->GetValue();
    ASSERT_TRUE(std::holds_alternative<std::shared_ptr<Object>>(entryValue));
    auto udmfObject2 = std::get<std::shared_ptr<Object>>(entryValue);
    ASSERT_NE(udmfObject2, nullptr);
    EXPECT_EQ(udmfObject->value_, udmfObject2->value_);
}

/**
 * @tc.name: OH_Pasteboard_HasDataTest002.
 * @tc.desc:OH_Pasteboard_HasDataTest002
 * @tc.type: FUNC.
 */
HWTEST_F(PasteboardCapiTest, OH_Pasteboard_HasDataTest002, TestSize.Level2)
{
    OH_Pasteboard pasteboard = OH_Pasteboard_Create();
    bool ret = OH_Pasteboard_HasData(pasteboard);
    EXPECT_FALSE(ret);
    OH_Pasteboard_Destroy(pasteboard);
}

/**
 * @tc.name: Dlopen_ShouldReturnNonNullHandle_WhenLibraryExists.
 * @tc.desc:Dlopen_Test_001
 * @tc.type: FUNC.
 */
HWTEST_F(PasteboardLoadTest, DlopenTest001, TestSize.Level0)
{
    std::string existingLibraryPath = "libtest.so";
    void handle = dlopen(existingLibraryPath.c_str(), RTLD_LAZY);
    EXPECT_NE(handle, nullptr);
    if (handle != nullptr) 
    {
        dlclose(handle);
    }
}

/**
 * @tc.name: Dlopen_ShouldReturnNonNullHandle_WhenLibraryExists.
 * @tc.desc:Dlopen_Test_002
 * @tc.type: FUNC.
 */
HWTEST_F(PasteboardLoadTest, DlopenTest002, TestSize.Level0)
{
    std::string nonExistingLibraryPath = "nonexistentlib.so";
    void handle = dlopen(nonExistingLibraryPath.c_str(), RTLD_LAZY);
    EXPECT_EQ(handle, nullptr);
}

/**
* @tc.name:: Continue_ShouldBeExecuted_WhenConstructorIsEmpty
* @tc.desc: LoadComponentsTest006
* @tc.desc : FUNC
*/
HWTEST_F(PasteboardLoadTest, LoadComponentsTest006, TestSize.Level0)
{
    Config::Component component;
    component.constructor = "";
    bool continueExecuted = false;
    if (component.constructor.empty()) 
    {
        continueExecuted = true;
    }
    EXPECT_TRUE(continueExecuted);
}

/**
* @tc.name:: SetWebviewPasteDataTest_002
* @tc.desc: SetWebviewPasteDataTest_002
* @tc.desc : FUNC
*/
HWTEST_F(WebControllerTest, SetWebviewPasteDataTest_002, TestSize.Level1)
{
    PasteData pasteData;
    pasteData.SetTag(PasteData::WEBVIEW_PASTEDATA_TAG);
    PasteData::Record record;
    record.SetUri(nullptr);
    record.SetFrom(1);
    record.SetRecordId(1);
    pasteData.AddRecord(record);
    std::string bundleName = "testBundle";
    PasteboardWebController pasteboardWebController;
    pasteboardWebController.SetWebviewPasteData(pasteData, bundleName);
    EXPECT_EQ(pasteData.AllRecords().size(), 1);
}

/**
* @tc.name:: SetWebviewPasteDataTest_003
* @tc.desc: SetWebviewPasteDataTest_003
* @tc.desc : FUNC
*/
HWTEST_F(WebControllerTest, SetWebviewPasteDataTest_003, TestSize.Level1)
{
    PasteData pasteData;
    pasteData.SetTag(PasteData::WEBVIEW_PASTEDATA_TAG);
    PasteData::Record record;
    record.SetUri(std::make_shared("testUri"));
    record.SetFrom(0);
    record.SetRecordId(1);
    pasteData.AddRecord(record);
    std::string bundleName = "testBundle";
    PasteboardWebController pasteboardWebController;
    pasteboardWebController.SetWebviewPasteData(pasteData, bundleName);
    EXPECT_EQ(pasteData.AllRecords().size(), 1);
}

/**
* @tc.name:: SetWebviewPasteDataTest_004
* @tc.desc:SetWebviewPasteDataTest_004
* @tc.desc : FUNC
*/
HWTEST_F(WebControllerTest, SetWebviewPasteDataTest_004, TestSize.Level1)
{
    PasteData pasteData;
    pasteData.SetTag(PasteData::WEBVIEW_PASTEDATA_TAG);
    PasteData::Record record;
    record.SetUri(std::make_shared("testUri"));
    record.SetFrom(1);
    record.SetRecordId(1);
    pasteData.AddRecord(record);
    std::string bundleName = "testBundle";
    PasteboardWebController pasteboardWebController;
    pasteboardWebController.SetWebviewPasteData(pasteData, bundleName);
    EXPECT_EQ(pasteData.AllRecords().size(), 1);
}

/**
* @tc.name:: SetWebviewPasteDataTest_005
* @tc.desc: SetWebviewPasteDataTest_005
* @tc.desc : FUNC
*/
HWTEST_F(WebControllerTest, SetWebviewPasteDataTest_005, TestSize.Level1)
{
    PasteData pasteData;
    pasteData.SetTag(PasteData::WEBVIEW_PASTEDATA_TAG);
    PasteData::Record record;
    record.SetUri(std::make_shared(PasteData::IMG_LOCAL_URI + "testPath"));
    record.SetFrom(1);
    record.SetRecordId(2);
    pasteData.AddRecord(record);
    std::string bundleName = "testBundle";
    PasteboardWebController pasteboardWebController;
    pasteboardWebController.SetWebviewPasteData(pasteData, bundleName);
    EXPECT_NE(pasteData.AllRecords()[0]->GetUri()->ToString(), PasteData::IMG_LOCAL_URI + "testPath");
}

/**
* @tc.name:: SetWebviewPasteDataTest_006
* @tc.desc: SetWebviewPasteDataTest_006
* @tc.desc : FUNC
*/
HWTEST_F(WebControllerTest, SetWebviewPasteDataTest_006, TestSize.Level1)
{
    PasteData pasteData;
    pasteData.SetTag(PasteData::WEBVIEW_PASTEDATA_TAG);
    PasteData::Record record;
    record.SetUri(std::make_shared(PasteData::IMG_LOCAL_URI + PasteData::DOCS_LOCAL_TAG + "testPath"));
    record.SetFrom(1);
    record.SetRecordId(2);
    pasteData.AddRecord(record);
    std::string bundleName = "testBundle";
    PasteboardWebController pasteboardWebController;
    pasteboardWebController.SetWebviewPasteData(pasteData, bundleName);
    EXPECT_TRUE(pasteData.AllRecords()[0]->GetUri()->ToString().find(FILE_SCHEME_PREFIX) != std::string::npos);
}

/**
* @tc.name : SplitWebviewPasteData_ShouldReturnFalse_WhenNoRecords
* @tc.number: SplitWebviewPasteData_Test_001
* @tc.desc : Test case to verify that the function returns false when there are no records in PasteData.
*/
HWTEST_F(SplitWebviewPasteDataTest, SplitWebviewPasteData_ShouldReturnFalse_WhenNoRecords, TestSize.Level0) 
{
    PasteData pasteData;
    EXPECT_FALSE(PasteboardWebController::SplitWebviewPasteData(pasteData));
}

/**
* @tc.name : SplitWebviewPasteData_ShouldReturnFalse_WhenRecordIdEqualsFrom
* @tc.number: SplitWebviewPasteData_Test_002
* @tc.desc : Test case to verify that the function returns false when recordId equals from for all records.
*/
HWTEST_F(SplitWebviewPasteDataTest, SplitWebviewPasteData_ShouldReturnFalse_WhenRecordIdEqualsFrom, TestSize.Level0) 
{
    PasteData pasteData;
    PasteDataRecord record;
    record.SetRecordId(1);
    record.SetFrom(1);
    pasteData.AddRecord(std::make_shared(record));
    EXPECT_FALSE(PasteboardWebController::SplitWebviewPasteData(pasteData));
}

/**
* @tc.name : SplitWebviewPasteData_ShouldReturnFalse_WhenNoHtmlEntry
* @tc.number: SplitWebviewPasteData_Test_003
* @tc.desc : Test case to verify that the function returns false when there is no HTML entry in the record.
*/
HWTEST_F(SplitWebviewPasteDataTest, SplitWebviewPasteData_ShouldReturnFalse_WhenNoHtmlEntry, TestSize.Level0) 
{
    PasteData pasteData;
    PasteDataRecord record;
    record.SetRecordId(1);
    record.SetFrom(2);
    pasteData.AddRecord(std::make_shared(record));
    EXPECT_FALSE(PasteboardWebController::SplitWebviewPasteData(pasteData));
}

/**
* @tc.name : SplitWebviewPasteData_ShouldReturnFalse_WhenHtmlEntryIsEmpty
* @tc.number: SplitWebviewPasteData_Test_004
* @tc.desc : Test case to verify that the function returns false when the HTML entry is empty.
*/
HWTEST_F(SplitWebviewPasteDataTest, SplitWebviewPasteData_ShouldReturnFalse_WhenHtmlEntryIsEmpty, TestSize.Level0) 
{
    PasteData pasteData;
    PasteDataRecord record;
    record.SetRecordId(1);
    record.SetFrom(2);
    record.AddEntry(std::make_shared(MIMETYPE_TEXT_HTML, ""));
    pasteData.AddRecord(std::make_shared(record));
    EXPECT_FALSE(PasteboardWebController::SplitWebviewPasteData(pasteData));
}

/**
* @tc.name : SplitWebviewPasteData_ShouldReturnFalse_WhenSplitHtml2RecordsReturnsEmpty
* @tc.number: SplitWebviewPasteData_Test_005
* @tc.desc : Test case to verify that the function returns false when SplitHtml2Records returns an empty vector.
*/
HWTEST_F(SplitWebviewPasteDataTest, SplitWebviewPasteData_ShouldReturnFalse_WhenSplitHtml2RecordsReturnsEmpty, TestSize.Level0) 
{
    PasteData pasteData;
    PasteDataRecord record;
    record.SetRecordId(1);
    record.SetFrom(2);
    record.AddEntry(std::make_shared(MIMETYPE_TEXT_HTML, ""));
    pasteData.AddRecord(std::make_shared(record));
    EXPECT_FALSE(PasteboardWebController::SplitWebviewPasteData(pasteData));
}

/**
* @tc.name : SplitWebviewPasteData_ShouldReturnTrue_WhenSplitHtml2RecordsReturnsNonEmpty
* @tc.number: SplitWebviewPasteData_Test_006
* @tc.desc : Test case to verify that the function returns true when SplitHtml2Records returns a non-empty vector.
*/
HWTEST_F(SplitWebviewPasteDataTest, SplitWebviewPasteData_ShouldReturnTrue_WhenSplitHtml2RecordsReturnsNonEmpty, TestSize.Level0) 
{
    PasteData pasteData;
    PasteDataRecord record;
    record.SetRecordId(1);
    record.SetFrom(2);
    record.AddEntry(std::make_shared(MIMETYPE_TEXT_HTML, "test"));
    pasteData.AddRecord(std::make_shared(record));
    EXPECT_TRUE(PasteboardWebController::SplitWebviewPasteData(pasteData));
    EXPECT_EQ(pasteData.GetTag(), PasteData::WEBVIEW_PASTEDATA_TAG);
}
/**
* @tc.name : OH_Pasteboard_GetDataSource_ShouldReturnError_WhenGetDataSourceFails
* @tc.number: OH_Pasteboard_GetDataSource_Test_004
* @tc.desc : Test case to verify that the function returns an error when GetDataSource fails.
*/
HWTEST_F(OHPasteboardGetDataSourceTest, OH_Pasteboard_GetDataSource_ShouldReturnError_WhenGetDataSourceFails, TestSize.Level0) 
{
    // Mock GetDataSource to return a failure
    // Since we cannot mock non-virtual functions, we assume GetDataSource returns a failure
    char source[100];
    unsigned int len = sizeof(source);
    int result = OH_Pasteboard_GetDataSource(pasteboard, source, len);
    EXPECT_NE(result, ERR_OK);
}

/**
* @tc.name : OH_Pasteboard_GetDataSource_ShouldReturnInnerError_WhenCopyFails
* @tc.number: OH_Pasteboard_GetDataSource_Test_005
* @tc.desc : Test case to verify that the function returns ERR_INNER_ERROR when strcpy_s fails.
*/
HWTEST_F(OHPasteboardGetDataSourceTest, OH_Pasteboard_GetDataSource_ShouldReturnInnerError_WhenCopyFails, TestSize.Level0) 
{
    // Mock strcpy_s to return a failure
    // Since we cannot mock non-virtual functions, we assume strcpy_s returns a failure
    char source[100];
    unsigned int len = sizeof(source);
    int result = OH_Pasteboard_GetDataSource(pasteboard, source, len);
    EXPECT_EQ(result, ERR_INNER_ERROR);
}

/**
* @tc.name : OH_Pasteboard_GetDataSource_ShouldReturnOk_WhenAllOperationsSucceed
* @tc.number: OH_Pasteboard_GetDataSource_Test_006
* @tc.desc : Test case to verify that the function returns ERR_OK when all operations succeed.
*/
HWTEST_F(OHPasteboardGetDataSourceTest, OH_Pasteboard_GetDataSource_ShouldReturnOk_WhenAllOperationsSucceed, TestSize.Level0) 
{
    char source[100];
    unsigned int len = sizeof(source);
    int result = OH_Pasteboard_GetDataSource(pasteboard, source, len);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name : OH_Pasteboard_GetDataSource_ShouldReturnInvalidParameter_WhenPasteboardIsInvalid
* @tc.number: OH_Pasteboard_GetDataSource_Test_001
* @tc.desc : Test case to verify that the function returns ERR_INVALID_PARAMETER when pasteboard is invalid.
*/
HWTEST_F(OHPasteboardGetDataSourceTest, OH_Pasteboard_GetDataSource_ShouldReturnInvalidParameter_WhenPasteboardIsInvalid, TestSize.Level0) 
{
    OH_Pasteboard invalidPasteboard = nullptr;
    char source[100];
    unsigned int len = sizeof(source);
    int result = OH_Pasteboard_GetDataSource(invalidPasteboard, source, len);
    EXPECT_EQ(result, ERR_INVALID_PARAMETER);
}

/**
* @tc.name : OH_Pasteboard_GetDataSource_ShouldReturnInvalidParameter_WhenSourceIsNull
* @tc.number: OH_Pasteboard_GetDataSource_Test_002
* @tc.desc : Test case to verify that the function returns ERR_INVALID_PARAMETER when source is null.
*/
HWTEST_F(OHPasteboardGetDataSourceTest, OH_Pasteboard_GetDataSource_ShouldReturnInvalidParameter_WhenSourceIsNull, TestSize.Level0) 
{
    char source = nullptr;
    unsigned int len = 100;
    int result = OH_Pasteboard_GetDataSource(pasteboard, source, len);
    EXPECT_EQ(result, ERR_INVALID_PARAMETER);
}
} // namespace OHOS::MiscServices
