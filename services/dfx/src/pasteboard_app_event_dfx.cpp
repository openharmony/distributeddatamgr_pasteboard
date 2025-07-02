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

#include "pasteboard_app_event_dfx.h"

#include "app_event.h"
#include "app_event_processor_mgr.h"

namespace OHOS {
namespace MiscServices {
constexpr int NOT_APP_PROCESSORID = -200;
constexpr const char *HIAPP_SDK_NAME = "BasicServicesKit";
int64_t  DfxAppEvent::processorId_ = -1;
std::mutex DfxAppEvent::mutex_;
DfxAppEvent::DfxAppEvent()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (DfxAppEvent::processorId_ == -1) {
        DfxAppEvent::processorId_ = DfxAppEvent::AddProcessor();
    }
    beginTime_ = static_cast<int64_t>(time(nullptr));
}

DfxAppEvent::~DfxAppEvent()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (DfxAppEvent::processorId_ == NOT_APP_PROCESSORID) {
        return;
    }
    OHOS::HiviewDFX::HiAppEvent::Event event("api_diagnostic", "api_exec_end", OHOS::HiviewDFX::HiAppEvent::BEHAVIOR);
    event.AddParam("trans_id", transId_);
    event.AddParam("api_name", apiName_);
    event.AddParam("sdk_name", string(HIAPP_SDK_NAME));
    event.AddParam("begin_time", beginTime_);
    event.AddParam("end_time", static_cast<int64_t>(time(nullptr)));
    event.AddParam("result", result_);
    event.AddParam("error_code", errCode_);
    Write(event);
}

int64_t DfxAppEvent::AddProcessor()
{
    HiviewDFX::HiAppEvent::ReportConfig config;
    config.name = "ha_app_event";
    config.configName = "SDK_OCG";
    return HiviewDFX::HiAppEvent::AppEventProcessorMgr::AddProcessor(config);
}

void DfxAppEvent::SetEvent(const std::string &apiName, int32_t errCode, int32_t result)
{
    result_ = result;
    apiName_ = apiName;
    errCode_ = errCode;
    transId_ = std::string("transId_") + std::to_string(std::rand());
}
} // namespace MiscServices
} // namespace OHOS