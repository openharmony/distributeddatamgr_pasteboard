/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "tlv_readable.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::MiscServices;
class TLVReadableTest : public testing::Test {
public:
    TLVReadableTest() {};
    ~TLVReadableTest() {};
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TLVReadableTest::SetUpTestCase(void) { }

void TLVReadableTest::TearDownTestCase(void) { }

void TLVReadableTest::SetUp(void) { }

void TLVReadableTest::TearDown(void) { }

/**
 * @tc.name: ReadValueTest001
 * @tc.desc: ReadValue
 * @tc.type: FUNC
 */
HWTEST_F(TLVReadableTest, ReadValueTest001, TestSize.Level1)
{
    std::vector<std::uint8_t> buffer;
    ReadOnlyBuffer buff(buffer);
    void *value = nullptr;
    TLVHead head;
    bool res = buff.ReadValue(value, head);
    EXPECT_TRUE(res);
}

/**
 * @tc.name: ReadValueTest002
 * @tc.desc: ReadValue
 * @tc.type: FUNC
 */
HWTEST_F(TLVReadableTest, ReadValueTest002, TestSize.Level1)
{
    std::vector<std::uint8_t> buffer;
    ReadOnlyBuffer buff(buffer);
    TLVHead head;
    head.len = 0;
    bool value = true;
    bool res = buff.ReadValue(value, head);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: ReadValueTest003
 * @tc.desc: ReadValue
 * @tc.type: FUNC
 */
HWTEST_F(TLVReadableTest, ReadValueTest003, TestSize.Level1)
{
    std::vector<std::uint8_t> buffer;
    ReadOnlyBuffer buff(buffer);
    TLVHead head;
    head.len = 0;
    int32_t value = 10;
    bool res = buff.ReadValue(value, head);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: ReadHeadTest001
 * @tc.desc: test ReadHead with empty buffer
 * @tc.type: FUNC
 */
HWTEST_F(TLVReadableTest, ReadHeadTest001, TestSize.Level1)
{
    std::vector<std::uint8_t> buffer;
    ReadOnlyBuffer buff(buffer);
    TLVHead head;
    bool res = buff.ReadHead(head);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: ReadHeadTest002
 * @tc.desc: test ReadHead with buffer size less than sizeof(TLVHead)
 * @tc.type: FUNC
 */
HWTEST_F(TLVReadableTest, ReadHeadTest002, TestSize.Level1)
{
    std::vector<std::uint8_t> buffer(1);
    ReadOnlyBuffer buff(buffer);
    TLVHead head;
    bool res = buff.ReadHead(head);
    EXPECT_FALSE(res);
}

/**
 * @tc.name: ReadHeadTest003
 * @tc.desc: test ReadHead with buffer size equal to sizeof(TLVHead)
 * @tc.type: FUNC
 */
HWTEST_F(TLVReadableTest, ReadHeadTest003, TestSize.Level1)
{
    std::vector<std::uint8_t> buffer(sizeof(TLVHead));
    TLVHead *pHead = reinterpret_cast<TLVHead *>(buffer.data());
    pHead->tag = 1;
    pHead->len = 10;
    ReadOnlyBuffer buff(buffer);
    TLVHead head;
    bool res = buff.ReadHead(head);
    EXPECT_TRUE(res);
    EXPECT_EQ(head.tag, 1);
    EXPECT_EQ(head.len, 10);
}

/**
 * @tc.name: ReadHeadTest004
 * @tc.desc: test ReadHead with buffer size larger than sizeof(TLVHead)
 * @tc.type: FUNC
 */
HWTEST_F(TLVReadableTest, ReadHeadTest004, TestSize.Level1)
{
    std::vector<std::uint8_t> buffer(sizeof(TLVHead) + 100);
    TLVHead *pHead = reinterpret_cast<TLVHead *>(buffer.data());
    pHead->tag = 100;
    pHead->len = 50;
    ReadOnlyBuffer buff(buffer);
    TLVHead head;
    bool res = buff.ReadHead(head);
    EXPECT_TRUE(res);
    EXPECT_EQ(head.tag, 100);
    EXPECT_EQ(head.len, 50);
}

/**
 * @tc.name: ReadHeadTest005
 * @tc.desc: test ReadHead called multiple times to verify cursor advancement
 * @tc.type: FUNC
 */
HWTEST_F(TLVReadableTest, ReadHeadTest005, TestSize.Level1)
{
    std::vector<std::uint8_t> buffer(sizeof(TLVHead) * 3);
    TLVHead *pHead = reinterpret_cast<TLVHead *>(buffer.data());
    pHead->tag = 1;
    pHead->len = 0;
    TLVHead *pHead2 = reinterpret_cast<TLVHead *>(buffer.data() + sizeof(TLVHead));
    pHead2->tag = 2;
    pHead2->len = 0;
    TLVHead *pHead3 = reinterpret_cast<TLVHead *>(buffer.data() + sizeof(TLVHead) * 2);
    pHead3->tag = 3;
    pHead3->len = 0;
    ReadOnlyBuffer buff(buffer);
    TLVHead head1, head2, head3;
    bool res1 = buff.ReadHead(head1);
    EXPECT_TRUE(res1);
    EXPECT_EQ(head1.tag, 1);
    bool res2 = buff.ReadHead(head2);
    EXPECT_TRUE(res2);
    EXPECT_EQ(head2.tag, 2);
    bool res3 = buff.ReadHead(head3);
    EXPECT_TRUE(res3);
    EXPECT_EQ(head3.tag, 3);
}
