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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_PARCEL_UTIL_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_PARCEL_UTIL_H

#include "api/visibility.h"
#include "parcel.h"
#include "tlv_object.h"
namespace OHOS::MiscServices {

class ParcelUtil {
public:
    // parcelable to buffer
    API_EXPORT static RawMem Parcelable2Raw(const Parcelable *value);

    // buffer to parcelable
    template<typename ParcelableType> API_EXPORT static ParcelableType *Raw2Parcelable(const RawMem &rawMem)
    {
        if (rawMem.buffer == 0 || rawMem.bufferLen == 0) {
            return nullptr;
        }
        Parcel parcel(nullptr);
        bool ret = parcel.ParseFrom(rawMem.buffer, rawMem.bufferLen);
        if (!ret) {
            return nullptr;
        }
        return ParcelableType::Unmarshalling(parcel);
    }
};
} // namespace OHOS::MiscServices
#endif //DISTRIBUTEDDATAMGR_PASTEBOARD_PARCEL_UTIL_H
