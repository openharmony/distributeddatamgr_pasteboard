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

#include "reporter.h"

#include "behaviour/pasteboard_behaviour_reporter_impl.h"
#include "fault/pasteboard_fault_impl.h"
#include "statistic/time_consuming_statistic_impl.h"

namespace OHOS {
namespace MiscServices {
Reporter &Reporter::GetInstance()
{
    static Reporter reporter;
    return reporter;
}

FaultReporter &Reporter::PasteboardFault()
{
    static PasteboardFaultImpl pasteboardFault;
    return pasteboardFault;
}

StatisticReporter<struct TimeConsumingStat> &Reporter::TimeConsumingStatistic()
{
    static TimeConsumingStatisticImpl TimeConsumingStatistic;
    return TimeConsumingStatistic;
}

BehaviourReporter &Reporter::PasteboardBehaviour()
{
    static PasteboardBehaviourReporterImpl PasteboardBehaviourReporter;
    return PasteboardBehaviourReporter;
}
} // namespace MiscServices
} // namespace OHOS
