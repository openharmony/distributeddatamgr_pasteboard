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

#define LOG_TAG "HiViewAdapter"

#include "hiview_adapter.h"

#include <unistd.h>

#include <thread>
#include <algorithm>
#include "def.h"
#include "pasteboard_hilog.h"

namespace OHOS {
using namespace HiviewDFX;
namespace MiscServices {
namespace {
const std::map<int, std::string> EVENT_COVERT_TABLE = {
    { DfxCodeConstant::PASTEBOARD_FAULT, "PASTEBOARD_FAULT" },
    { DfxCodeConstant::TIME_CONSUMING_STATISTIC, "TIME_CONSUMING_STATISTIC" },
    { DfxCodeConstant::PASTEBOARD_BEHAVIOUR, "PASTEBOARD_BEHAVIOUR" },
};
const std::string DOMAIN_STR = std::string(HiviewDFX::HiSysEvent::Domain::PASTEBOARD);
} // namespace

bool HiViewAdapter::running_ = false;
std::mutex HiViewAdapter::runMutex_;
std::mutex HiViewAdapter::timeConsumingMutex_;
std::mutex HiViewAdapter::behaviourMutex_;
std::vector<std::map<int, int>> HiViewAdapter::copyTimeConsumingStat_;
std::vector<std::map<int, int>> HiViewAdapter::pasteTimeConsumingStat_;
std::vector<std::map<int, int>> HiViewAdapter::remotePasteTimeConsumingStat_;

std::map<std::string, int> HiViewAdapter::copyPasteboardBehaviour_;
std::map<std::string, int> HiViewAdapter::pastePasteboardBehaviour_;
std::map<std::string, int> HiViewAdapter::remotePastePasteboardBehaviour_;

std::map<int, int> HiViewAdapter::dataMap_ = HiViewAdapter::InitDataMap();
std::map<int, int> HiViewAdapter::timeMap_ = HiViewAdapter::InitTimeMap();

std::map<int, int> HiViewAdapter::InitDataMap()
{
    std::map<int, int> dataMap;
    dataMap.insert(std::pair<int, int>(
        static_cast<int>(DataRange::DR_ZERO_TO_HUNDRED_KB), static_cast<int>(DataConsumingLevel::DATA_LEVEL_ONE)));
    dataMap.insert(std::pair<int, int>(static_cast<int>(DataRange::DR_HUNDRED_TO_FIVE_HUNDREDS_KB),
        static_cast<int>(DataConsumingLevel::DATA_LEVEL_TWO)));
    dataMap.insert(std::pair<int, int>(static_cast<int>(DataRange::DR_FIVE_HUNDREDS_TO_THOUSAND_KB),
        static_cast<int>(DataConsumingLevel::DATA_LEVEL_THREE)));
    dataMap.insert(std::pair<int, int>(
        static_cast<int>(DataRange::DR_ONE_TO_FIVE_MB), static_cast<int>(DataConsumingLevel::DATA_LEVEL_FOUR)));
    dataMap.insert(std::pair<int, int>(
        static_cast<int>(DataRange::DR_FIVE_TO_TEN_MB), static_cast<int>(DataConsumingLevel::DATA_LEVEL_FIVE)));
    dataMap.insert(std::pair<int, int>(
        static_cast<int>(DataRange::DR_TEN_TO_FIFTY_MB), static_cast<int>(DataConsumingLevel::DATA_LEVEL_SIX)));
    dataMap.insert(std::pair<int, int>(
        static_cast<int>(DataRange::DR_OVER_FIFTY_MB), static_cast<int>(DataConsumingLevel::DATA_LEVEL_SEVEN)));
    return dataMap;
}

std::map<int, int> HiViewAdapter::InitTimeMap()
{
    std::map<int, int> timeMap;
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_ONE),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_ONE)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_TWO),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_TWO)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_THREE),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_THREE)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_FOUR),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_FOUR)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_FIVE),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_FIVE)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_SIX),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_SIX)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_SEVEN),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_SEVEN)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_EIGHT),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_EIGHT)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_NINE),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_NINE)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_TEN),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_TEN)));
    timeMap.insert(std::pair<int, int>(static_cast<int>(TimeConsumingStatistic::TCS_TIME_CONSUMING_LEVEL_ELEVEN),
        static_cast<int>(TimeConsumingLevel::TIME_LEVEL_ELEVEN)));
    return timeMap;
}

