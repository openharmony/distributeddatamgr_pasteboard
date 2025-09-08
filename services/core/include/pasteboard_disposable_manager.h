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

#ifndef PASTEBOARD_DISPOSABLE_MANAGER_H
#define PASTEBOARD_DISPOSABLE_MANAGER_H

#include "ipasteboard_delay_getter.h"
#include "ipasteboard_entry_getter.h"
#include "ipasteboard_disposable_observer.h"
#include "paste_data.h"

namespace OHOS {
namespace MiscServices {
struct DisposableInfo {
    pid_t pid;
    uint32_t tokenId;
    DisposableType type;
    uint32_t maxLen;
    int32_t targetWindowId;
    sptr<IPasteboardDisposableObserver> observer;

    DisposableInfo(pid_t pid, uint32_t tokenId, int32_t targetWindowId, DisposableType type,
        uint32_t maxLen, const sptr<IPasteboardDisposableObserver> observer) : pid(pid), tokenId(tokenId),
        type(type), maxLen(maxLen), targetWindowId(targetWindowId), observer(observer) {}
};

class DisposableManager {
public:
    static DisposableManager &GetInstance();
    bool TryProcessDisposableData(PasteData &pasteData,
        const sptr<IPasteboardDelayGetter> &delayGetter, const sptr<IPasteboardEntryGetter> &entryGetter);
    int32_t AddDisposableInfo(const DisposableInfo &info);
    void RemoveDisposableInfo(pid_t pid, bool needNotify);

private:
    std::string GetPlainText(PasteData &pasteData,
        const sptr<IPasteboardDelayGetter> &delayGetter, const sptr<IPasteboardEntryGetter> &entryGetter);
    void ProcessMatchedInfo(const std::vector<DisposableInfo> &matchedInfoList, PasteData &pasteData,
        const sptr<IPasteboardDelayGetter> &delayGetter, const sptr<IPasteboardEntryGetter> &entryGetter);
    void ProcessNoMatchInfo(const std::vector<DisposableInfo> &noMatchInfoList);

    static constexpr int32_t DISPOSABLE_EXPIRATION_DEFAULT = 100; // ms
    static constexpr int32_t DISPOSABLE_EXPIRATION_MIN = 1; // ms
    static constexpr int32_t DISPOSABLE_EXPIRATION_MAX = 200; // ms
    std::mutex disposableInfoMutex_;
    std::vector<DisposableInfo> disposableInfoList_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_DISPOSABLE_MANAGER_H
