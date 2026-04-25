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
#include "tlv_utils.h"
#include "parcel.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::MiscServices;

class TLVUtilsTest : public testing::Test {
public:
    TLVUtilsTest() {};
    ~TLVUtilsTest() {};
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void TLVUtilsTest::SetUpTestCase(void) { }

void TLVUtilsTest::TearDownTestCase(void) { }

void TLVUtilsTest::SetUp(void) { }

void TLVUtilsTest::TearDown(void) { }

/**
 * @tc.name: Parcelable2RawTest001
 * @tc.desc: test Parcelable2Raw with nullptr
 * @tc.type: FUNC
 */
HWTEST_F(TLVUtilsTest, Parcelable2RawTest001, TestSize.Level1)
{
    RawMem result = TLVUtils::Parcelable2Raw(nullptr);
    EXPECT_EQ(result.buffer, 0);
    EXPECT_EQ(result.bufferLen, 0);
    EXPECT_EQ(result.parcel, nullptr);
}

/**
 * @tc.name: Raw2ParcelTest001
 * @tc.desc: test Raw2Parcel with empty buffer
 * @tc.type: FUNC
 */
HWTEST_F(TLVUtilsTest, Raw2ParcelTest001, TestSize.Level1)
{
    RawMem rawMem;
    rawMem.buffer = 0;
    rawMem.bufferLen = 0;
    Parcel parcel(nullptr);
    bool result = TLVUtils::Raw2Parcel(rawMem, parcel);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: Raw2ParcelTest002
 * @tc.desc: test Raw2Parcel with valid buffer
 * @tc.type: FUNC
 */
HWTEST_F(TLVUtilsTest, Raw2ParcelTest002, TestSize.Level1)
{
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    RawMem rawMem;
    rawMem.buffer = reinterpret_cast<uintptr_t>(data.data());
    rawMem.bufferLen = data.size();
    Parcel parcel(nullptr);
    bool result = TLVUtils::Raw2Parcel(rawMem, parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: Vector2PixelMapTest001
 * @tc.desc: test Vector2PixelMap with empty vector
 * @tc.type: FUNC
 */
HWTEST_F(TLVUtilsTest, Vector2PixelMapTest001, TestSize.Level1)
{
    std::vector<uint8_t> data;
    auto result = TLVUtils::Vector2PixelMap(data);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: PixelMap2VectorTest001
 * @tc.desc: test PixelMap2Vector with nullptr
 * @tc.type: FUNC
 */
HWTEST_F(TLVUtilsTest, PixelMap2VectorTest001, TestSize.Level1)
{
    auto result = TLVUtils::PixelMap2Vector(nullptr);
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name: RecursiveGuardTest001
 * @tc.desc: test RecursiveGuard within depth limit
 * @tc.type: FUNC
 */
HWTEST_F(TLVUtilsTest, RecursiveGuardTest001, TestSize.Level1)
{
    RecursiveGuard guard1;
    EXPECT_TRUE(guard1.IsValid());
}

/**
 * @tc.name: RecursiveGuardTest002
 * @tc.desc: test RecursiveGuard depth increment
 * @tc.type: FUNC
 */
HWTEST_F(TLVUtilsTest, RecursiveGuardTest002, TestSize.Level1)
{
    RecursiveGuard guard1;
    EXPECT_TRUE(guard1.IsValid());
    RecursiveGuard guard2;
    EXPECT_TRUE(guard2.IsValid());
    RecursiveGuard guard3;
    EXPECT_TRUE(guard3.IsValid());
}

/**
 * @tc.name: RecursiveGuardTest003
 * @tc.desc: test RecursiveGuard at max depth
 * @tc.type: FUNC
 */
HWTEST_F(TLVUtilsTest, RecursiveGuardTest003, TestSize.Level1)
{
    RecursiveGuard guards[10];
    for (int i = 0; i < 10; i++) {
        EXPECT_TRUE(guards[i].IsValid());
    }
}

/**
 * @tc.name: RawMemStructTest001
 * @tc.desc: test RawMem struct initialization
 * @tc.type: FUNC
 */
HWTEST_F(TLVUtilsTest, RawMemStructTest001, TestSize.Level1)
{
    RawMem rawMem;
    EXPECT_EQ(rawMem.buffer, 0);
    EXPECT_EQ(rawMem.bufferLen, 0);
    EXPECT_EQ(rawMem.parcel, nullptr);
}