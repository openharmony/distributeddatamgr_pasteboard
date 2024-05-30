/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef PASTEBOARD_ERROR_H
#define PASTEBOARD_ERROR_H

#include "errors.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
enum PasteboardModule {
    PASTEBOARD_MODULE_SERVICE_ID = 0x06,
};

// Pasteboard error offset, used only in this file.
constexpr ErrCode PASTEBOARD_ERR_OFFSET = ErrCodeOffset(SUBSYS_SMALLSERVICES, PASTEBOARD_MODULE_SERVICE_ID);

enum class PasteboardError : int32_t {
    E_OK = PASTEBOARD_ERR_OFFSET,
    E_INVALID_VALUE,
    E_INVALID_OPTION,
    E_WRITE_PARCEL_ERROR,
    E_READ_PARCEL_ERROR,
    E_SA_DIED,
    E_ERROR,
    E_OUT_OF_RANGE,
    E_NO_PERMISSION,
    E_INVALID_PARAMETERS,
    E_TIMEOUT,
    E_CANCELED,
    E_EXCEEDS_LIMIT,
    E_IS_BEGING_PROCESSED,
    E_COPY_FORBIDDEN,
    E_UNKNOWN,
    E_BUTT,
    E_REMOTE,
    E_INVALID_OPERATION,
};

const std::map<PasteboardError, const char *> PasteboardErrorMap = {
    {PasteboardError::E_OK, "E_OK"},
    {PasteboardError::E_INVALID_VALUE, "E_INVALID_VALUE"},
    {PasteboardError::E_INVALID_OPTION, "E_INVALID_OPTION"},
    {PasteboardError::E_WRITE_PARCEL_ERROR, "E_WRITE_PARCEL_ERROR"},
    {PasteboardError::E_READ_PARCEL_ERROR, "E_READ_PARCEL_ERROR"},
    {PasteboardError::E_SA_DIED, "E_SA_DIED"},
    {PasteboardError::E_ERROR, "E_ERROR"},
    {PasteboardError::E_OUT_OF_RANGE, "E_OUT_OF_RANGE"},
    {PasteboardError::E_NO_PERMISSION, "E_NO_PERMISSION"},
    {PasteboardError::E_INVALID_PARAMETERS, "E_INVALID_PARAMETERS"},
    {PasteboardError::E_TIMEOUT, "E_TIMEOUT"},
    {PasteboardError::E_CANCELED, "E_CANCELED"},
    {PasteboardError::E_EXCEEDS_LIMIT, "E_EXCEEDS_LIMIT"},
    {PasteboardError::E_IS_BEGING_PROCESSED, "E_IS_BEGING_PROCESSED"},
    {PasteboardError::E_COPY_FORBIDDEN, "E_COPY_FORBIDDEN"},
    {PasteboardError::E_UNKNOWN, "E_UNKNOWN"},
    {PasteboardError::E_BUTT, "E_BUTT"},
    {PasteboardError::E_REMOTE, "E_REMOTE"},
    {PasteboardError::E_INVALID_OPERATION, "E_INVALID_OPERATION"},
};

} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_ERROR_H