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

#include "pasteboarddumphelper_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "pasteboard_dump_helper.h"

namespace OHOS::MiscServices {
namespace {
constexpr uint8_t TARGET_SIZE = 1;

std::vector<std::string> CreateFuzzArgs(FuzzedDataProvider &fdp)
{
    std::vector<std::string> args;
    uint8_t count = fdp.ConsumeIntegralInRange<uint8_t>(0, 5);
    for (uint8_t i = 0; i < count; i++) {
        args.push_back(fdp.ConsumeRandomLengthString(64));
    }
    return args;
}

void DoDump(FuzzedDataProvider &fdp)
{
    auto args = CreateFuzzArgs(fdp);
    int devNull = open("/dev/null", O_WRONLY);
    if (devNull < 0) {
        return;
    }
    PasteboardDumpHelper::GetInstance().Dump(devNull, args);
    close(devNull);
}

using DoFunc = void (*)(FuzzedDataProvider &);
const DoFunc FUNC_LIST[] = {
    DoDump,
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
