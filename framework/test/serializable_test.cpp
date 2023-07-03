/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "serializable.h"

#include <gtest/gtest.h>

#include "config.h"
#include "pasteboard_hilog.h"
namespace OHOS::DistributedData {
using namespace testing::ext;
using namespace OHOS::MiscServices;
class SerializableTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SerializableTest::SetUpTestCase(void)
{
}

void SerializableTest::TearDownTestCase(void)
{
}

void SerializableTest::SetUp(void)
{
}

void SerializableTest::TearDown(void)
{
}

/**
* @tc.name: SerializableTest001
* @tc.desc: test serializable with invalid jsonStr .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest001, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string jsonStr;
    auto json = Config::ToJson(jsonStr);
    ASSERT_TRUE(json.is_null());

    Config config;
    auto ret = config.Unmarshall(jsonStr);
    ASSERT_FALSE(ret);

    jsonStr = "{ a }";
    json = Config::ToJson(jsonStr);
    ASSERT_TRUE(json.is_null());

    ret = config.Unmarshall(jsonStr);
    ASSERT_FALSE(ret);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest002
* @tc.desc: test serializable with valid jsonStr .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest002, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string jsonStr = R"({"key":"key1", "passwd":"*******"})";
    auto json = Config::ToJson(jsonStr);
    ASSERT_TRUE(!json.is_null());
    std::string value1;
    auto ret = Config::GetValue(json, "key", value1);
    ASSERT_TRUE(ret);
    ASSERT_EQ(value1, "key1");

    ret = Config::GetValue(json, "passwd", value1);
    ASSERT_TRUE(ret);
    ASSERT_EQ(value1, "*******");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest003
* @tc.desc: test serializable GetValue and SetValue with invalid string value .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest003, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string jsonStr = R"({"key":null})";
    auto json = Config::ToJson(jsonStr);
    std::string value;
    auto ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":1})";
    json = Config::ToJson(jsonStr);
    ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest004
* @tc.desc: test serializable GetValue and SetValue with invalid uint32_t value .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest004, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string jsonStr = R"({"key":null})";
    auto json = Config::ToJson(jsonStr);
    uint32_t value;
    auto ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":-1})";
    json = Config::ToJson(jsonStr);
    ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest005
* @tc.desc: test serializable GetValue and SetValue with invalid int32_t value .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest005, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string jsonStr = R"({"key":null})";
    auto json = Config::ToJson(jsonStr);
    int32_t value;
    auto ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":"1"})";
    json = Config::ToJson(jsonStr);
    ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest006
* @tc.desc: test serializable GetValue and SetValue with invalid int64_t value .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest006, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string jsonStr = R"({"key":null})";
    auto json = Config::ToJson(jsonStr);
    int64_t value;
    auto ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":"1"})";
    json = Config::ToJson(jsonStr);
    ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest007
* @tc.desc: test serializable GetValue and SetValue with invalid bool value .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest007, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string jsonStr = R"({"key":null})";
    auto json = Config::ToJson(jsonStr);
    bool value;
    auto ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":1})";
    json = Config::ToJson(jsonStr);
    ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest008
* @tc.desc: test serializable GetValue and SetValue with invalid std::vector<uint8_t> value .
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest008, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string jsonStr = R"({"key":null})";
    auto json = Config::ToJson(jsonStr);
    std::vector<uint8_t> value;
    auto ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":"{1, 2, 3}"})";
    json = Config::ToJson(jsonStr);
    ret = Config::GetValue(json, "key", value);
    ASSERT_FALSE(ret);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest009
* @tc.desc: test serializable GetValue and SetValue with valid sub node.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest009, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string param1 = "test";
    uint32_t param2 = 2;
    int32_t param3 = 3;
    int64_t param4 = 4;
    bool param5 = false;
    std::vector<uint8_t> param6 = { 1, 2, 3, 4, 5 };
    Serializable::json node;
    auto ret = Config::SetValue(node[GET_NAME(param1)], param1);
    ASSERT_TRUE(ret);
    ret = Config::SetValue(node[GET_NAME(param2)], param2);
    ASSERT_TRUE(ret);
    ret = Config::SetValue(node[GET_NAME(param3)], param3);
    ASSERT_TRUE(ret);
    ret = Config::SetValue(node[GET_NAME(param4)], param4);
    ASSERT_TRUE(ret);
    ret = Config::SetValue(node[GET_NAME(param5)], param5);
    ASSERT_TRUE(ret);
    ret = Config::SetValue(node[GET_NAME(param6)], param6);
    ASSERT_TRUE(ret);

    std::string res1;
    uint32_t res2;
    int32_t res3;
    int64_t res4;
    bool res5;
    std::vector<uint8_t> res6;
    ret = Config::GetValue(node, GET_NAME(param1), res1);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res1, param1);
    ret = Config::GetValue(node, GET_NAME(param2), res2);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res2, param2);
    ret = Config::GetValue(node, GET_NAME(param3), res3);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res3, param3);
    ret = Config::GetValue(node, GET_NAME(param4), res4);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res4, param4);
    ret = Config::GetValue(node, GET_NAME(param5), res5);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res5, param5);
    ret = Config::GetValue(node, GET_NAME(param6), res6);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res1.size(), param1.size());
    for (uint64_t i = 0; i < res1.size(); i++) {
        ASSERT_EQ(res1[i], param1[i]);
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}
} // namespace OHOS::DistributedData