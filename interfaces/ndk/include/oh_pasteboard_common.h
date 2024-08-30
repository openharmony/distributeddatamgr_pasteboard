/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef OH_PASTEBOARD_COMMON_H
#define OH_PASTEBOARD_COMMON_H
#include <cstdint>
#include <map>
#include "pasteboard_observer.h"
#include "oh_pasteboard.h"
#include "pasteboard_error.h"
#include "oh_pasteboard_err_code.h"

namespace OHOS {
namespace MiscServices {
const std::map<PasteboardError, PASTEBOARD_ErrCode> errCodeMap = {
    {PasteboardError::E_NO_PERMISSION, ERR_PERMISSION_ERROR},
    {PasteboardError::E_INVALID_PARAMETERS, ERR_INVALID_PARAMETER},
    {PasteboardError::E_IS_BEGING_PROCESSED, ERR_BUSY},
};

class PasteboardObserverCapiImpl;
}
}

enum PasteboardNdkStructId : std::int64_t {
    SUBSCRIBER_STRUCT_ID = 1002950,
    PASTEBOARD_STRUCT_ID,
};

struct OH_Pasteboard {
    const int64_t cid = PASTEBOARD_STRUCT_ID;
    std::mutex mutex;
    std::map<const OH_PasteboardObserver*, OHOS::sptr<OHOS::MiscServices::PasteboardObserverCapiImpl>> observers_;
};

/** @} */
#endif