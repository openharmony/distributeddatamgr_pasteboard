/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef PASTE_BOARD_CHANGER_OBSERVER_INTERFACE_H
#define PASTE_BOARD_CHANGER_OBSERVER_INTERFACE_H

#include "iremote_broker.h"

namespace OHOS {
namespace MiscServices {
enum class PasteboardEventStatus { PASTEBOARD_CLEAR = 1, PASTEBOARD_READ = 2, PASTEBOARD_WRITE = 3 };
enum class PasteboardObserverType { OBSERVER_LOCAL = 1, OBSERVER_REMOTE = 2, OBSERVER_ALL = 3, OBSERVER_EVENT = 4 };
class IPasteboardChangedObserver : public IRemoteBroker {
public:
    virtual void OnPasteboardChanged() = 0;
    virtual void OnPasteboardEvent(std::string bundleName, int32_t status) = 0;
    virtual ~IPasteboardChangedObserver() = default;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.pasteboard.IPasteboardChangedObserver");
    pid_t pid_ = 0;
    uint32_t tokenId_ = 0;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_CHANGER_OBSERVER_INTERFACE_H
