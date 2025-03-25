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

#ifndef PASTEBOARD_DEV_PROFILE_MOCK_TEST_H
#define PASTEBOARD_DEV_PROFILE_MOCK_TEST_H

#include "device_manager_mock.h"
#include "distributed_device_profile_client_mock.h"
#include <gtest/gtest.h>

namespace OHOS {
namespace MiscServices {

using namespace testing::ext;

class DevProfileMockTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static inline std::shared_ptr<DistributedDeviceProfile::DistributedDeviceProfileClientMock>
        distributedDeviceProfileClientMock_ =
            std::make_shared<DistributedDeviceProfile::DistributedDeviceProfileClientMock>();
    static inline std::shared_ptr<DistributedHardware::DeviceManagerMock> deviceManagerMock_ =
        std::make_shared<DistributedHardware::DeviceManagerMock>();
};

} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_DEV_PROFILE_MOCK_TEST_H