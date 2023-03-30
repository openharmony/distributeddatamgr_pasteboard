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

#ifndef MISCSERVICES_PASTEBOARD_TRACE_H
#define MISCSERVICES_PASTEBOARD_TRACE_H

#include <atomic>
#include <string>

namespace OHOS {
namespace MiscServices {
class PasteboardTrace {
public:
    explicit PasteboardTrace(const std::string &value);
    ~PasteboardTrace();

private:
    bool SetBytraceEnable() const;
    static std::atomic_bool isSetBytraceEnabled_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // MISCSERVICES_PASTEBOARD_TRACE_H
