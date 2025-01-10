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
#include <cstdio>
#include <chrono>
#include <map>

#include "distributed_file_daemon_manager.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_progress_signal.h"

namespace OHOS {
namespace MiscServices {
std::mutex ProgressSignalClient::mutex_;
ProgressSignalClient *ProgressSignalClient::instance_ = nullptr;

ProgressSignalClient &ProgressSignalClient::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        instance_ = new ProgressSignalClient();
    }
    return *instance_;
}

void ProgressSignalClient::Init()
{
    instance_ = nullptr;
    needCancel_.store(false);
    remoteTask_.store(false);
    sessionName_.clear();
    filePath_.clear();
}

void ProgressSignalClient::SetFilePathOfRemoteTask(const std::string &sessionName, const std::string &filePath)
{
    sessionName_ = sessionName;
    filePath_ = filePath;
}

void ProgressSignalClient::Cancel()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "ProgressSignalClient Cancel in!");
    if (!sessionName_.empty()) {
        auto ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().CancelCopyTask(sessionName_);
        if (ret != 0) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "CancelCopyTask, ret = %{public}d", ret);
            return;
        }
        std::remove(filePath_.c_str());
        sessionName_.clear();
        filePath_.clear();
    }
    needCancel_.store(true);
}

bool ProgressSignalClient::IsCanceled()
{
    return needCancel_.load() || remoteTask_.load();
}

bool ProgressSignalClient::CheckCancelIfNeed()
{
    if (!needCancel_.load()) {
        return false;
    }
    return true;
}

void ProgressSignalClient::MarkRemoteTask()
{
    remoteTask_.store(true);
}
} // namespace MiscServices
} // namespace OHOS