void HiViewAdapter::ReportPasteboardFault(int dfxCode, const PasteboardFaultMsg &msg)
{
    constexpr const char *USER_ID = "USER_ID";
    constexpr const char *ERROR_TYPE = "ERROR_TYPE";
    int ret = HiSysEvent::Write(DOMAIN_STR, CoverEventID(dfxCode), HiSysEvent::EventType::FAULT, USER_ID, msg.userId,
        ERROR_TYPE, msg.errorCode);
    if (ret != 0) {
        PASTEBOARD_HILOGD(
            PASTEBOARD_MODULE_SERVICE, "hisysevent write failed! ret %{public}d. errCode %{public}d", ret, dfxCode);
    }
}

void HiViewAdapter::InitializeTimeConsuming(int initFlag)
{
    constexpr const int DATA_LEVEL_NUMBERS = 7;
    std::map<int, int> initTimeConsuming;
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_ONE), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_TWO), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_THREE), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_FOUR), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_FIVE), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_SIX), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_SEVEN), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_EIGHT), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_NINE), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_TEN), 0));
    initTimeConsuming.insert(std::pair<int, int>(static_cast<int>(TimeConsumingLevel::TIME_LEVEL_ELEVEN), 0));

    if (initFlag == INIT_COPY_TIME_SONSUMING) {
        for (int i = 0; i < DATA_LEVEL_NUMBERS; ++i) {
            copyTimeConsumingStat_.push_back(initTimeConsuming);
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "copyTimeConsumingStat_.push_back");
        }
    } else if (initFlag == INIT_PASTE_TIME_SONSUMING) {
        for (int i = 0; i < DATA_LEVEL_NUMBERS; ++i) {
            pasteTimeConsumingStat_.push_back(initTimeConsuming);
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "pasteTimeConsumingStat_.push_back");
        }
    } else {
        for (int i = 0; i < DATA_LEVEL_NUMBERS; ++i) {
            remotePasteTimeConsumingStat_.push_back(initTimeConsuming);
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "remotePasteTimeConsumingStat_.push_back");
        }
    }
}

void HiViewAdapter::ReportTimeConsumingStatistic(const TimeConsumingStat &stat)
{
    std::lock_guard<std::mutex> lock(timeConsumingMutex_);
    if (copyTimeConsumingStat_.empty()) {
        InitializeTimeConsuming(INIT_COPY_TIME_SONSUMING);
    }
    if (pasteTimeConsumingStat_.empty()) {
        InitializeTimeConsuming(INIT_PASTE_TIME_SONSUMING);
    }
    if (remotePasteTimeConsumingStat_.empty()) {
        InitializeTimeConsuming(INIT_REMOTE_PASTE_TIME_SONSUMING);
    }

    if (stat.pasteboardState == static_cast<int>(StatisticPasteboardState::SPS_COPY_STATE)) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "hisysevent pasteboard state is %{public}d", stat.pasteboardState);
        auto iter = dataMap_.find(stat.dataSize);
        if (iter != dataMap_.end()) {
            CopyTimeConsuming(stat, iter->second);
        } else {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "wrong data level");
        }
    } else if ((stat.pasteboardState == static_cast<int>(StatisticPasteboardState::SPS_PASTE_STATE)) ||
               (stat.pasteboardState == static_cast<int>(StatisticPasteboardState::SPS_REMOTE_PASTE_STATE))) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "hisysevent pasteboard state is %{public}d", stat.pasteboardState);
        auto iter = dataMap_.find(stat.dataSize);
        if (iter != dataMap_.end()) {
            PasteTimeConsuming(stat, iter->second);
        } else {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "wrong data level");
        }
    } else {
        PASTEBOARD_HILOGD(
            PASTEBOARD_MODULE_SERVICE, "hisysevent wrong pasteboard state! errCode %{public}d", stat.pasteboardState);
    }
}

void HiViewAdapter::CopyTimeConsumingCount(int dataLevel, int timeLevel)
{
    if (static_cast<int>(copyTimeConsumingStat_.size()) <= dataLevel) {
        return;
    }
    auto it = copyTimeConsumingStat_[dataLevel].find(timeLevel);
    if (it != copyTimeConsumingStat_[dataLevel].end()) {
        (it->second)++;
    } else {
        PASTEBOARD_HILOGD(
            PASTEBOARD_MODULE_SERVICE, "hisysevent wrong copy time level, tiem level:  %{public}d", timeLevel);
    }
}

