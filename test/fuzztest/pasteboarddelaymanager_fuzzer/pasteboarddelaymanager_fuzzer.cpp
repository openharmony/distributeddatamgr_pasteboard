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

#include "pasteboarddelaymanager_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "paste_data.h"
#include "paste_data_record.h"
#include "pasteboard_delay_manager.h"

namespace OHOS::MiscServices {
namespace {
constexpr size_t TARGET_SIZE = 2;

std::shared_ptr<PasteDataRecord> CreateFuzzDelayRecord(FuzzedDataProvider &fdp)
{
    auto record = std::make_shared<PasteDataRecord>();
    size_t entryCount = fdp.ConsumeIntegralInRange<uint8_t>(1, 4);
    for (size_t i = 0; i < entryCount; i++) {
        std::string utdId = fdp.ConsumeRandomLengthString(32);
        auto entry = std::make_shared<PasteDataEntry>(utdId, std::monostate{});
        record->AddEntry(utdId, entry);
    }
    record->SetDelayRecordFlag(true);
    return record;
}

PasteData CreateFuzzPasteData(FuzzedDataProvider &fdp)
{
    PasteData data;
    size_t recordCount = fdp.ConsumeIntegralInRange<uint8_t>(1, 5);
    for (size_t i = 0; i < recordCount; i++) {
        bool isDelay = fdp.ConsumeBool();
        if (isDelay) {
            auto record = CreateFuzzDelayRecord(fdp);
            data.AddRecord(record);
        } else {
            std::string text = fdp.ConsumeRandomLengthString(64);
            auto record = PasteDataRecord::NewPlainTextRecord(text);
            data.AddRecord(record);
        }
    }
    return data;
}

void DoGetAllDelayEntryInfo(FuzzedDataProvider &fdp)
{
    PasteData data = CreateFuzzPasteData(fdp);
    DelayManager::GetAllDelayEntryInfo(data);
}

void DoGetPrimaryDelayEntryInfo(FuzzedDataProvider &fdp)
{
    PasteData data = CreateFuzzPasteData(fdp);
    DelayManager::GetPrimaryDelayEntryInfo(data);
}

using DoFunc = void (*)(FuzzedDataProvider &);
const DoFunc FUNC_LIST[] = {
    DoGetAllDelayEntryInfo,
    DoGetPrimaryDelayEntryInfo,
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
