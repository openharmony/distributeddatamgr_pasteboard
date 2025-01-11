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

#ifndef I_PASTEBOARD_SIGNAL_H
#define I_PASTEBOARD_SIGNAL_H

#include "iremote_broker.h"

namespace OHOS {
namespace MiscServices {
class IPasteboardSignal : public IRemoteBroker {
public:
    virtual ~IPasteboardSignal() = default;
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.miscservices.dialog.callback");

    virtual void HandleProgressSignalValue(const std::string &signalValue) = 0;
    enum InterfaceCode {
        GET_PROGRESS_SIGNAL = 0,
    };
};
} // namespace MiscServices
} // namespace OHOS
#endif // I_PASTEBOARD_SIGNAL_H
