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
enum TimeConsumingStatistic : std::int32_t {
    TCS_TIME_CONSUMING_LEVEL_ONE = 0,
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
};

enum BehaviourPasteboardState : std::int32_t {
    BPS_COPY_STATE = 0,
    BPS_PASTE_STATE,
    BPS_REMOTE_PASTE_STATE,
    BPS_INVALID_STATE,
};

enum StatisticPasteboardState : std::int32_t {
    SPS_COPY_STATE = 0,
    SPS_PASTE_STATE,
    SPS_REMOTE_PASTE_STATE,
    SPS_INVALID_STATE,
};

enum DataRange : std::int32_t {
    DR_ZERO_TO_HUNDRED_KB = 0,
    DR_HUNDRED_TO_FIVE_HUNDREDS_KB,
    DR_FIVE_HUNDREDS_TO_THOUSAND_KB,
    DR_ONE_TO_FIVE_MB,
    DR_FIVE_TO_TEN_MB,
    DR_TEN_TO_FIFTY_MB,
    DR_OVER_FIFTY_MB,
};

struct PasteboardFaultMsg {
    int32_t userId;
    std::string errorCode;
};

struct PasteboardBehaviourMsg {
    int pasteboardState;
    std::string bundleName;
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
