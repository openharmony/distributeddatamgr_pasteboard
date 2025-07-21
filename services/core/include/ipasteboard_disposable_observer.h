/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef PASTEBOARD_DISPOSABLE_OBSERVER_INTERFACE_H
#define PASTEBOARD_DISPOSABLE_OBSERVER_INTERFACE_H

#include "iremote_broker.h"
#include "pasteboard_types.h"

namespace OHOS {
namespace MiscServices {
class IPasteboardDisposableObserver : public IRemoteBroker {
public:
    enum ErrCode : int32_t {
        ERR_OK = 0,
        ERR_TIMEOUT,
        ERR_NO_PERMISSION,
        ERR_TYPE_NOT_SUPPORT,
        ERR_TARGET_MISMATCH,
        ERR_LENGTH_MISMATCH,
        ERR_NO_TEXT,
        ERR_DATA_IN_APP,
    };

    virtual void OnTextReceived(const std::string &text, int32_t errCode) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.pasteboard.IPasteboardDisposableObserver");
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_DISPOSABLE_OBSERVER_INTERFACE_H
