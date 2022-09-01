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
#include "parcel.h"
namespace OHOS::MiscServices {
class ParcelUtil {
public:
    // parcelable to buffer
    static bool GetRawData(const Parcelable *value, uintptr_t &data, size_t &size);

    // buffer to parcelable
    template<typename Parcelable> bool SetRawData(uintptr_t data, size_t size, Parcelable *&value)
    {
        Parcel parcel(nullptr);
        bool ret = parcel.ParseFrom(data, size);
        value = parcel.ReadParcelable<Parcelable>();
        return ret && value != nullptr;
    }
};
} // namespace OHOS::MiscServices
#endif //DISTRIBUTEDDATAMGR_PASTEBOARD_PARCEL_UTIL_H
