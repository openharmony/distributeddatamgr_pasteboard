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

#include "eventcenter/pasteboard_event.h"

namespace OHOS::MiscServices {
using namespace testing::ext;
namespace {
    const std::string NETWORK_ID = "networkId";
}

class PasteboardEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void PasteboardEventTest::SetUpTestCase() {}

void PasteboardEventTest::TearDownTestCase() {}

void PasteboardEventTest::SetUp() {}

void PasteboardEventTest::TearDown() {}

/**
 * @tc.name: GetNetworkId_001
 * @tc.desc: Verify that GetNetworkId returns the correct network ID.
 * @tc.type: FUNC
 */
HWTEST_F(PasteboardEventTest, GetNetworkId_001, TestSize.Level1)
{
    int32_t evtId = 0;
    PasteboardEvent pasteboardEvent(evtId, NETWORK_ID);
    EXPECT_EQ(pasteboardEvent.GetNetworkId(), NETWORK_ID);
}
} // namespace OHOS::MiscServices
