/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <chrono>
#include <map>

#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_progress_signal.h"

namespace OHOS {
namespace MiscServices {
ProgressSignalClient &ProgressSignalClient::GetInstance()
{
    static ProgressSignalClient instance;
    return instance;
}

void ProgressSignalClient::CancelCopyTask()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "CancelCopyTask in!");
}

int32_t ProgressSignalClient::Cancel()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "ProgressSignalClient Cancel in!");
    if (remoteTask_.load()) {
        if (sessionName_.empty()) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "sessionName_ is nullptr!");
            return static_cast<int32_t>(PasteboardError::GET_REMOTE_DATA_ERROR);
        }
        CancelCopyTask();
        return static_cast<int32_t>(PasteboardError::E_OK);
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "ProgressSignalClient Cancel end!");
    needCancel_.store(true);
    return static_cast<int32_t>(PasteboardError::E_OK);
}

bool ProgressSignalClient::IsCanceled()
{
    return needCancel_.load() || remoteTask_.load();
}

void ProgressSignalClient::MarkRemoteTask()
{
    remoteTask_.store(true);
}

void ProgressSignalClient::SetFileInfoOfRemoteTask(const std::string &sessionName, const std::string &filePath)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "SetFileInfoOfRemoteTask sessionName=%{public}s", sessionName.c_str());
    sessionName_ = sessionName;
    filePath_ = filePath;
}
} // namespace MiscServices
} // namespace OHOS