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

#include "pasteboardabilitymanager_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "pasteboard_ability_manager.h"
#include "want.h"

namespace OHOS::MiscServices {
namespace {
constexpr size_t TARGET_SIZE = 2;

void DoCheckUIExtensionIsFocused(FuzzedDataProvider &fdp)
{
    uint32_t tokenId = fdp.ConsumeIntegral<uint32_t>();
    bool isFocused = false;
    PasteboardAbilityManager::CheckUIExtensionIsFocused(tokenId, isFocused);
}

void DoStartAbility(FuzzedDataProvider &fdp)
{
    OHOS::AAFwk::Want want;
    std::string element = fdp.ConsumeRandomLengthString(256);
    std::string action = fdp.ConsumeRandomLengthString(128);
    std::string uri = fdp.ConsumeRandomLengthString(256);
    want.SetElementName(element, element, element);
    want.SetAction(action);
    want.SetUri(uri);
    PasteboardAbilityManager::StartAbility(want);
}

using DoFunc = void (*)(FuzzedDataProvider &);
const DoFunc FUNC_LIST[] = {
    DoCheckUIExtensionIsFocused,
    DoStartAbility,
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
