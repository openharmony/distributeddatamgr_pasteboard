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

#include "pasteboardconvertutils_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "convert_utils.h"
#include "paste_data.h"
#include "paste_data_record.h"

namespace OHOS::MiscServices {
namespace {
constexpr uint8_t TARGET_SIZE = 2;

PasteData CreateFuzzPasteData(FuzzedDataProvider &fdp)
{
    PasteData data;
    uint8_t recordCount = fdp.ConsumeIntegralInRange<uint8_t>(1, 5);
    for (uint8_t i = 0; i < recordCount; i++) {
        uint8_t recordType = fdp.ConsumeIntegral<uint8_t>() % 3;
        std::shared_ptr<PasteDataRecord> record;
        if (recordType == 0) {
            std::string text = fdp.ConsumeRandomLengthString(128);
            record = PasteDataRecord::NewPlainTextRecord(text);
        } else if (recordType == 1) {
            std::string html = fdp.ConsumeRandomLengthString(256);
            record = PasteDataRecord::NewHtmlRecord(html);
        } else {
            std::string uri = fdp.ConsumeRandomLengthString(128);
            OHOS::Uri uriObj(uri);
            record = PasteDataRecord::NewUriRecord(uriObj);
        }
        if (record != nullptr) {
            data.AddRecord(record);
        }
    }
    return data;
}

void DoConvertPasteDataToUnifiedData(FuzzedDataProvider &fdp)
{
    PasteData data = CreateFuzzPasteData(fdp);
    auto result = ConvertUtils::Convert(data);
    (void)result;
}

void DoConvertRecords(FuzzedDataProvider &fdp)
{
    PasteData data = CreateFuzzPasteData(fdp);
    auto records = data.AllRecords();
    auto result = ConvertUtils::Convert(records);
    (void)result;
}

using DoFunc = void (*)(FuzzedDataProvider &);
const DoFunc FUNC_LIST[] = {
    DoConvertPasteDataToUnifiedData,
    DoConvertRecords,
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
