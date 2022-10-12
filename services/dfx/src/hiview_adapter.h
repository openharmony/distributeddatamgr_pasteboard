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

#ifndef MISCSERVICES_PASTEBOARD_HI_VIEW_ADAPTER_H
#define MISCSERVICES_PASTEBOARD_HI_VIEW_ADAPTER_H

#include <sys/time.h>

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "dfx_code_constant.h"
#include "dfx_types.h"
#include "hisysevent.h"

namespace OHOS {
namespace MiscServices {
enum DataConsumingLevel : std::int32_t {
    DATA_LEVEL_ONE = 0,
    DATA_LEVEL_TWO,
    DATA_LEVEL_THREE,
    DATA_LEVEL_FOUR,
    DATA_LEVEL_FIVE,
    DATA_LEVEL_SIX,
    DATA_LEVEL_SEVEN,
};
enum TimeConsumingLevel : std::int32_t {
    TIME_LEVEL_ONE = 0,
    TIME_LEVEL_TWO,
    TIME_LEVEL_THREE,
    TIME_LEVEL_FOUR,
    TIME_LEVEL_FIVE,
    TIME_LEVEL_SIX,
    TIME_LEVEL_SEVEN,
    TIME_LEVEL_EIGHT,
    TIME_LEVEL_NINE,
    TIME_LEVEL_TEN,
    TIME_LEVEL_ELEVEN,
};

class HiViewAdapter {
public:
    ~HiViewAdapter();
    static void ReportPasteboardFault(int dfxCode, const PasteboardFaultMsg &msg);
    static void ReportTimeConsumingStatistic(const TimeConsumingStat &stat);
    static void ReportPasteboardBehaviour(const PasteboardBehaviourMsg &msg);
    static void StartTimerThread();
    static std::map<int, int> InitDataMap();
    static std::map<int, int> InitTimeMap();

private:
    static void InvokePasteBoardBehaviour();
    static void InitializeTimeConsuming(int initFlag);
    static void CopyTimeConsumingCount(int dataLevel, int timeLevel);
    static void PasteTimeConsumingCount(int dataLevel, int timeLevel);
    static void RemotePasteTimeConsumingCount(int dataLevel, int timeLevel);
    static void CopyTimeConsuming(const TimeConsumingStat &stat, int level);
    static void PasteTimeConsuming(const TimeConsumingStat &stat, int level);
    static const char *GetDataLevel(int dataLevel);
    static void InvokeTimeConsuming();
    static void ReportBehaviour(std::map<std::string, int> &behaviour, const char *statePasteboard);
    static void ReportStatisticEvent(
        const std::vector<std::map<int, int>> &timeConsumingStat, const std::string &pasteboardState);

    static std::mutex timeConsumingMutex_;
    static std::vector<std::map<int, int>> copyTimeConsumingStat_;
    static std::vector<std::map<int, int>> pasteTimeConsumingStat_;
    static std::vector<std::map<int, int>> remotePasteTimeConsumingStat_;

    static std::mutex behaviourMutex_;
    static std::map<std::string, int> copyPasteboardBehaviour_;
    static std::map<std::string, int> pastePasteboardBehaviour_;
    static std::map<std::string, int> remotePastePasteboardBehaviour_;
    
    static std::map<int, int> dataMap_;
    static std::map<int, int> timeMap_;

    static std::string CoverEventID(int dfxCode);
private:
    static std::mutex runMutex_;
    static bool running_;
    
    static inline constexpr int ONE_DAY_IN_HOURS  = 24;
    static inline constexpr int EXEC_HOUR_TIME = 23;
    static inline constexpr int EXEC_MIN_TIME = 60;
    static inline constexpr int ONE_MINUTE_IN_SECONDS  = 60;
    static inline constexpr int ONE_HOUR_IN_SECONDS = 1 * 60 * 60; // 1 hour

// statistic key
    static inline const char *PASTEBOARD_STATE = "PASTEBOARD_STATE";
    
    static inline const char *ZERO_TO_HUNDRED_KB = "ZERO_TO_HUNDRED_KB";
    static inline const char *HUNDRED_TO_FIVE_HUNDREDS_KB = "HUNDRED_TO_FIVE_HUNDREDS_KB";
    static inline const char *FIVE_HUNDREDS_TO_THOUSAND_KB = "FIVE_HUNDREDS_TO_THOUSAND_KB";
    static inline const char *ONE_TO_FIVE_MB = "ONE_TO_FIVE_MB";
    static inline const char *FIVE_TO_TEN_MB = "FIVE_TO_TEN_MB";
    static inline const char *TEN_TO_FIFTY_MB = "TEN_TO_FIFTY_MB";
    static inline const char *OVER_FIFTY_MB = "OVER_FIFTY_MB";
    static inline const char *CONSUMING_DATA = "CONSUMING_DATA";
    static inline const char *DATA_LEVEL = "DATA_LEVEL";
    static inline constexpr const char *NET_TYPE = "NET_TYPE";
// behaviour key
    static inline const char *TOP_ONE_APP = "TOP_ONE_APP";
    static inline const char *TOP_TOW_APP = "TOP_TOW_APP";
    static inline const char *TOP_THREE_APP = "TOP_THREE_APP";
    static inline const char *TOP_FOUR_APP = "TOP_FOUR_APP";
    static inline const char *TOP_FIVE_APP = "TOP_FIVE_APP";
    static inline const char *TOP_SIX_APP = "TOP_SIX_APP";
    static inline const char *TOP_SEVEN_APP = "TOP_SEVEN_APP";
    static inline const char *TOP_EIGHT_APP = "TOP_EIGHT_APP";
    static inline const char *TOP_NINE_APP = "TOP_NINE_APP";
    static inline const char *TOP_TEN_APP = "TOP_TEN_APP";
    
    static inline const char *COPY_STATE = "COPY_STATE";
    static inline const char *PASTE_STATE = "PASTE_STATE";
    static inline constexpr const char *REMOTE_PASTE_STATE = "REMOTE_PASTE_STATE";
    
    static inline const int INIT_COPY_TIME_SONSUMING = 7;
    static inline const int INIT_PASTE_TIME_SONSUMING = 8;
    static inline constexpr const int INIT_REMOTE_PASTE_TIME_SONSUMING = 9;
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // MISCSERVICES_PASTEBOARD_HI_VIEW_ADAPTER_H
