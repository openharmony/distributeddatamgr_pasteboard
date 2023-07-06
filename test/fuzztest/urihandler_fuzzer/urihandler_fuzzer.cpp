/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "urihandler_fuzzer.h"

#include "copy_uri_handler.h"

using namespace OHOS::MiscServices;

namespace OHOS {
constexpr size_t THRESHOLD = 10;
constexpr size_t OFFSET = 4;
uint32_t ConvertToUint32(const uint8_t *ptr)
{
    if (ptr == nullptr) {
        return 0;
    }
    uint32_t bigVar = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
    return bigVar;
}

bool FuzzUriToFd(const uint8_t *rawData, size_t size)
{
    uint32_t value = ConvertToUint32(rawData);
    rawData = rawData + OFFSET;
    size = size - OFFSET;
    std::string uri(reinterpret_cast<const char *>(rawData), size);
    bool isClient = value % 2;
    CopyUriHandler uriHandler;
    uriHandler.ToFd(uri, isClient);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::THRESHOLD) {
        return 0;
    }
    /* Run your code on data */
    OHOS::FuzzUriToFd(data, size);
    return 0;
}
