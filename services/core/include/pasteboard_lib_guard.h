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

#ifndef PASTE_BOARD_LIB_GUARD_H
#define PASTE_BOARD_LIB_GUARD_H
#include <string>

namespace OHOS::MiscServices {
class LibGuard {
public:
    LibGuard(const char *libPath);
    ~LibGuard();
    bool Ready();
    void *GetLibHandle() const;

private:
    void *libHandle = nullptr;
    std::string libPath_{};
};
} // namespace OHOS::MiscServices
#endif // PASTE_BOARD_LIB_GUARD_H