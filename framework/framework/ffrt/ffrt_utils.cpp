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

#include "ffrt/ffrt_utils.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
void FFRTUtils::SubmitTask(const FFRTTask &task)
{
    ffrt::submit(task);
}

void FFRTUtils::SubmitQueueTasks(const std::vector<FFRTTask> &tasks, FFRTQueue &queue)
{
    if (tasks.empty()) {
        return;
    }
    for (const auto &task : tasks) {
        queue.submit(task);
    }
}

FFRTHandle FFRTUtils::SubmitDelayTask(FFRTTask &task, uint32_t delayMs, FFRTQueue &queue)
{
    using namespace std::chrono;
    milliseconds ms(delayMs);
    microseconds us = duration_cast<microseconds>(ms);
    return queue.submit_h(task, ffrt::task_attr().delay(us.count()));
}

FFRTHandle FFRTUtils::SubmitDelayTask(FFRTTask &task, uint32_t delayMs, std::shared_ptr<FFRTQueue> queue)
{
    using namespace std::chrono;
    milliseconds ms(delayMs);
    microseconds us = duration_cast<microseconds>(ms);
    return queue->submit_h(task, ffrt::task_attr().delay(us.count()));
}

bool FFRTUtils::SubmitTimeoutTask(const FFRTTask &task, uint32_t timeoutMs)
{
    ffrt::future<void> future = ffrt::async(task);
    auto status = future.wait_for(std::chrono::milliseconds(timeoutMs));
    return status == ffrt::future_status::ready;
}

int FFRTUtils::CancelTask(FFRTHandle &handle, FFRTQueue &queue)
{
    return queue.cancel(handle);
}

int FFRTUtils::CancelTask(FFRTHandle &handle, std::shared_ptr<FFRTQueue> &queue)
{
    return queue->cancel(handle);
}

FFRTTimer::FFRTTimer() : queue_("ffrt_timer") {}

FFRTTimer::FFRTTimer(const char *timer_name) : queue_(timer_name) {}

FFRTTimer::~FFRTTimer()
{
    Clear();
}

void FFRTTimer::Clear()
{
    mutex_.lock();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "FFRT Timer Clear");
    CancelAllTimerInner();
    handleMap_.clear();
    taskId_.clear();
    mutex_.unlock();
}

void FFRTTimer::CancelAllTimer()
{
    mutex_.lock();
    CancelAllTimerInner();
    mutex_.unlock();
}

void FFRTTimer::CancelTimer(const std::string &timerId)
{
    mutex_.lock();
    CancelTimerInner(timerId);
    mutex_.unlock();
}

void FFRTTimer::SetTimer(const std::string &timerId, FFRTTask &task, uint32_t delayMs)
{
    mutex_.lock();
    CancelTimerInner(timerId);
    ++taskId_[timerId];
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Timer[%{public}s] Add Task[%{public}u] with delay = %{public}u",
        timerId.c_str(), taskId_[timerId], delayMs);
    if (delayMs == 0) {
        FFRTUtils::SubmitTask(task);
    } else {
        handleMap_[timerId] = FFRTUtils::SubmitDelayTask(task, delayMs, queue_);
    }
    mutex_.unlock();
}

uint32_t FFRTTimer::GetTaskId(const std::string &timerId)
{
    mutex_.lock();
    uint32_t id = taskId_[timerId];
    mutex_.unlock();
    return id;
}

/* inner functions must be called when mutex_ is locked */
void FFRTTimer::CancelAllTimerInner()
{
    for (auto &p : handleMap_) {
        if (p.second != nullptr) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "Timer[%{public}s] Cancel Task[%{public}u]", p.first.c_str(),
                taskId_[p.first]);
            FFRTUtils::CancelTask(p.second, queue_);
        }
    }
}

void FFRTTimer::CancelTimerInner(const std::string &timerId)
{
    if (handleMap_[timerId] != nullptr) {
        PASTEBOARD_HILOGD(
            PASTEBOARD_MODULE_SERVICE, "Timer[%{public}s] Cancel Task[%{public}u]", timerId.c_str(), taskId_[timerId]);
        FFRTUtils::CancelTask(handleMap_[timerId], queue_);
        handleMap_[timerId] = nullptr;
    }
}
} // namespace MiscServices
} // namespace OHOS