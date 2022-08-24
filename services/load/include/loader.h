/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_PASTEBOARD_SERVICES_LOAD_LOADER_H
#define OHOS_PASTEBOARD_SERVICES_LOAD_LOADER_H
#include "config.h"
namespace OHOS::MiscServices {
class Loader {
public:
    Loader();
    ~Loader();
    void LoadComponents();

private:
    using Constructor = void (*)(const char *);
    Config LoadConfig();
    static constexpr const char *CONF_FILE = "/system/etc/pasteboard/conf/pasteboard.json";
};
} // namespace OHOS::MiscServices
#endif // OHOS_PASTEBOARD_SERVICES_LOAD_LOADER_H
