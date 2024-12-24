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

#ifndef PASTEBOARD_PROGRESS_SIGNAL_H
#define PASTEBOARD_PROGRESS_SIGNAL_H

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
    static ProgressSignalClient &GetInstance();
    void Cancel();
    bool IsCanceled();
    bool CheckCancelIfNeed();
    void MarkRemoteTask();
    void Init();
private:
    ProgressSignalClient() = default;
    ~ProgressSignalClient() = default;
    DISALLOW_COPY_AND_MOVE(ProgressSignalClient);
    std::atomic_bool needCancel_{ false };
    std::atomic_bool remoteTask_{ false };
    static std::mutex mutex_;
    static ProgressSignalClient *instance_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_PROGRESS_SIGNAL_H