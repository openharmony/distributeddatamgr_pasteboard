/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef PASTE_BOARD_PARA_HANDLE_H
#define PASTE_BOARD_PARA_HANDLE_H
#include <string>
namespace OHOS {
namespace MiscServices {
class ParaHandle {
public:
    ParaHandle() = default;
    virtual ~ParaHandle() = default;
    static void SetDpbEnable();
    static std::string GetDpbEnable();
    static void SubscribeDpbEnable();
    static void UnSubscribeDpbEnable();
    static void ParameterChange(const char *key, const char *value, void *context);

private:
    static const char *DISTRIBUTED_PASTEBOARD_ENABLE;
    static const char *DEFAULT_VALUE;
    static constexpr int CONFIG_LEN = 10;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_PARA_HANDLE_H
