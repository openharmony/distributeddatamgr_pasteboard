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

#include "eventcenter_fuzzer.h"
#include "eventcenter/pasteboard_event.h"
#include "eventcenter/event_center.h"
#include <memory>

namespace OHOS {
using namespace OHOS::MiscServices;

template<class T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

bool FuzzPbEventGetNetworkId(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    int32_t evtId = TypeCast<int32_t>(rawData);
    PasteboardEvent pbEvt(evtId, "pasteboard_fuzz_test");
    pbEvt.GetNetworkId();
    pbEvt.GetEventId();
    return true;
}

bool FuzzEventCenterSubscribe(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    int32_t evtId = TypeCast<int32_t>(rawData);
    EventCenter::GetInstance().Subscribe(evtId, [](const Event &evt) {
    });
    EventCenter::GetInstance().Unsubscribe(evtId);
    return true;
}

bool FuzzEventCenterPostEvent(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    int32_t evtId = TypeCast<int32_t>(rawData);
    EventCenter::GetInstance().PostEvent(nullptr);
    EventCenter::GetInstance().PostEvent(std::make_unique<Event>(evtId));
    return true;
}

bool FuzzEventCenterDefer(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    int32_t evtId = TypeCast<int32_t>(rawData);
    EventCenter::Defer Defer([](const Event &evt) {
    }, evtId);
    return true;
}

} // namespace OHOS
/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzPbEventGetNetworkId(data, size);
    OHOS::FuzzEventCenterSubscribe(data, size);
    OHOS::FuzzEventCenterPostEvent(data, size);
    OHOS::FuzzEventCenterDefer(data, size);
    return 0;
}
