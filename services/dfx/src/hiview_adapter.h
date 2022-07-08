/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <map>
#include <mutex>
#include <vector>
#include <string>
#include <sys/time.h>
#include "dfx_types.h"
#include "hisysevent.h"
#include "dfx_code_constant.h"
#include "hisysevent.h"

namespace OHOS {
namespace MiscServices {
enum class DataConsumingLevel {
    DATA_LEVEL_ONE = 0,
    DATA_LEVEL_TWO = 1,
    DATA_LEVEL_THREE = 2,
    DATA_LEVEL_FOUR = 3,
    DATA_LEVEL_FIVE = 4,
    DATA_LEVEL_SIX = 5,
    DATA_LEVEL_SEVEN = 6,
};
enum class TimeConsumingLevel {
    TIME_LEVEL_ONE = 72,
    TIME_LEVEL_TWO = 73,
    TIME_LEVEL_THREE = 74,
    TIME_LEVEL_FOUR = 75,
    TIME_LEVEL_FIVE = 76,
    TIME_LEVEL_SIX = 77,
    TIME_LEVEL_SEVEN = 78,
    TIME_LEVEL_EIGHT = 79,
    TIME_LEVEL_NINE = 80,
    TIME_LEVEL_TEN = 81,
    TIME_LEVEL_ELEVEN = 82,
};

class HiViewAdapter {
public:
    ~HiViewAdapter();
    static void ReportInitializationFault(int dfxCode, const InitializationFaultMsg &msg);
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
    static void CopyTimeConsuming(const TimeConsumingStat &stat, int level);
    static void PasteTimeConsuming(const TimeConsumingStat &stat, int level);
    static const char *GetDataLevel(int dataLevel);
    static void InvokeTimeConsuming();
    static void ReportBehaviour(std::map<std::string, int> &behaviour, const char *statePasteboard);

    static std::mutex timeConsumingMutex_;
    static std::vector<std::map<int, int>> copyTimeConsumingStat_;
    static std::vector<std::map<int, int>> pasteTimeConsumingStat_;

    static std::mutex behaviourMutex_;
    static std::map<std::string, int> copyPasteboardBehaviour_;
    static std::map<std::string, int> pastePasteboardBehaviour_;
    
    static std::map<int, int> dataMap_;
    static std::map<int, int> timeMap_;

    static std::string CoverEventID(int dfxCode);
private:
    static std::mutex runMutex_;
    static bool running_;
    
    static const inline int ONE_DAY_IN_HOURS  = 24;
    static const inline int EXEC_HOUR_TIME = 23;
    static const inline int EXEC_MIN_TIME = 60;
    static const inline int ONE_MINUTE_IN_SECONDS  = 60;
    static const inline int ONE_HOUR_IN_SECONDS = 1 * 60 * 60; // 1 hour
// fault key
    static const inline char *USER_ID = "USER_ID";
    static const inline char *ERROR_TYPE = "ERROR_TYPE";
// statistic key
    static const inline char *PASTEBOARD_STATE = "PASTEBOARD_STATE";
    static const inline char *DATA_LEVEL = "DATA_LEVEL";

    static const inline char *ZERO_TO_HUNDRED_KB = "ZERO_TO_HUNDRED_KB";
    static const inline char *HUNDRED_TO_FIVE_HUNDREDS_KB = "HUNDRED_TO_FIVE_HUNDREDS_KB";
    static const inline char *FIVE_HUNDREDS_TO_THOUSAND_KB = "FIVE_HUNDREDS_TO_THOUSAND_KB";
    static const inline char *ONE_TO_FIVE_MB = "ONE_TO_FIVE_MB";
    static const inline char *FIVE_TO_TEN_MB = "FIVE_TO_TEN_MB";
    static const inline char *TEN_TO_FIFTY_MB = "TEN_TO_FIFTY_MB";
    static const inline char *OVER_FIFTY_MB = "OVER_FIFTY_MB";

    static const inline char *TIME_CONSUMING_LEVEL_ONE = "TIME_CONSUMING_LEVEL_ONE";
    static const inline char *TIME_CONSUMING_LEVEL_TWO = "TIME_CONSUMING_LEVEL_TWO";
    static const inline char *TIME_CONSUMING_LEVEL_THREE = "TIME_CONSUMING_LEVEL_THREE";
    static const inline char *TIME_CONSUMING_LEVEL_FOUR = "TIME_CONSUMING_LEVEL_FOUR";
    static const inline char *TIME_CONSUMING_LEVEL_FIVE = "TIME_CONSUMING_LEVEL_FIVE";
    static const inline char *TIME_CONSUMING_LEVEL_SIX = "TIME_CONSUMING_LEVEL_SIX";
    static const inline char *TIME_CONSUMING_LEVEL_SEVEN = "TIME_CONSUMING_LEVEL_SEVEN";
    static const inline char *TIME_CONSUMING_LEVEL_EIGHT = "TIME_CONSUMING_LEVEL_EIGHT";
    static const inline char *TIME_CONSUMING_LEVEL_NINE = "TIME_CONSUMING_LEVEL_NINE";
    static const inline char *TIME_CONSUMING_LEVEL_TEN = "TIME_CONSUMING_LEVEL_TEN";
    static const inline char *TIME_CONSUMING_LEVEL_ELEVEN = "TIME_CONSUMING_LEVEL_ELEVEN";
// behaviour key
    static const inline char *TOP_ONE_APP = "TOP_ONE_APP";
    static const inline char *TOP_TOW_APP = "TOP_TOW_APP";
    static const inline char *TOP_THREE_APP = "TOP_THREE_APP";
    static const inline char *TOP_FOUR_APP = "TOP_FOUR_APP";
    static const inline char *TOP_FIVE_APP = "TOP_FIVE_APP";
    static const inline char *TOP_SIX_APP = "TOP_SIX_APP";
    static const inline char *TOP_SEVEN_APP = "TOP_SEVEN_APP";
    static const inline char *TOP_EIGHT_APP = "TOP_EIGHT_APP";
    static const inline char *TOP_NINE_APP = "TOP_NINE_APP";
    static const inline char *TOP_TEN_APP = "TOP_TEN_APP";

    static const inline char *WRONG_LEVEL = "WRONG_LEVEL";

    static const inline char *COPY_STATE = "COPY_STATE";
    static const inline char *PASTE_STATE = "PASTE_STATE";

    static const inline int INIT_COPY_TIME_SONSUMING = 80;
    static const inline int INIT_PASTE_TIME_SONSUMING = 81;
};
}  // namespace MiscServices
}  // namespace OHOS
#endif // MISCSERVICES_PASTEBOARD_HI_VIEW_ADAPTER_H
