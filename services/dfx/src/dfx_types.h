/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
enum class Fault {
    // Time Consuming Stat
    TCS_COPY_STATE = 47,
    TCS_PASTE_STATE = 48,
    TCS_DEVICES_PASTE_WIFI_STATE = 49,
    TCS_DEVICES_PASTE_BTONLY_STATE = 50,
    TCS_PASTEBOARD_STATE = 51,

    TCS_ZERO_TO_HUNDRED_KB = 52,
    TCS_HUNDRED_TO_FIVE_HUNDREDS_KB = 53,
    TCS_FIVE_HUNDREDS_TO_THOUSAND_KB = 54,
    TCS_ONE_TO_FIVE_MB = 56,
    TCS_FIVE_TO_TEN_MB = 55,
    TCS_TEN_TO_FIFTY_MB = 57,
    TCS_OVER_FIFTY_MB = 58,

    TCS_TIME_CONSUMING_LEVEL_ONE = 59,
    TCS_TIME_CONSUMING_LEVEL_TWO = 60,
    TCS_TIME_CONSUMING_LEVEL_THREE = 61,
    TCS_TIME_CONSUMING_LEVEL_FOUR = 62,
    TCS_TIME_CONSUMING_LEVEL_FIVE = 63,
    TCS_TIME_CONSUMING_LEVEL_SIX = 64,
    TCS_TIME_CONSUMING_LEVEL_SEVEN = 65,
    TCS_TIME_CONSUMING_LEVEL_EIGHT = 66,
    TCS_TIME_CONSUMING_LEVEL_NINE = 67,
    TCS_TIME_CONSUMING_LEVEL_TEN = 68,
    TCS_TIME_CONSUMING_LEVEL_ELEVEN = 69,

    // Pasteboard Behaviour
    PB_COPY_STATE = 70,
    PB_PASTE_STATE = 71,
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