void HiViewAdapter::PasteTimeConsumingCount(int dataLevel, int timeLevel)
{
    if (static_cast<int>(pasteTimeConsumingStat_.size()) <= dataLevel) {
        return;
    }
    auto it = pasteTimeConsumingStat_[dataLevel].find(timeLevel);
    if (it != pasteTimeConsumingStat_[dataLevel].end()) {
        (it->second)++;
    } else {
        PASTEBOARD_HILOGD(
            PASTEBOARD_MODULE_SERVICE, "hisysevent wrong copy time level, tiem level:  %{public}d", timeLevel);
    }
}

void HiViewAdapter::RemotePasteTimeConsumingCount(int dataLevel, int timeLevel)
{
    if (static_cast<int>(remotePasteTimeConsumingStat_.size()) <= dataLevel) {
        return;
    }
    auto it = remotePasteTimeConsumingStat_[dataLevel].find(timeLevel);
    if (it != remotePasteTimeConsumingStat_[dataLevel].end()) {
        (it->second)++;
    } else {
        PASTEBOARD_HILOGD(
            PASTEBOARD_MODULE_SERVICE, "hisysevent wrong copy time level, tiem level:  %{public}d", timeLevel);
    }
}

void HiViewAdapter::CopyTimeConsuming(const TimeConsumingStat &stat, int level)
{
    auto iter = timeMap_.find(stat.timeConsuming);
    if (iter != timeMap_.end()) {
        CopyTimeConsumingCount(level, iter->second);
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "wrong time level");
    }
}

void HiViewAdapter::PasteTimeConsuming(const TimeConsumingStat &stat, int level)
{
    auto iter = timeMap_.find(stat.timeConsuming);
    if (iter != timeMap_.end()) {
        if (stat.pasteboardState == SPS_PASTE_STATE) {
            PasteTimeConsumingCount(level, iter->second);
        } else {
            RemotePasteTimeConsumingCount(level, iter->second);
        }
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "wrong time level");
    }
}

void HiViewAdapter::ReportPasteboardBehaviour(const PasteboardBehaviourMsg &msg)
{
    std::lock_guard<std::mutex> lock(behaviourMutex_);

    if (msg.pasteboardState == static_cast<int>(BehaviourPasteboardState::BPS_COPY_STATE)) {
        auto it = copyPasteboardBehaviour_.find(msg.bundleName);
        if (it != copyPasteboardBehaviour_.end()) {
            (it->second)++;
        } else {
            copyPasteboardBehaviour_.insert(std::pair<std::string, int>(msg.bundleName, 1));
        }
    } else if (msg.pasteboardState == static_cast<int>(BehaviourPasteboardState::BPS_PASTE_STATE)) {
        auto it = pastePasteboardBehaviour_.find(msg.bundleName);
        if (it != pastePasteboardBehaviour_.end()) {
            (it->second)++;
        } else {
            pastePasteboardBehaviour_.insert(std::pair<std::string, int>(msg.bundleName, 1));
        }
    } else if (msg.pasteboardState == static_cast<int>(BehaviourPasteboardState::BPS_REMOTE_PASTE_STATE)) {
        auto it = remotePastePasteboardBehaviour_.find(msg.bundleName);
        if (it != remotePastePasteboardBehaviour_.end()) {
            (it->second)++;
        } else {
            remotePastePasteboardBehaviour_.insert(std::pair<std::string, int>(msg.bundleName, 1));
        }
    } else {
        PASTEBOARD_HILOGD(
            PASTEBOARD_MODULE_SERVICE, "hisysevent wrong pasteboard state! errCode %{public}d", msg.pasteboardState);
    }
}

