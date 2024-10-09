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

#include "paste_data_record.h"
#include "tlv_object.h"
#include "unified_meta.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
using namespace testing;
using namespace OHOS::Media;
class PasteDataRecordTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    Details details_;
    std::vector<uint8_t> rawData_;
    std::string text_ = "test";
    std::string extraText_ = "extr";
    std::string uri_ = "file://123.txt";
    std::string html_ = "<div class='disable'>helloWorld</div>";
    std::string link_ = "http://abc.com";
    int32_t width_ = 5;
    int32_t height_ = 7;

    void CheckEntries(const std::vector<std::shared_ptr<PasteDataEntry>> &entries);
    void CheckPlainUds(const std::shared_ptr<PasteDataEntry> entry);
    void CheckFileUriUds(const std::shared_ptr<PasteDataEntry> entry);
    void CheckPixelMapUds(const std::shared_ptr<PasteDataEntry> entry);
    void CheckHtmlUds(const std::shared_ptr<PasteDataEntry> entry);
    void CheckLinkUds(const std::shared_ptr<PasteDataEntry> entry);

    void AddPlainUdsEntry(PasteDataRecord &record);
    void AddFileUriUdsEntry(PasteDataRecord &record);
    void AddHtmlUdsEntry(PasteDataRecord &record);
    void AddPixelMapUdsEntry(PasteDataRecord &record);
    void AddLinkUdsEntry(PasteDataRecord &record);
};

void PasteDataRecordTest::SetUpTestCase(void) {}

void PasteDataRecordTest::TearDownTestCase(void) {}

void PasteDataRecordTest::SetUp(void)
{
    rawData_ = { 1, 2, 3, 4, 5, 6, 7, 8 };
    details_.insert({ "keyString", "string_test" });
    details_.insert({ "keyInt32", 1 });
    details_.insert({ "keyBool", true });
    details_.insert({ "KeyU8Array", rawData_ });
    details_.insert({ "KeyDouble", 1.234 });
}

void PasteDataRecordTest::TearDown(void) {}

void PasteDataRecordTest::AddPlainUdsEntry(PasteDataRecord &record)
{
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::PLAIN_TEXT);
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::CONTENT] = text_;
    record.AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
}

void PasteDataRecordTest::AddFileUriUdsEntry(PasteDataRecord &record)
{
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI);
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::FILE_URI_PARAM] = uri_;
    object->value_[UDMF::FILE_TYPE] = "";
    record.AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
}

void PasteDataRecordTest::AddHtmlUdsEntry(PasteDataRecord &record)
{
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML);
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::HTML_CONTENT] = html_;
    record.AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
}

void PasteDataRecordTest::AddLinkUdsEntry(PasteDataRecord &record)
{
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HYPERLINK);
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::URL] = link_;
    record.AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
}

void PasteDataRecordTest::AddPixelMapUdsEntry(PasteDataRecord &record)
{
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP);
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    uint32_t color[100] = { 3, 7, 9, 9, 7, 6 };
    InitializationOptions opts = { { width_, height_ }, PixelFormat::ARGB_8888, PixelFormat::ARGB_8888 };
    std::unique_ptr<PixelMap> pixelMap = PixelMap::Create(color, sizeof(color) / sizeof(color[0]), opts);
    std::shared_ptr<PixelMap> pixelMapIn = move(pixelMap);
    object->value_[UDMF::PIXEL_MAP] = pixelMapIn;
    record.AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
}

void PasteDataRecordTest::CheckEntries(const std::vector<std::shared_ptr<PasteDataEntry>> &entries)
{
    for (auto const &entry : entries) {
        if (entry->GetUtdId() == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::PLAIN_TEXT)) {
            CheckPlainUds(entry);
        }
        if (entry->GetUtdId() == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI)) {
            CheckFileUriUds(entry);
        }
        if (entry->GetUtdId() == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP)) {
            CheckPixelMapUds(entry);
        }
        if (entry->GetUtdId() == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HYPERLINK)) {
            CheckLinkUds(entry);
        }
        if (entry->GetUtdId() == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML)) {
            CheckHtmlUds(entry);
        }
    }
}

void PasteDataRecordTest::CheckPlainUds(const std::shared_ptr<PasteDataEntry> entry)
{
    ASSERT_EQ(MIMETYPE_TEXT_PLAIN, entry->GetMimeType());
    auto decodeValue = entry->GetValue();
    auto object = std::get_if<std::shared_ptr<Object>>(&decodeValue);
    ASSERT_NE(object, nullptr);
    auto objectValue = (*object)->value_;
    auto typeValue = std::get_if<std::string>(&objectValue[UDMF::UNIFORM_DATA_TYPE]);
    ASSERT_NE(typeValue, nullptr);
    ASSERT_EQ(*typeValue, UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::PLAIN_TEXT));
    auto value = std::get_if<std::string>(&objectValue[UDMF::CONTENT]);
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(*value, text_);
}

void PasteDataRecordTest::CheckFileUriUds(const std::shared_ptr<PasteDataEntry> entry)
{
    ASSERT_EQ(MIMETYPE_TEXT_URI, entry->GetMimeType());
    auto decodeValue = entry->GetValue();
    auto object = std::get_if<std::shared_ptr<Object>>(&decodeValue);
    ASSERT_NE(object, nullptr);
    auto objectValue = (*object)->value_;
    auto typeValue = std::get_if<std::string>(&objectValue[UDMF::UNIFORM_DATA_TYPE]);
    ASSERT_NE(typeValue, nullptr);
    ASSERT_EQ(*typeValue, UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI));
    auto value = std::get_if<std::string>(&objectValue[UDMF::FILE_URI_PARAM]);
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(*value, uri_);
}

