/*
 * Copyright (C) 2022-2022 Huawei Device Co., Ltd.
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

#ifndef MISCSERVICES_PASTEBOARD_DFX_TYPES_H
#define MISCSERVICES_PASTEBOARD_DFX_TYPES_H

#include <string>
#include <memory>
#include <map>

namespace OHOS {
namespace MiscServices {
enum Fault : std::int32_t{
    // Time Consuming Stat
    TCS_COPY_STATE = 0,
    TCS_PASTE_STATE,
    TCS_DEVICES_PASTE_WIFI_STATE,
    TCS_DEVICES_PASTE_BTONLY_STATE,
    TCS_PASTEBOARD_STATE,
    TCS_ZERO_TO_HUNDRED_KB,
    TCS_HUNDRED_TO_FIVE_HUNDREDS_KB,
    TCS_FIVE_HUNDREDS_TO_THOUSAND_KB,
    TCS_ONE_TO_FIVE_MB,
    TCS_FIVE_TO_TEN_MB,
    TCS_TEN_TO_FIFTY_MB,
    TCS_OVER_FIFTY_MB,
    TCS_TIME_CONSUMING_LEVEL_ONE,
    TCS_TIME_CONSUMING_LEVEL_TWO,
    TCS_TIME_CONSUMING_LEVEL_THREE,
    TCS_TIME_CONSUMING_LEVEL_FOUR,
    TCS_TIME_CONSUMING_LEVEL_FIVE,
    TCS_TIME_CONSUMING_LEVEL_SIX,
    TCS_TIME_CONSUMING_LEVEL_SEVEN,
    TCS_TIME_CONSUMING_LEVEL_EIGHT,
    TCS_TIME_CONSUMING_LEVEL_NINE,
    TCS_TIME_CONSUMING_LEVEL_TEN,
    TCS_TIME_CONSUMING_LEVEL_ELEVEN,
    PB_COPY_STATE,
    PB_PASTE_STATE,
};

struct InitializationFaultMsg {
    int32_t userId;
    std::string errorCode;
};

struct PasteboardBehaviourMsg {
    int pasteboardState;
    std::string appId;
};

struct TimeConsumingStat {
    int pasteboardState;
    int dataSize;
    int timeConsuming;
};

enum class ReportStatus {
    SUCCESS = 0,
    ERROR = 1,
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // MISCSERVICES_PASTEBOARD_DFX_TYPES_H
