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

#include "pasteboarddialog_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "pasteboard_dialog.h"

namespace OHOS::MiscServices {
namespace {
constexpr uint8_t TARGET_SIZE = 1;

PasteboardDialog::ProgressMessageInfo CreateFuzzMessageInfo(FuzzedDataProvider &fdp)
{
    PasteboardDialog::ProgressMessageInfo info;
    info.promptText = fdp.ConsumeRandomLengthString(128);
    info.remoteDeviceName = fdp.ConsumeRandomLengthString(64);
    info.progressKey = fdp.ConsumeRandomLengthString(64);
    info.isRemote = fdp.ConsumeBool();
    info.left = fdp.ConsumeIntegral<int32_t>();
    info.top = fdp.ConsumeIntegral<int32_t>();
    info.width = fdp.ConsumeIntegral<int32_t>();
    info.height = fdp.ConsumeIntegral<int32_t>();
    info.callerToken = nullptr;
    info.clientCallback = nullptr;
    return info;
}

void DoShowProgress(FuzzedDataProvider &fdp)
{
    auto info = CreateFuzzMessageInfo(fdp);
    PasteboardDialog::ShowProgress(info);
}

using DoFunc = void (*)(FuzzedDataProvider &);
const DoFunc FUNC_LIST[] = {
    DoShowProgress,
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
