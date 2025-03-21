/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <dlfcn.h>

#include "pasteboard_hilog.h"
#include "pasteboard_lib_guard.h"

namespace OHOS::MiscServices {
LibGuard::LibGuard(const char *libPath)
{
    if (libPath == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "lib path is null.");
        return;
    }
    libPath_.assign(libPath);
    libHandle = dlopen(libPath, RTLD_LAZY);
    if (libHandle == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "dlopen %{public}s failed.", libPath);
    }
}

LibGuard::~LibGuard()
{
    if (libHandle == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "lib handle is null.");
        return;
    }
    if (dlclose(libHandle) == 0) {
        return;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "dlclose %{public}s error: %{public}s", libPath_.c_str(), dlerror());
}

bool LibGuard::Ready()
{
    return libHandle ? true : false;
}

void *LibGuard::GetLibHandle() const
{
    return libHandle;
}
} // namespace OHOS::MiscServices