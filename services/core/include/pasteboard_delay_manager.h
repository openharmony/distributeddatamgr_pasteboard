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

#ifndef PASTEBOARD_DELAY_MANAGER_H
#define PASTEBOARD_DELAY_MANAGER_H

#include "ipasteboard_entry_getter.h"
#include "paste_data.h"

namespace OHOS {
namespace MiscServices {
struct DelayEntryInfo {
    uint8_t priority = 0;
    uint32_t recordId = 0;
    std::shared_ptr<PasteDataEntry> entry;

    DelayEntryInfo(uint8_t priority, uint32_t recordId, std::shared_ptr<PasteDataEntry> entry)
        : priority(priority), recordId(recordId), entry(entry) {}
};

class DelayManager {
public:
    static std::vector<DelayEntryInfo> GetAllDelayEntryInfo(const PasteData &data);
    static std::vector<DelayEntryInfo> GetPrimaryDelayEntryInfo(const PasteData &data);
    static void GetLocalEntryValue(const std::vector<DelayEntryInfo> &delayEntryInfos,
        sptr<IPasteboardEntryGetter> entryGetter, PasteData &data);

private:
    static uint8_t GetEntryPriority(const std::string &utdId);
    static void SortEntryInfo(std::vector<DelayEntryInfo> &entryInfos);
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_DELAY_MANAGER_H
