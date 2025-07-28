/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "pasteboarddata_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "paste_data.h"

namespace {
using namespace OHOS::MiscServices;

void TestPasteData(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    PasteData pasteData;
    {
        std::vector<uint8_t> buffer = fdp.ConsumeRemainingBytes<uint8_t>();
        pasteData.Decode(buffer);
    }
    {
        std::vector<uint8_t> buffer;
        pasteData.Encode(buffer, false);
    }
    {
        std::vector<uint8_t> buffer;
        pasteData.Encode(buffer, true);
    }
}

void TestPasteDataRecord(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    PasteDataRecord record;
    {
        std::vector<uint8_t> buffer = fdp.ConsumeRemainingBytes<uint8_t>();
        record.Decode(buffer);
    }
    {
        std::vector<uint8_t> buffer;
        record.Encode(buffer, false);
    }
    {
        std::vector<uint8_t> buffer;
        record.Encode(buffer, true);
    }
}

void TestPasteDataEntry(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    PasteDataEntry entry;
    {
        std::vector<uint8_t> buffer = fdp.ConsumeRemainingBytes<uint8_t>();
        entry.Decode(buffer);
    }
    {
        std::vector<uint8_t> buffer;
        entry.Encode(buffer, false);
    }
    {
        std::vector<uint8_t> buffer;
        entry.Encode(buffer, true);
    }
}
} // anonymous namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    TestPasteData(data, size);
    TestPasteDataRecord(data, size);
    TestPasteDataEntry(data, size);
    return 0;
}
