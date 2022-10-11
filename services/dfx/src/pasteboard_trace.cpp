/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd.
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

#define LOG_TAG "MiscPasteboardDfx"

#include "pasteboard_trace.h"

#include <atomic>
#include <cstdint>

#include "hitrace/trace.h"
#include "hitrace_meter.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
static constexpr uint64_t BYTRACE_LABEL = HITRACE_TAG_MISC;
using OHOS::HiviewDFX::HiTraceChain;
using namespace MiscServices;

std::atomic_bool PasteboardTrace::isSetBytraceEnabled_ = false;

PasteboardTrace::PasteboardTrace(const std::string& value)
{
    SetBytraceEnable();
    StartTrace(BYTRACE_LABEL, value);
}

PasteboardTrace::~PasteboardTrace()
{
    FinishTrace(BYTRACE_LABEL);
}

bool PasteboardTrace::SetBytraceEnable() const
{
    if (isSetBytraceEnabled_.exchange(true)) {
        return true;
    }
    UpdateTraceLabel();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "success, current tag is true");
    return true;
}
} // namespace MiscServices
} // namespace OHOS
