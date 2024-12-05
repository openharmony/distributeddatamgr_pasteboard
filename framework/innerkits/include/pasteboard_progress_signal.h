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

#ifndef PASTEBOARD_TASK_SIGNAL_H
#define PASTEBOARD_TASK_SIGNAL_H

#include <condition_variable>
#include <memory>
#include <functional>
#include <singleton.h>
#include <string>
#include <unistd.h>
#include "visibility.h"

namespace OHOS {
namespace MiscServices {
class API_EXPORT ProgressSignalClient {
public:
    ProgressSignalClient() {};
    ~ProgressSignalClient() {};
    static ProgressSignalClient &GetInstance();
    int32_t Cancel();
    bool IsCanceled();
    bool CheckCancelIfNeed(const std::string &path);
    void MarkRemoteTask();
    void SetFileInfoOfRemoteTask(const std::string &sessionName, const std::string &filePath);
    std::atomic_bool needCancel_{ false };
    std::atomic_bool remoteTask_{ false };
    std::string sessionName_ = std::string("");
    std::string filePath_ = std::string("");

private:
    static void CancelCopyTask();
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_TASK_SIGNAL_H