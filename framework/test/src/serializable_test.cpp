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
#include "config.h"
#include "pasteboard_hilog.h"
#include "serializable.h"
#include <gtest/gtest.h>

namespace OHOS::DistributedData {
using namespace testing::ext;
using namespace OHOS::MiscServices;
class SerializableTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void CreateConfig(Config &config);
    void CreateComponent(Config::Component &component, int32_t index);
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

void SerializableTest::CreateComponent(Config::Component &component, int32_t index)
{
    component.description = "description" + std::to_string(index);
    component.lib = "lib" + std::to_string(index);
    component.constructor = "constructor" + std::to_string(index);
    component.destructor = "destructor" + std::to_string(index);
    component.params = "params" + std::to_string(index);
}

void SerializableTest::CreateConfig(Config &config)
{
    config.processLabel = "processLabel";
    config.version = "version";
    config.features = { "feature1", "feature2" };
    config.plugins = { "plugin1", "plugin2" };
    Config::Component component1;
    Config::Component component2;
    int32_t index = 0;
    CreateComponent(component1, index++);
    CreateComponent(component2, index++);
    config.components = { component1, component2 };
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
    auto json = Serializable::ToJson(jsonStr);
    ASSERT_TRUE(cJSON_IsNull(&json));

    Config config;
    auto ret = config.Unmarshall(jsonStr);
    ASSERT_FALSE(ret);

    jsonStr = "{ a }";
    json = Serializable::ToJson(jsonStr);
    ASSERT_TRUE(cJSON_IsNull(&json));

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

    std::vector<uint8_t> vec = { 1, 2, 3, 4, 5 };
    std::string jsonStr = "{\n"
                          "        \"param1\":       2,                                \n"
                          "        \"param2\":       3,                                \n"
                          "        \"param3\":       4,                                \n"
                          "        \"param4\":       true,                             \n"
                          "        \"param5\":       [1, 2, 3, 4, 5]                   \n"
                          "}";

    uint32_t res1;
    int32_t res2;
    int64_t res3;
    bool res4;
    std::vector<uint8_t> res5;

    auto node = Serializable::ToJson(jsonStr);

    auto ret = Serializable::GetValue(node, GET_NAME(param1), res1);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res1, 2);
    ret = Serializable::GetValue(node, GET_NAME(param2), res2);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res2, 3);
    ret = Serializable::GetValue(node, GET_NAME(param3), res3);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res3, 4);
    ret = Serializable::GetValue(node, GET_NAME(param4), res4);
    ASSERT_TRUE(ret);
    ASSERT_TRUE(res4);
    ret = Serializable::GetValue(node, GET_NAME(param5), res5);
    ASSERT_TRUE(ret);
    ASSERT_EQ(res5.size(), vec.size());
    for (uint64_t i = 0; i < res5.size(); i++) {
        ASSERT_EQ(res5[i], vec[i]);
    }
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
    auto json = Serializable::ToJson(jsonStr);
    std::string value;
    auto ret = Serializable::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":1})";
    json = Serializable::ToJson(jsonStr);
    ret = Serializable::GetValue(json, "key", value);
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
    auto ret = Serializable::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":"1"})";
    json = Serializable::ToJson(jsonStr);
    ret = Serializable::GetValue(json, "key", value);
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
    auto ret = Serializable::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":"1"})";
    json = Serializable::ToJson(jsonStr);
    ret = Serializable::GetValue(json, "key", value);
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
    auto json = Serializable::ToJson(jsonStr);
    int64_t value;
    auto ret = Serializable::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":"1"})";
    json = Serializable::ToJson(jsonStr);
    ret = Serializable::GetValue(json, "key", value);
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
    auto json = Serializable::ToJson(jsonStr);
    bool value;
    auto ret = Serializable::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":1})";
    json = Serializable::ToJson(jsonStr);
    ret = Serializable::GetValue(json, "key", value);
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
    auto json = Serializable::ToJson(jsonStr);
    std::vector<uint8_t> value;
    auto ret = Serializable::GetValue(json, "key", value);
    ASSERT_FALSE(ret);

    jsonStr = R"({"key":"{1, 2, 3}"})";
    json = Serializable::ToJson(jsonStr);
    ret = Serializable::GetValue(json, "key", value);
    ASSERT_FALSE(ret);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest009
