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
#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_OBJECT_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_OBJECT_H
#include <cstdint>
#include <vector>

#include "api/visibility.h"

namespace OHOS {
namespace MiscServices {
struct API_EXPORT TLVObject {
public:
    virtual ~TLVObject() = default;
    virtual bool Encode(std::vector<uint8_t> &buffer);
    virtual bool Decode(const std::vector<uint8_t> &buffer);
};
} // namespace MiscServices
} // namespace OHOS
#endif //DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_OBJECT_H
