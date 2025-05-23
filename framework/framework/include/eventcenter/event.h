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

#ifndef OHOS_DISTRIBUTED_DATA_PASTEBOARD_SERVICES_FRAMEWORK_EVENTCENTER_EVENT_H
#define OHOS_DISTRIBUTED_DATA_PASTEBOARD_SERVICES_FRAMEWORK_EVENTCENTER_EVENT_H
#include <memory>

#include "api/visibility.h"
namespace OHOS::MiscServices {
class Event {
public:
    enum EventId : int32_t {
        EVT_INVALID,
        EVT_INIT,
        EVT_UPDATE,
        EVT_REMOTE_CHANGE = 0x1000,
    };
    API_EXPORT Event(int32_t evtId);
    API_EXPORT Event(Event &&) noexcept = delete;
    API_EXPORT Event(const Event &) = delete;
    API_EXPORT Event &operator=(Event &&) noexcept = delete;
    API_EXPORT Event &operator=(const Event &) = delete;
    API_EXPORT virtual ~Event();
    API_EXPORT virtual bool Equals(const Event &) const;
    API_EXPORT int32_t GetEventId() const;

private:
    int32_t evtId_;
};
} // namespace OHOS::MiscServices
#endif // OHOS_DISTRIBUTED_DATA_PASTEBOARD_SERVICES_FRAMEWORK_EVENTCENTER_EVENT_H
