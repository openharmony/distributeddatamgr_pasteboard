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

#include "eventcenter/event_center.h"
#include "pasteboard_hilog.h"

namespace OHOS::MiscServices {
thread_local EventCenter::AsyncQueue *EventCenter::asyncQueue_ = nullptr;
constexpr int32_t EventCenter::AsyncQueue::MAX_CAPABILITY;
EventCenter &EventCenter::GetInstance()
{
    static EventCenter eventCenter;
    return eventCenter;
}

bool EventCenter::Subscribe(int32_t evtId, const std::function<void(const Event &)> &observer)
{
    return observers_.Compute(evtId, [&observer](const auto &id, auto &list) -> bool {
        list.push_back(observer);
        return true;
    });
}

bool EventCenter::Unsubscribe(int32_t evtId)
{
    return observers_.Erase(evtId);
}

int32_t EventCenter::PostEvent(std::unique_ptr<Event> evt) const
{
    if (evt == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "evt is null");
        return CODE_INVALID_ARGS;
    }
    if (asyncQueue_ == nullptr) {
        Dispatch(*evt);
        return CODE_SYNC;
    }
    asyncQueue_->Post(std::move(evt));
    return CODE_ASYNC;
}

void EventCenter::Dispatch(const Event &evt) const
{
    auto observers = observers_.Find(evt.GetEventId());
    if (!observers.first) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "event not find, id=%{public}d", evt.GetEventId());
        return;
    }

    for (const auto &observer : observers.second) {
        observer(evt);
    }
}

EventCenter::Defer::Defer(std::function<void(const Event &)> handler, int32_t evtId)
{
    if (asyncQueue_ == nullptr) {
        asyncQueue_ = new (std::nothrow) AsyncQueue();
    }

    if (asyncQueue_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "asyncQueue_ is null");
        return;
    }

    ++(*asyncQueue_);
    asyncQueue_->AddHandler(evtId, std::move(handler));
}

EventCenter::Defer::~Defer()
{
    if (asyncQueue_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "asyncQueue_ is null");
        return;
    }
    --(*asyncQueue_);
    if ((*asyncQueue_) <= 0) {
        delete asyncQueue_;
        asyncQueue_ = nullptr;
    }
}

EventCenter::AsyncQueue &EventCenter::AsyncQueue::operator++()
{
    ++depth_;
    return *this;
}

EventCenter::AsyncQueue &EventCenter::AsyncQueue::operator--()
{
    --depth_;
    if (depth_ > 0) {
        return *this;
    }
    depth_ = 1;
    for (int32_t count = 0; !events_.empty() && count < MAX_CAPABILITY; count++) {
        auto &evt = events_.front();
        // dispatch to resident handlers
        GetInstance().Dispatch(*evt);

        // dispatch to temporary handlers
        auto handler = handlers_.find(evt->GetEventId());
        if (handler != handlers_.end()) {
            handler->second(*evt);
        }
        events_.pop_front();
    }
    depth_ = 0;
    return *this;
}

bool EventCenter::AsyncQueue::operator<=(int32_t depth) const
{
    return depth_ <= depth;
}

void EventCenter::AsyncQueue::Post(std::unique_ptr<Event> evt)
{
    for (auto &event : events_) {
        if (event->GetEventId() != evt->GetEventId()) {
            continue;
        }

        if (event->Equals(*evt)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "event not equal");
            return;
        }
    }
    events_.push_back(std::move(evt));
}

void EventCenter::AsyncQueue::AddHandler(int32_t evtId, std::function<void(const Event &)> handler)
{
    if (evtId == Event::EVT_INVALID || handler == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE,
            "invalid param, evtId=%{public}d, handler is null=%{public}d", evtId, (handler == nullptr));
        return;
    }

    // The topper layer event will be effective
    if (handlers_.find(evtId) != handlers_.end()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "handler already exist, evtId=%{public}d", evtId);
        return;
    }

    handlers_[evtId] = std::move(handler);
}
} // namespace OHOS::MiscServices