* @tc.desc: test serializable SetValue with valid value.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest009, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    uint32_t param1 = 2;
    int32_t param2 = 3;
    int64_t param3 = 4;
    bool param4 = false;
    std::vector<uint8_t> param5 = { 1, 2, 3, 4, 5 };
    Config config;
    CreateConfig(config);

    auto node = config.Marshall();
    ASSERT_FALSE(cJSON_IsNull(&node));

    auto ret = Serializable::SetValue(node, param1, GET_NAME(param1));
    ASSERT_TRUE(ret);
    ret = Serializable::SetValue(node, param2, GET_NAME(param2));
    ASSERT_TRUE(ret);
    ret = Serializable::SetValue(node, param3, GET_NAME(param3));
    ASSERT_TRUE(ret);
    ret = Serializable::SetValue(node, param4, GET_NAME(param4));
    ASSERT_TRUE(ret);
    ret = Serializable::SetValue(node, param5, GET_NAME(param5));
    ASSERT_TRUE(ret);
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(processLabel)));
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(version)));
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(features)));
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(plugins)));
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(components)));
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(param1)));
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(param2)));
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(param3)));
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(param4)));
    ASSERT_TRUE(cJSON_HasObjectItem(&node, GET_NAME(param5)));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}

/**
* @tc.name: SerializableTest010
* @tc.desc: test serializable Unmarshall with valid jsonstr.
* @tc.type: FUNC
* @tc.require:
* @tc.author:
*/
HWTEST_F(SerializableTest, SerializableTest010, TestSize.Level0)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "Start.");
    std::string jsonStr = "{\n"
                          "        \"processLabel\": \"processLabel\",                \n"
                          "        \"version\":      \"version\",                     \n"
                          "        \"features\":     [\"features1\", \"features2\"], \n"
                          "        \"plugins\":      [\"plugins1\", \"plugins2\"],   \n"
                          "        \"components\":   [{                                \n"
                          "                        \"description\":  \"description1\",\n"
                          "                        \"lib\":  \"lib1\",                \n"
                          "                        \"constructor\":  \"constructor1\",\n"
                          "                        \"destructor\":   \"destructor1\", \n"
                          "                        \"params\":       \"params1\"      \n"
                          "                }, {                                      \n"
                          "                        \"description\":  \"description2\",\n"
                          "                        \"lib\":  \"lib2\",                \n"
                          "                        \"constructor\":  \"constructor2\",\n"
                          "                        \"destructor\":   \"destructor2\", \n"
                          "                        \"params\":       \"params2\"      \n"
                          "                }]                                      \n"
                          "}";

    Config config;
    auto ret = config.Unmarshall(jsonStr);
    ASSERT_TRUE(ret);
    ASSERT_EQ(config.processLabel, "processLabel");
    ASSERT_EQ(config.version, "version");
    ASSERT_EQ(config.features[0], "features1");
    ASSERT_EQ(config.features[1], "features2");
    ASSERT_EQ(config.plugins[0], "plugins1");
    ASSERT_EQ(config.plugins[1], "plugins2");
    ASSERT_EQ(config.components[0].description, "description1");
    ASSERT_EQ(config.components[0].lib, "lib1");
    ASSERT_EQ(config.components[0].constructor, "constructor1");
    ASSERT_EQ(config.components[0].destructor, "destructor1");
    ASSERT_EQ(config.components[0].params, "params1");
    ASSERT_EQ(config.components[1].description, "description2");
    ASSERT_EQ(config.components[1].lib, "lib2");
    ASSERT_EQ(config.components[1].constructor, "constructor2");
    ASSERT_EQ(config.components[1].destructor, "destructor2");
    ASSERT_EQ(config.components[1].params, "params2");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_INNERKIT, "End.");
}
} // namespace OHOS::DistributedData