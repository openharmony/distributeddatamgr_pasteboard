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
#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_FFRT_UTILS_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_FFRT_UTILS_H

#include <unordered_map>

#include "api/visibility.h"
#include "ffrt_inner.h"

namespace OHOS {
namespace MiscServices {
/**
 * Defines the task of the FFRT.
 */
using FFRTTask = std::function<void()>;

/**
 * Defines the task handle of the FFRT.
 */
using FFRTHandle = ffrt::task_handle;

/**
 * Defines the task queue of the FFRT.。
 */
using FFRTQueue = ffrt::queue;

/**
 * The mutex for FFRT tasks.
 */
using FFRTMutex = ffrt::mutex;

class API_EXPORT FFRTUtils final {
public:
    /**
 * Submit an FFRT atomization task without blocking the current thread.
 *
 * @param task FFRT task.
 */
    static void SubmitTask(const FFRTTask &task);

    /**
 * Submit an FFRT serial task without blocking the current thread.
 *
 * @param task FFRT task.
 */
    static void SubmitQueueTasks(const std::vector<FFRTTask> &tasks, FFRTQueue &queue);

    /**
 * Submit the FFRT delayed task without blocking the current thread.
 * <p>
 * When the delay time is reached, the task starts to be executed.
 *
 * @param task FFRT task.
 * @param delayMs Delay time, in milliseconds.
 * @param queue FFRT task execution queue.
 *
 * @return FFRT task handle.
 */
    static FFRTHandle SubmitDelayTask(FFRTTask &task, uint32_t delayMs, FFRTQueue &queue);

    /**
 * Submit the FFRT delayed task without blocking the current thread.
 * <p>
 * When the delay time is reached, the task starts to be executed.
 *
 * @param task FFRT task.
 * @param delayMs Delay time, in milliseconds.
 * @param queue Shared_ptr of FFRT task execution queue.
 *
 * @return FFRT task handle.
 */
    static FFRTHandle SubmitDelayTask(FFRTTask &task, uint32_t delayMs, std::shared_ptr<FFRTQueue> queue);

    /**
 * Submit an FFRT timeout task without blocking the current thread.
 * <p>
 * When the timeout period is reached, the task will be canceled.
 *
 * @param task FFRT task.
 * @param timeoutMs Timeout interval, in milliseconds.
 *
 * @return true: The task is executed successfully. false: The task execution times out.
 */
    static bool SubmitTimeoutTask(const FFRTTask &task, uint32_t timeoutMs);

    /**
 * Cancel the FFRT task.
 * <p>
 * You cannot cancel a completed task.
 *
 * @param handle FFRT task.
 */
    static int CancelTask(FFRTHandle &handle, FFRTQueue &queue);

    /**
 * Cancel the FFRT task.
 * <p>
 * You cannot cancel a completed task.
 *
 * @param handle FFRT task.
 * @param queue Shared_ptr of FFRT task cancel queue.
 */
    static int CancelTask(FFRTHandle &handle, std::shared_ptr<FFRTQueue> &queue);
};

class API_EXPORT FFRTTimer {
public:
    FFRTTimer();
    FFRTTimer(const std::string &timerName);
    ~FFRTTimer();
    void Clear();
    void CancelAllTimer();
    void CancelTimer(const std::string &timerId);
    void SetTimer(const std::string &timerId, FFRTTask &task, uint32_t delayMs = 0);
    uint32_t GetTaskId(const std::string &timerId);

private:
    /* inner functions must be called when mutex_ is locked */
    void CancelAllTimerInner();
    void CancelTimerInner(const std::string &timerId);

    FFRTMutex mutex_;
    FFRTQueue queue_;
    std::unordered_map<std::string, FFRTHandle> handleMap_;
    std::unordered_map<std::string, uint32_t> taskId_;
};

class API_EXPORT FFRTPool {
public:
   static std::shared_ptr<FFRTTimer> GetTimer(const std::string &name);
   static void Clear();

private:
   static std::unordered_map<std::string, std::shared_ptr<FFRTTimer>> ffrtPool_;
   static std::mutex mutex_;
};
} // namespace MiscServices
} // namespace OHOS

#endif //DISTRIBUTEDDATAMGR_PASTEBOARD_FFRT_UTILS_H
