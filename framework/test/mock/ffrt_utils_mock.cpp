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

#include "ffrt/ffrt_utils.h"

namespace OHOS {
namespace MiscServices {
void FFRTUtils::SubmitTask(const FFRTTask &task)
{
    (void)task;
}

void FFRTUtils::SubmitQueueTasks(const std::vector<FFRTTask> &tasks, FFRTQueue &queue)
{
    (void)tasks;
    (void)queue;
}

FFRTHandle FFRTUtils::SubmitDelayTask(FFRTTask &task, uint32_t delayMs, FFRTQueue &queue)
{
    (void)task;
    (void)delayMs;
    (void)queue;
    return {};
}

FFRTHandle FFRTUtils::SubmitDelayTask(FFRTTask &task, uint32_t delayMs, std::shared_ptr<FFRTQueue> queue)
{
    (void)task;
    (void)delayMs;
    (void)queue;
    return {};
}

bool FFRTUtils::SubmitTimeoutTask(const FFRTTask &task, uint32_t timeoutMs)
{
    (void)task;
    (void)timeoutMs;
    return false;
}

int FFRTUtils::CancelTask(FFRTHandle &handle, FFRTQueue &queue)
{
    (void)handle;
    (void)queue;
    return 0;
}

int FFRTUtils::CancelTask(FFRTHandle &handle, std::shared_ptr<FFRTQueue> &queue)
{
    (void)handle;
    (void)queue;
    return 0;
}

FFRTTimer::FFRTTimer() : queue_("ffrt_timer") {}

FFRTTimer::FFRTTimer(const char *timer_name) : queue_(timer_name) {}

FFRTTimer::~FFRTTimer()
{
}

void FFRTTimer::Clear()
{
}

void FFRTTimer::CancelAllTimer()
{
}

void FFRTTimer::CancelTimer(const std::string &timerId)
{
    (void)timerId;
}

void FFRTTimer::SetTimer(const std::string &timerId, FFRTTask &task, uint32_t delayMs)
{
    (void)timerId;
    (void)task;
    (void)delayMs;
}

uint32_t FFRTTimer::GetTaskId(const std::string &timerId)
{
    (void)timerId;
    return 0;
}

void FFRTTimer::CancelAllTimerInner()
{
}

void FFRTTimer::CancelTimerInner(const std::string &timerId)
{
    (void)timerId;
}
} // namespace MiscServices
} // namespace OHOS