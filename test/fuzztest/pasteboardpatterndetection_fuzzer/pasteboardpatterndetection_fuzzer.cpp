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

#include "pasteboardpatterndetection_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "paste_data.h"
#include "paste_data_record.h"
#include "pasteboard_pattern.h"

namespace OHOS::MiscServices {
namespace {
constexpr uint8_t MAX_PATTERN_COUNT = static_cast<uint8_t>(Pattern::COUNT);
constexpr uint8_t TARGET_SIZE = 2;

std::set<Pattern> CreateFuzzPatterns(FuzzedDataProvider &fdp)
{
    std::set<Pattern> patterns;
    uint8_t count = fdp.ConsumeIntegralInRange<uint8_t>(1, MAX_PATTERN_COUNT);
    for (uint8_t i = 0; i < count; i++) {
        Pattern p = static_cast<Pattern>(fdp.ConsumeIntegral<uint8_t>() % MAX_PATTERN_COUNT);
        patterns.insert(p);
    }
    return patterns;
}

PasteData CreateFuzzPasteData(FuzzedDataProvider &fdp)
{
    PasteData data;
    uint8_t recordCount = fdp.ConsumeIntegralInRange<uint8_t>(1, 4);
    for (uint8_t i = 0; i < recordCount; i++) {
        std::string text = fdp.ConsumeRandomLengthString(256);
        auto record = PasteDataRecord::NewPlainTextRecord(text);
        if (record != nullptr) {
            data.AddRecord(record);
        }
    }
    return data;
}

void DoDetect(FuzzedDataProvider &fdp)
{
    auto patterns = CreateFuzzPatterns(fdp);
    PasteData data = CreateFuzzPasteData(fdp);
    bool hasHTML = fdp.ConsumeBool();
    bool hasPlain = fdp.ConsumeBool();
    PatternDetection::Detect(patterns, data, hasHTML, hasPlain);
}

void DoIsValid(FuzzedDataProvider &fdp)
{
    auto patterns = CreateFuzzPatterns(fdp);
    PatternDetection::IsValid(patterns);
}

using DoFunc = void (*)(FuzzedDataProvider &);
const DoFunc FUNC_LIST[] = {
    DoDetect,
    DoIsValid,
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
