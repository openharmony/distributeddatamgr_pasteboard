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

#ifndef OHOS_DISTRIBUTED_DATA_PASTEBOARD_SERVICES_FRAMEWORK_EVENTCENTER_DISTRIBUTED_PASTEBOARD_EVENT_H
#define OHOS_DISTRIBUTED_DATA_PASTEBOARD_SERVICES_FRAMEWORK_EVENTCENTER_DISTRIBUTED_PASTEBOARD_EVENT_H

#include <string>

#include "eventcenter/event.h"

namespace OHOS::MiscServices {
class API_EXPORT PasteboardEvent : public Event {
public:
    enum EventId : int32_t {
        REMOTE_CHANGE = EVT_REMOTE_CHANGE,
        DISCONNECT,
    };

    PasteboardEvent(int32_t evtId, std::string networkId);
    ~PasteboardEvent() = default;
    std::string GetNetworkId() const;

private:
    std::string networkId_;
};
} // namespace OHOS::MiscServices
#endif // OHOS_DISTRIBUTED_DATA_PASTEBOARD_SERVICES_FRAMEWORK_EVENTCENTER_DISTRIBUTED_PASTEBOARD_EVENT_H