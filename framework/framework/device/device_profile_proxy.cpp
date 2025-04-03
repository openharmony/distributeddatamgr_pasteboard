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

#include "device_profile_proxy.h"

#include <dlfcn.h>
#include <thread>

#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
constexpr const char *LIB_NAME = "libpasteboard_adapter.z.so";
constexpr const char *FUN_NAME_GET = "GetDeviceProfileAdapter";
constexpr const char *FUN_NAME_DEINIT = "DeinitDeviceProfileAdapter";

DeviceProfileProxy::DeviceProfileProxy()
{
    handler = dlopen(LIB_NAME, RTLD_NOW);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(handler != nullptr, PASTEBOARD_MODULE_COMMON,
        "dlopen failed, msg=%{public}s", dlerror());
}

DeviceProfileProxy::~DeviceProfileProxy()
{
    if (handler == nullptr) {
        return;
    }

    auto func = reinterpret_cast<DeinitDeviceProfileAdapterFunc>(dlsym(handler, FUN_NAME_DEINIT));
    if (func != nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "deinit adapter");
        func();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } else {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "dlsym failed, msg=%{public}s", dlerror());
    }

    dlclose(handler);
    handler = nullptr;
}

IDeviceProfileAdapter *DeviceProfileProxy::GetAdapter()
{
    auto func = reinterpret_cast<GetDeviceProfileAdapterFunc>(dlsym(handler, FUN_NAME_GET));
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(func != nullptr, nullptr, PASTEBOARD_MODULE_COMMON,
        "dlsym failed, msg=%{public}s", dlerror());
    return func();
}

} // namespace MiscServices
} // namespace OHOS
