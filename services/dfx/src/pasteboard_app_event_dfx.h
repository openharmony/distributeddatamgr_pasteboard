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

#ifndef MISCSERVICES_PASTEBOARD_DFX_PROCESSOR_EVENT_H
#define MISCSERVICES_PASTEBOARD_DFX_PROCESSOR_EVENT_H
#include <mutex>

namespace OHOS {
namespace MiscServices {
enum EventReportResult : std::int32_t {
    EVENT_REPORT_SUCCESS = 0,
    EVENT_REPORT_FAIL = 1,
};

class DfxAppEvent {
public:
    DfxAppEvent();
    ~DfxAppEvent();
    static int64_t AddProcessor();
    void SetEvent(const std::string &apiName, int32_t errCode, int32_t result);
private:
    static int64_t processorId_;
    static std::mutex mutex_;
    int32_t result_ = 1;
    int32_t errCode_ = -1;
    int64_t beginTime_ = 0;
    std::string apiName_;
    std::string transId_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // MISCSERVICES_PASTEBOARD_DFX_PROCESSOR_EVENT_H