const char *HiViewAdapter::GetDataLevel(int dataLevel)
{
    constexpr const char *WRONG_LEVEL = "WRONG_LEVEL";
    switch (dataLevel) {
        case static_cast<int>(DataConsumingLevel::DATA_LEVEL_ONE): {
            return ZERO_TO_HUNDRED_KB;
        }
        case static_cast<int>(DataConsumingLevel::DATA_LEVEL_TWO): {
            return HUNDRED_TO_FIVE_HUNDREDS_KB;
        }
        case static_cast<int>(DataConsumingLevel::DATA_LEVEL_THREE): {
            return FIVE_HUNDREDS_TO_THOUSAND_KB;
        }
        case static_cast<int>(DataConsumingLevel::DATA_LEVEL_FOUR): {
            return ONE_TO_FIVE_MB;
        }
        case static_cast<int>(DataConsumingLevel::DATA_LEVEL_FIVE): {
            return FIVE_TO_TEN_MB;
        }
        case static_cast<int>(DataConsumingLevel::DATA_LEVEL_SIX): {
            return TEN_TO_FIFTY_MB;
        }
        case static_cast<int>(DataConsumingLevel::DATA_LEVEL_SEVEN): {
            return OVER_FIFTY_MB;
        }
        default: {
            return WRONG_LEVEL;
        }
    }
}

void HiViewAdapter::InvokeTimeConsuming()
{
    std::lock_guard<std::mutex> lock(timeConsumingMutex_);
    ReportStatisticEvent(copyTimeConsumingStat_, COPY_STATE);
    copyTimeConsumingStat_.clear();
    ReportStatisticEvent(pasteTimeConsumingStat_, PASTE_STATE);
    pasteTimeConsumingStat_.clear();
    ReportStatisticEvent(remotePasteTimeConsumingStat_, REMOTE_PASTE_STATE);
    remotePasteTimeConsumingStat_.clear();
}

void HiViewAdapter::ReportStatisticEvent(
    const std::vector<std::map<int, int>> &timeConsumingStat, const std::string &pasteboardState)
{
    if (timeConsumingStat.empty()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "hisysevent timeConsumingStat is empty.");
        return;
    }
    for (std::int32_t i = 0; i < static_cast<int>(timeConsumingStat.size()); ++i) {
        std::string buffMsg = ": [";
        for (std::int32_t j = TimeConsumingLevel::TIME_LEVEL_ONE; j <= TimeConsumingLevel::TIME_LEVEL_ELEVEN; ++j) {
            buffMsg = buffMsg + std::to_string(timeConsumingStat[i].at(j)) + ",";
        }
        buffMsg += "]";

        int ret = -1;
        if (pasteboardState == REMOTE_PASTE_STATE) {
            std::string netType = "WIFI";
            ret = HiSysEvent::Write(DOMAIN_STR, CoverEventID(DfxCodeConstant::TIME_CONSUMING_STATISTIC),
                HiSysEvent::EventType::STATISTIC, PASTEBOARD_STATE, pasteboardState, NET_TYPE, netType, DATA_LEVEL,
                GetDataLevel(i), CONSUMING_DATA, buffMsg);
        } else {
            ret = HiSysEvent::Write(DOMAIN_STR, CoverEventID(DfxCodeConstant::TIME_CONSUMING_STATISTIC),
                HiSysEvent::EventType::STATISTIC, PASTEBOARD_STATE, pasteboardState, DATA_LEVEL, GetDataLevel(i),
                CONSUMING_DATA, buffMsg);
        }

        if (ret != HiviewDFX::SUCCESS) {
            PASTEBOARD_HILOGD(
                PASTEBOARD_MODULE_SERVICE, "hisysevent write failed! ret = %{public}d, i = %{public}d.", ret, i);
        }
    }
}

