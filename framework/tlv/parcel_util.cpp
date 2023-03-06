/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "parcel_util.h"
#include "pasteboard_hilog.h"

namespace OHOS::MiscServices {
RawMem ParcelUtil::Parcelable2Raw(const Parcelable *value)
{
    RawMem rawMem{};
    if (value == nullptr) {
        return rawMem;
    }
    rawMem.parcel = std::make_shared<Parcel>(nullptr);
    bool ret = value->Marshalling(*rawMem.parcel);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Marshalling failed.");
        return rawMem;
    }
    rawMem.buffer = rawMem.parcel->GetData();
    rawMem.bufferLen = rawMem.parcel->GetDataSize();
    return rawMem;
}
bool ParcelUtil::Raw2Parcel(const RawMem &rawMem, Parcel &parcel)
{
    if (rawMem.buffer == 0 || rawMem.bufferLen == 0) {
        return false;
    }
    auto *temp = malloc(rawMem.bufferLen); // free by Parcel!
    if (temp == nullptr) {
        return false;
    }
    auto err = memcpy_s(temp, rawMem.bufferLen, reinterpret_cast<const void *>(rawMem.buffer), rawMem.bufferLen);
    if (err != EOK) {
        free(temp);
        return false;
    }
    bool ret = parcel.ParseFrom(reinterpret_cast<uintptr_t>(temp), rawMem.bufferLen);
    if (!ret) {
        free(temp);
        return false;
    }
    return true;
}
} // namespace OHOS::MiscServices