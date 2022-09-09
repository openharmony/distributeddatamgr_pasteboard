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
#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_URI_INFO_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_URI_INFO_H
#include "message_parcel.h"
#include "tlv_object.h"
namespace OHOS::MiscServices {
struct UriInfo : public TLVObject {
    ~UriInfo();
    bool Encode(std::vector<std::uint8_t> &buffer) override;
    bool Decode(const std::vector<std::uint8_t> &buffer) override;
    size_t Count() override;

    bool Marshalling(MessageParcel &parcel);
    bool Unmarshalling(MessageParcel &parcel);
    std::string ToShareUri() const;
    std::string ToClientUri() const;
    bool OpenPath();

    enum TAG_INFO : uint16_t {
        TAG_URI = TAG_BUFF + 1,
    };
    std::string uri_;
    int32_t fd_ = -1;
};
} // namespace OHOS::MiscServices
#endif //DISTRIBUTEDDATAMGR_PASTEBOARD_URI_INFO_H
