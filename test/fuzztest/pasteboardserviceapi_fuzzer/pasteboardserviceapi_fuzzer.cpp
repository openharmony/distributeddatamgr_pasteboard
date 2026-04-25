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

#include "pasteboardserviceapi_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "pasteboard_service.h"

namespace OHOS::MiscServices {
namespace {
constexpr uint8_t MAX_PATTERN_COUNT = static_cast<uint8_t>(Pattern::COUNT);
constexpr uint8_t TARGET_SIZE = 8;

std::shared_ptr<PasteboardService> g_service = nullptr;

std::vector<Pattern> CreateFuzzPatterns(FuzzedDataProvider &fdp)
{
    std::vector<Pattern> patterns;
    uint8_t count = fdp.ConsumeIntegralInRange<uint8_t>(1, MAX_PATTERN_COUNT);
    for (uint8_t i = 0; i < count; i++) {
        Pattern p = static_cast<Pattern>(fdp.ConsumeIntegral<uint8_t>() % MAX_PATTERN_COUNT);
        patterns.push_back(p);
    }
    return patterns;
}

void DoCallbackEnter(FuzzedDataProvider &fdp)
{
    uint32_t code = fdp.ConsumeIntegral<uint32_t>();
    g_service->CallbackEnter(code);
}

void DoCallbackExit(FuzzedDataProvider &fdp)
{
    uint32_t code = fdp.ConsumeIntegral<uint32_t>();
    int32_t result = fdp.ConsumeIntegral<int32_t>();
    g_service->CallbackExit(code, result);
}

void DoHasDataType(FuzzedDataProvider &fdp)
{
    std::string mimeType = fdp.ConsumeRandomLengthString(128);
    bool funcResult = false;
    g_service->HasDataType(mimeType, funcResult);
}

void DoHasUtdType(FuzzedDataProvider &fdp)
{
    std::string utdType = fdp.ConsumeRandomLengthString(128);
    bool funcResult = false;
    g_service->HasUtdType(utdType, funcResult);
}

void DoDetectPatterns(FuzzedDataProvider &fdp)
{
    auto patternsToCheck = CreateFuzzPatterns(fdp);
    std::vector<Pattern> funcResult;
    g_service->DetectPatterns(patternsToCheck, funcResult);
}

void DoDump(FuzzedDataProvider &fdp)
{
    std::vector<std::u16string> args;
    uint8_t argCount = fdp.ConsumeIntegralInRange<uint8_t>(0, 4);
    for (uint8_t i = 0; i < argCount; i++) {
        std::string arg = fdp.ConsumeRandomLengthString(64);
        args.push_back(std::u16string(arg.begin(), arg.end()));
    }
    int fd = open("/dev/null", O_WRONLY);
    if (fd < 0) {
        return;
    }
    g_service->Dump(fd, args);
    close(fd);
}

void DoSetAppShareOptions(FuzzedDataProvider &fdp)
{
    int32_t shareOptions = fdp.ConsumeIntegral<int32_t>();
    g_service->SetAppShareOptions(shareOptions);
}

void DoPasteStart(FuzzedDataProvider &fdp)
{
    std::string pasteId = fdp.ConsumeRandomLengthString(64);
    g_service->PasteStart(pasteId);
}

using DoFunc = void (*)(FuzzedDataProvider &);
const DoFunc FUNC_LIST[] = {
    DoCallbackEnter,
    DoCallbackExit,
    DoHasDataType,
    DoHasUtdType,
    DoDetectPatterns,
    DoDump,
    DoSetAppShareOptions,
    DoPasteStart,
};
} // namespace
} // namespace OHOS::MiscServices

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::MiscServices::g_service = std::make_shared<OHOS::MiscServices::PasteboardService>();
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 1 || OHOS::MiscServices::g_service == nullptr) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    uint8_t tarPos = fdp.ConsumeIntegral<uint8_t>() % OHOS::MiscServices::TARGET_SIZE;
    OHOS::MiscServices::FUNC_LIST[tarPos](fdp);
    return 0;
}
