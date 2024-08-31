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
#include "paste_uri_handler.h"

using namespace OHOS::MiscServices;

namespace OHOS {

template<class T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

bool FuzzUriToFd(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < (sizeof(uint32_t) + sizeof(uint32_t))) {
        return true;
    }
    int pos = 0;
    uint32_t value = TypeCast<uint32_t>(rawData, &pos);
    std::string uri(reinterpret_cast<const char *>(rawData + pos), size - pos);
    bool isClient = value % 2;
    CopyUriHandler uriHandler;
    uriHandler.ToFd(uri, isClient);
    return true;
}

bool FuzzCopyUriHandlerToUri(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    int32_t fd = TypeCast<int32_t>(rawData, nullptr);
    CopyUriHandler uriHandler;
    (void)uriHandler.ToUri(fd);
    return true;
}

bool FuzzPasteUriHandlerToUri(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < sizeof(int32_t)) {
        return true;
    }
    int32_t fd = TypeCast<int32_t>(rawData, nullptr);
    PasteUriHandler pasteUriHandler;
    (void)pasteUriHandler.ToUri(fd);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::FuzzUriToFd(data, size);
    OHOS::FuzzCopyUriHandlerToUri(data, size);
    OHOS::FuzzPasteUriHandlerToUri(data, size);
    return 0;
}
