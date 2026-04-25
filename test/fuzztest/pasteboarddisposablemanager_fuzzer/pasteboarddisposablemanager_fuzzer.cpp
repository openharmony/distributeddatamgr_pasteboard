/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "pasteboarddisposablemanager_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "pasteboard_disposable_manager.h"

namespace OHOS::MiscServices {
namespace {
constexpr uint8_t TARGET_SIZE = 2;

void DoAddDisposableInfo(FuzzedDataProvider &fdp)
{
    pid_t pid = static_cast<pid_t>(fdp.ConsumeIntegral<int32_t>());
    uint32_t tokenId = fdp.ConsumeIntegral<uint32_t>();
    int32_t targetWindowId = fdp.ConsumeIntegral<int32_t>();
    uint8_t typeVal = fdp.ConsumeIntegral<uint8_t>() % static_cast<uint8_t>(DisposableType::MAX);
    DisposableType type = static_cast<DisposableType>(typeVal);
    uint32_t maxLen = fdp.ConsumeIntegral<uint32_t>();
    DisposableInfo info(pid, tokenId, targetWindowId, type, maxLen, nullptr);
    DisposableManager::GetInstance().AddDisposableInfo(info);
}

void DoRemoveDisposableInfo(FuzzedDataProvider &fdp)
{
    pid_t pid = static_cast<pid_t>(fdp.ConsumeIntegral<int32_t>());
    bool needNotify = fdp.ConsumeBool();
    DisposableManager::GetInstance().RemoveDisposableInfo(pid, needNotify);
}

using DoFunc = void (*)(FuzzedDataProvider &);
const DoFunc FUNC_LIST[] = {
    DoAddDisposableInfo,
    DoRemoveDisposableInfo,
};
} // namespace
} // namespace OHOS::MiscServices

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 1) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    uint8_t tarPos = fdp.ConsumeIntegral<uint8_t>() % OHOS::MiscServices::TARGET_SIZE;
    OHOS::MiscServices::FUNC_LIST[tarPos](fdp);
    return 0;
}
