/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "cJSON.h"
#include "clip/clip_plugin.h"
#include "serializable/serializable.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
class ClipPluginTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ClipPluginTest::SetUpTestCase(void) { }

void ClipPluginTest::TearDownTestCase(void) { }

void ClipPluginTest::SetUp(void) { }

void ClipPluginTest::TearDown(void) { }

class CustomClipPlugin : public ClipPlugin {
public:
    int32_t SetPasteData(const GlobalEvent &event, const std::vector<uint8_t> &data) override
    {
        (void)event;
        (void)data;
        return 0;
    }

    std::pair<int32_t, int32_t> GetPasteData(const GlobalEvent &event, std::vector<uint8_t> &data) override
    {
        (void)event;
        (void)data;
        return std::make_pair(0, 0);
    }
};

/**
 * @tc.name: MarshalTest001
 * @tc.desc: Marshal.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(ClipPluginTest, MarshalTest001, TestSize.Level0)
{
    std::string PLUGIN_NAME_VAL = "distributed_clip";
    auto release = [&PLUGIN_NAME_VAL, this](ClipPlugin *plugin) {
        ClipPlugin::DestroyPlugin(PLUGIN_NAME_VAL, plugin);
    };
    std::shared_ptr<ClipPlugin> clipPlugin;
    clipPlugin = std::shared_ptr<ClipPlugin>(ClipPlugin::CreatePlugin(PLUGIN_NAME_VAL), release);
    DistributedData::Serializable::json node;
    ClipPlugin::GlobalEvent globalEvent;
    std::string networkId = "networkId";
    int32_t result = clipPlugin->PublishServiceState(networkId, ClipPlugin::ServiceStatus::CONNECT_SUCC);
    ASSERT_TRUE(globalEvent.Marshal(node));
}

/**
 * @tc.name: UnmarshalTest001
 * @tc.desc: Unmarshal.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(ClipPluginTest, UnmarshalTest001, TestSize.Level0)
{
    std::string str = R"({
        {"version", 1},
        {"frameNum", 100},
        {"user", "testUser"},
        {"seqId", 12345},
        {"expiration", 1000},
        {"status", "active"},
        {"deviceId", "123456"},
        {"account", "testAcount"},
        {"dataType", "image"},
        {"syncTime", 2000},
    })";
    DistributedData::Serializable::json node = cJSON_Parse(str.c_str());
    ClipPlugin::GlobalEvent globalEvent;
    ASSERT_FALSE(globalEvent.Unmarshal(node));
    cJSON_Delete(node);
}

/**
 * @tc.name: PublishServiceStateTest
 * @tc.desc: PublishServiceState.
 * @tc.type: FUNC
 * @tc.require:
 * @tc.author:
 */
HWTEST_F(ClipPluginTest, PublishServiceStateTest, TestSize.Level0)
{
    CustomClipPlugin clipPlugin;
    std::string networkId = "testNetworkId";
    int32_t result = clipPlugin.PublishServiceState(networkId, ClipPlugin::ServiceStatus::CONNECT_SUCC);
    ASSERT_EQ(0, result);
}
} // namespace OHOS::MiscServices