void PasteDataRecordTest::CheckHtmlUds(const std::shared_ptr<PasteDataEntry> entry)
{
    ASSERT_EQ(MIMETYPE_TEXT_HTML, entry->GetMimeType());
    auto decodeValue = entry->GetValue();
    auto object = std::get_if<std::shared_ptr<Object>>(&decodeValue);
    ASSERT_NE(object, nullptr);
    auto objectValue = (*object)->value_;
    auto typeValue = std::get_if<std::string>(&objectValue[UDMF::UNIFORM_DATA_TYPE]);
    ASSERT_NE(typeValue, nullptr);
    ASSERT_EQ(*typeValue, UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML));
    auto value = std::get_if<std::string>(&objectValue[UDMF::HTML_CONTENT]);
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(*value, html_);
}

void PasteDataRecordTest::CheckPixelMapUds(const std::shared_ptr<PasteDataEntry> entry)
{
    ASSERT_EQ(MIMETYPE_PIXELMAP, entry->GetMimeType());
    auto decodeValue = entry->GetValue();
    auto object = std::get_if<std::shared_ptr<Object>>(&decodeValue);
    ASSERT_NE(object, nullptr);
    auto objectValue = (*object)->value_;
    auto typeValue = std::get_if<std::string>(&objectValue[UDMF::UNIFORM_DATA_TYPE]);
    ASSERT_NE(typeValue, nullptr);
    ASSERT_EQ(*typeValue, UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP));
    auto value = std::get_if<std::shared_ptr<PixelMap>>(&objectValue[UDMF::PIXEL_MAP]);
    ASSERT_NE(value, nullptr);
    ImageInfo imageInfo = {};
    (*value)->GetImageInfo(imageInfo);
    ASSERT_TRUE(imageInfo.size.height == height_);
    ASSERT_TRUE(imageInfo.size.width == width_);
    ASSERT_TRUE(imageInfo.pixelFormat == PixelFormat::ARGB_8888);
}

void PasteDataRecordTest::CheckLinkUds(const std::shared_ptr<PasteDataEntry> entry)
{
    ASSERT_EQ(MIMETYPE_TEXT_PLAIN, entry->GetMimeType());
    auto decodeValue = entry->GetValue();
    auto object = std::get_if<std::shared_ptr<Object>>(&decodeValue);
    ASSERT_NE(object, nullptr);
    auto objectValue = (*object)->value_;
    auto typeValue = std::get_if<std::string>(&objectValue[UDMF::UNIFORM_DATA_TYPE]);
    ASSERT_NE(typeValue, nullptr);
    ASSERT_EQ(*typeValue, UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HYPERLINK));
    auto value = std::get_if<std::string>(&objectValue[UDMF::URL]);
    ASSERT_NE(value, nullptr);
    ASSERT_EQ(*value, link_);
}

/**
* @tc.name: GetValidTypesTest001
* @tc.desc: GetValidTypesTest001;
* @tc.type: FUNC
* @tc.require:entries
* @tc.author: tarowang
*/
HWTEST_F(PasteDataRecordTest, GetValidTypesTest001, TestSize.Level0)
{
    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    AddPlainUdsEntry(*record);
    AddHtmlUdsEntry(*record);

    std::vector<std::string> inputTypes;
    inputTypes.emplace_back(UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI));
    inputTypes.emplace_back(UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML));

    auto validTypes = record->GetValidTypes(inputTypes);
    ASSERT_EQ(validTypes.size(), 1);
    ASSERT_EQ(validTypes[0], UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML));
}

/**
* @tc.name: AddEntryTest001
* @tc.desc: Add entry test
* @tc.type: FUNC
* @tc.require:entries
* @tc.author: tarowang
*/
HWTEST_F(PasteDataRecordTest, AddEntryTest001, TestSize.Level0)
{
    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    AddPlainUdsEntry(*record);
    AddHtmlUdsEntry(*record);
}

/**
* @tc.name: GetEntries001
* @tc.desc: convert to palinText;
* @tc.type: FUNC
* @tc.require:entries
* @tc.author: tarowang
*/
HWTEST_F(PasteDataRecordTest, GetEntries001, TestSize.Level0)
{
    std::vector<std::string> inputTypes;
    inputTypes.emplace_back(UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI));
    inputTypes.emplace_back(UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML));
    inputTypes.emplace_back(UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI));
    inputTypes.emplace_back(UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HYPERLINK));
    inputTypes.emplace_back(UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP));

    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    AddPlainUdsEntry(*record);
    auto types = record->GetValidTypes(inputTypes);
    ASSERT_EQ(types.size(), 0);

    AddHtmlUdsEntry(*record);
    types = record->GetValidTypes(inputTypes);
    auto it = std::find(types.begin(), types.end(), UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML));
    ASSERT_NE(it, types.end());

    AddFileUriUdsEntry(*record);
    types = record->GetValidTypes(inputTypes);
    it = std::find(types.begin(), types.end(), UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI));
    ASSERT_NE(it, types.end());

    AddLinkUdsEntry(*record);
    types = record->GetValidTypes(inputTypes);
    it = std::find(types.begin(), types.end(), UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HYPERLINK));
    ASSERT_NE(it, types.end());

    AddPixelMapUdsEntry(*record);
    types = record->GetValidTypes(inputTypes);
    it = std::find(types.begin(), types.end(), UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP));
    ASSERT_NE(it, types.end());

    auto entries = record->GetEntries();
    CheckEntries(entries);
}

} // namespace OHOS::MiscServices