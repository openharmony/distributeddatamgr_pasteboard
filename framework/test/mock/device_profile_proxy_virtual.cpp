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

#include "device/device_profile_proxy.h"
#include "device_profile_adapter.h"

namespace OHOS {
namespace MiscServices {
DeviceProfileProxy::DeviceProfileProxy()
{
}

DeviceProfileProxy::~DeviceProfileProxy()
{
}

IDeviceProfileAdapter *DeviceProfileProxy::GetAdapter()
{
    return GetDeviceProfileAdapter();
}
} // namespace MiscServices
} // namespace OHOS