void HiViewAdapter::ReportBehaviour(std::map<std::string, int> &behaviour, const char *pasteboardState)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ReportBehaviour  enter");
    if (!behaviour.empty()) {
        std::vector<std::pair<std::string, int>> vec;
        constexpr const int TOTAL_APP_NUMBERS = 10;

        for (auto it = behaviour.begin(); it != behaviour.end(); ++it) {
            vec.push_back(std::pair<std::string, int>(it->first, it->second));
        }
// sort
        sort(vec.begin(), vec.end(), [](std::pair<std::string, int>a, std::pair<std::string, int>b) {
            return a.second > b.second;
        });

// init container for report.
        std::vector<std::string> appPackName;
        for (int i = 0; i < TOTAL_APP_NUMBERS; ++i) {
            appPackName.push_back("default");
        }

// push in container.
        int j = 0;
        for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
            appPackName[j] = iter->first + " :" + std::to_string(iter->second);
            ++j;
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ReportBehaviour report  ");
        int ret = HiSysEvent::Write(DOMAIN_STR, CoverEventID(DfxCodeConstant::PASTEBOARD_BEHAVIOUR),
            HiSysEvent::EventType::BEHAVIOR,
            PASTEBOARD_STATE, pasteboardState,
            TOP_ONE_APP, appPackName[0],
            TOP_TOW_APP, appPackName[1],
            TOP_THREE_APP, appPackName[2],
            TOP_FOUR_APP, appPackName[3],
            TOP_FIVE_APP, appPackName[4],
            TOP_SIX_APP, appPackName[5],
            TOP_SEVEN_APP, appPackName[6],
            TOP_EIGHT_APP, appPackName[7],
            TOP_NINE_APP, appPackName[8],
            TOP_TEN_APP, appPackName[9]);
        if (ret != HiviewDFX::SUCCESS) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "hisysevent write failed! ret %{public}d.", ret);
        }
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "behaviour is empty!");
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "ReportBehaviour end");
}

void HiViewAdapter::InvokePasteBoardBehaviour()
{
    std::lock_guard<std::mutex> lock(behaviourMutex_);
    if (!copyPasteboardBehaviour_.empty()) {
        ReportBehaviour(copyPasteboardBehaviour_, COPY_STATE);
        copyPasteboardBehaviour_.clear();
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "InvokePasteBoardBehaviour :copyPasteboardBehaviour_ is empty ");
    }
    if (!pastePasteboardBehaviour_.empty()) {
        ReportBehaviour(pastePasteboardBehaviour_, PASTE_STATE);
        pastePasteboardBehaviour_.clear();
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "InvokePasteBoardBehaviour :pastePasteboardBehaviour_ is empty ");
    }
    if (!remotePastePasteboardBehaviour_.empty()) {
        ReportBehaviour(remotePastePasteboardBehaviour_, REMOTE_PASTE_STATE);
        remotePastePasteboardBehaviour_.clear();
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "remotePastePasteboardBehaviour_ is empty ");
    }
}

void HiViewAdapter::StartTimerThread()
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "StartTimerThread enter");
    std::lock_guard<std::mutex> lock(runMutex_);
    if (running_) {
        return;
    }
    running_ = true;
    auto fun = []() {
        while (true) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "StartTimerThread while");
            time_t current = time(nullptr);
            if (current == -1) {
                sleep(ONE_HOUR_IN_SECONDS);
                continue;
            }
            
            tm localTime = { 0 };
            tm *result = localtime_r(&current, &localTime);
            if (result == nullptr) {
                sleep(ONE_HOUR_IN_SECONDS);
                continue;
            }
            int currentHour = localTime.tm_hour;
            int currentMin = localTime.tm_min;
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "StartTimerThread get");
            if ((EXEC_MIN_TIME - currentMin) != EXEC_MIN_TIME) {
                int nHours = EXEC_HOUR_TIME - currentHour;
                int nMin = EXEC_MIN_TIME - currentMin;
                int nTime = (nMin) * ONE_MINUTE_IN_SECONDS + (nHours) * ONE_HOUR_IN_SECONDS;
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE,
                    " StartTimerThread if needHours=%{public}d,needMin=%{public}d,needTime=%{public}d", nHours,
                    nMin, nTime);
                sleep(nTime);
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "StartTimerThread invoke");
                InvokePasteBoardBehaviour();
                InvokeTimeConsuming();
            } else {
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "StartTimerThread sleep");
                sleep(ONE_HOUR_IN_SECONDS * (ONE_DAY_IN_HOURS - currentHour));
                current = time(nullptr);
                InvokePasteBoardBehaviour();
                InvokeTimeConsuming();
            }
                PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "StartTimerThread end");
        }
    };
    std::thread th = std::thread(fun);
    th.detach();
}

std::string HiViewAdapter::CoverEventID(int dfxCode)
{
    std::string sysEventID = "";
    auto operatorIter = EVENT_COVERT_TABLE.find(dfxCode);
    if (operatorIter != EVENT_COVERT_TABLE.end()) {
        sysEventID = operatorIter->second;
    }
    return sysEventID;
}
} // namespace MiscServices
} // namespace OHOS
