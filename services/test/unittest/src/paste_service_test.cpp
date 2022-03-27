/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include <cstdint>
#include <vector>
#include <ohos/aafwk/content/want.h>
#include "pasteboard_client.h"
#include "uri.h"
#include "pasteboard_observer_callback.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::MiscServices;

class PasteboardServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void PasteboardServiceTest::SetUpTestCase(void)
{
    
}

void PasteboardServiceTest::TearDownTestCase(void)
{

}

void PasteboardServiceTest::SetUp(void)
{
}

void PasteboardServiceTest::TearDown(void)
{}

void PasteboardObserverCallback::OnPasteboardChanged()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "callback.");
}

namespace {
/**
* @tc.name: PasteboardTest001
* @tc.desc: Create paste board test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteboardTest001, TestSize.Level0)
{
    auto record = PasteboardClient::GetInstance()->CreatePlainTextRecord("paste record1");
    EXPECT_TRUE(record != nullptr);

    auto data = PasteboardClient::GetInstance()->CreatePlainTextData("paste data1");
    EXPECT_TRUE(data != nullptr);

    PasteboardClient::GetInstance()->SetPasteData(*data);
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "get.");
    EXPECT_TRUE(ok == true);
    auto textPtr = pasteData.GetPrimaryText();
}

/**
* @tc.name: PasteboardTest002
* @tc.desc: Create paste board test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteboardTest002, TestSize.Level0)
{
    auto observer = std::make_shared<PasteboardObserverCallback>();
    PasteboardClient::GetInstance()->AddPasteboardChangedObserver(observer);

    auto data = PasteboardClient::GetInstance()->CreatePlainTextData("call back");
    EXPECT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->SetPasteData(*data);
    PasteboardClient::GetInstance()->Clear();
    PasteboardClient::GetInstance()->RemovePasteboardChangedObserver(observer);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "end.");
}

/**
* @tc.name: PasteboardTest001
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest001, TestSize.Level0)
{
    auto record = PasteboardClient::GetInstance()->CreatePlainTextRecord("plain text");
    EXPECT_TRUE(record != nullptr);
}

/**
* @tc.name: PasteRecordTest002
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest002, TestSize.Level0)
{
    auto record = PasteboardClient::GetInstance()->CreateHtmlTextRecord("html record");
    EXPECT_TRUE(record != nullptr);
}

/**
* @tc.name: PasteRecordTest003
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest003, TestSize.Level0)
{
    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
    auto record = PasteboardClient::GetInstance()->CreateWantRecord(want);
    EXPECT_TRUE(record != nullptr);
}

/**
* @tc.name: PasteRecordTest004
* @tc.desc: Create paste board record test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteRecordTest004, TestSize.Level0)
{
    OHOS::Uri uri("uri");
    auto record = PasteboardClient::GetInstance()->CreateUriRecord(uri);
    EXPECT_TRUE(record != nullptr);
}


/**
* @tc.name: PasteDataTest001
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest001, TestSize.Level0)
{
    std::shared_ptr<OHOS::AAFwk::Want> want = std::make_shared<OHOS::AAFwk::Want>();
    auto data = PasteboardClient::GetInstance()->CreateWantData(want);
    EXPECT_TRUE(data != nullptr);
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has != true); 
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    EXPECT_TRUE(ok == true);
    auto record = pasteData.GetPrimaryWant();
    EXPECT_TRUE(record != nullptr);
}

/**
* @tc.name: PasteDataTest002
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest002, TestSize.Level0)
{
    OHOS::Uri uri("uri");
    auto data = PasteboardClient::GetInstance()->CreateUriData(uri);
    EXPECT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has != true);
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true); 
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    EXPECT_TRUE(ok == true);
    auto record = pasteData.GetPrimaryUri();
    EXPECT_TRUE(record != nullptr);
}

/**
* @tc.name: PasteDataTest003
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest003, TestSize.Level0)
{
    std::string text = "plain text";
    auto data = PasteboardClient::GetInstance()->CreatePlainTextData(text);
    EXPECT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has != true); 
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true);
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    EXPECT_TRUE(ok == true);
    auto record = pasteData.GetPrimaryText();
    EXPECT_TRUE(record != nullptr);
}

/**
* @tc.name: PasteDataTest003
* @tc.desc: Create paste board data test.
* @tc.type: FUNC
*/
HWTEST_F(PasteboardServiceTest, PasteDataTest004, TestSize.Level0)
{
    std::string html = "plain text";
    auto data = PasteboardClient::GetInstance()->CreateHtmlData(html);
    EXPECT_TRUE(data != nullptr);
    PasteboardClient::GetInstance()->Clear();
    auto has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has != true);
    PasteboardClient::GetInstance()->SetPasteData(*data);
    has = PasteboardClient::GetInstance()->HasPasteData();
    EXPECT_TRUE(has == true); 
    PasteData pasteData;
    auto ok = PasteboardClient::GetInstance()->GetPasteData(pasteData);
    EXPECT_TRUE(ok == true);
    auto record = pasteData.GetPrimaryHtml();
    EXPECT_TRUE(record != nullptr);
}
}
