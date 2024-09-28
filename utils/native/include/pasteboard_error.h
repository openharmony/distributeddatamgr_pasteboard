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
constexpr int DIST_DATA_MGR_SYS_ID = 0xd;
constexpr int PASTEBOARD_ID = 8;
enum PasteboardModule {
    PASTEBOARD_MODULE_SERVICE_ID = 0x06,
};

enum class PasteboardError : int32_t {
    E_OK = (DIST_DATA_MGR_SYS_ID << 21) | (PASTEBOARD_ID << 16),
    INVALID_RETURN_VALUE_ERROR,
    INVALID_PARAM_ERROR,
    SERIALIZATION_ERROR,
    DESERIALIZATION_ERROR,
    OBTAIN_SERVER_SA_ERROR,
    OTHER_ERROR,
    CROSS_BORDER_ERROR,
    PERMISSION_VERIFICATION_ERROR,
    PARAM_ERROR,
    TIMEOUT_ERROR,
    CANCELED,
    EXCEEDING_LIMIT_EXCEPTION,
    TASK_PROCESSING,
    PROHIBIT_COPY,
    UNKNOWN_ERROR,
    BACKUP_EXCEPTION,
    REMOTE_EXCEPTION,
    INVALID_DATA_ERROR,
    NO_DATA_ERROR,
    INVALID_USERID_ERROR,
    REMOTE_TASK_ERROR,
    INVALID_EVENT_ERROR,
    GET_REMOTE_DATA_ERROR,
    SEND_BROADCAST_ERROR,
    SYNC_DATA_ERROR,
    URI_GRANT_ERROR,
    DP_LOAD_SERVICE_ERROR,
    INVALID_OPTION_ERROR,
    INVALID_OPERATION_ERROR,
    BUTT_ERROR,
    NO_TRUST_DEVICE_ERROR,
    NO_USER_DATA_ERROR,
    DATA_EXPIRED_ERROR,
    CREATE_DATASHARE_SERVICE_ERROR,
    QUERY_SETTING_NO_DATA_ERROR,
    GET_LOCAL_DEVICE_ID_ERROR,
    LOCAL_SWITCH_NOT_TURNED_ON,
};

const std::map<PasteboardError, const char *> PasteboardErrorMap = {
    { PasteboardError::E_OK, "E_OK" },
    { PasteboardError::INVALID_RETURN_VALUE_ERROR, "INVALID_RETURN_VALUE_ERROR" },
    { PasteboardError::INVALID_PARAM_ERROR, "INVALID_PARAM_ERROR" },
    { PasteboardError::SERIALIZATION_ERROR, "SERIALIZATION_ERROR" },
    { PasteboardError::DESERIALIZATION_ERROR, "DESERIALIZATION_ERROR" },
    { PasteboardError::OBTAIN_SERVER_SA_ERROR, "OBTAIN_SERVER_SA_ERROR" },
    { PasteboardError::OTHER_ERROR, "OTHER_ERROR" },
    { PasteboardError::CROSS_BORDER_ERROR, "CROSS_BORDER_ERROR" },
    { PasteboardError::PERMISSION_VERIFICATION_ERROR, "PERMISSION_VERIFICATION_ERROR" },
    { PasteboardError::PARAM_ERROR, "PARAM_ERROR" },
    { PasteboardError::TIMEOUT_ERROR, "TIMEOUT_ERROR" },
    { PasteboardError::CANCELED, "CANCELED" },
    { PasteboardError::EXCEEDING_LIMIT_EXCEPTION, "EXCEEDING_LIMIT_EXCEPTION" },
    { PasteboardError::TASK_PROCESSING, "TASK_PROCESSING" },
    { PasteboardError::PROHIBIT_COPY, "PROHIBIT_COPY" },
    { PasteboardError::UNKNOWN_ERROR, "UNKNOWN_ERROR" },
    { PasteboardError::BACKUP_EXCEPTION, "BACKUP_EXCEPTION" },
    { PasteboardError::REMOTE_EXCEPTION, "REMOTE_EXCEPTION" },
    { PasteboardError::INVALID_DATA_ERROR, "INVALID_DATA_ERROR" },
    { PasteboardError::NO_DATA_ERROR, "NO_DATA_ERROR" },
    { PasteboardError::INVALID_USERID_ERROR, "INVALID_USERID_ERROR" },
    { PasteboardError::REMOTE_TASK_ERROR, "REMOTE_TASK_ERROR" },
    { PasteboardError::INVALID_EVENT_ERROR, "INVALID_EVENT_ERROR" },
    { PasteboardError::GET_REMOTE_DATA_ERROR, "GET_REMOTE_DATA_ERROR" },
    { PasteboardError::SEND_BROADCAST_ERROR, "SEND_BROADCAST_ERROR" },
    { PasteboardError::SYNC_DATA_ERROR, "SYNC_DATA_ERROR" },
    { PasteboardError::URI_GRANT_ERROR, "URI_GRANT_ERROR" },
    { PasteboardError::DP_LOAD_SERVICE_ERROR, "DP_LOAD_SERVICE_ERROR" },
    { PasteboardError::INVALID_OPTION_ERROR, "INVALID_OPTION_ERROR" },
    { PasteboardError::INVALID_OPERATION_ERROR, "INVALID_OPERATION_ERROR" },
    { PasteboardError::BUTT_ERROR, "BUTT_ERROR" },
    { PasteboardError::NO_TRUST_DEVICE_ERROR, "NO_TRUST_DEVICE_ERROR" },
    { PasteboardError::NO_USER_DATA_ERROR, "NO_USER_DATA_ERROR" },
    { PasteboardError::DATA_EXPIRED_ERROR, "DATA_EXPIRED_ERROR" },
    { PasteboardError::CREATE_DATASHARE_SERVICE_ERROR, "CREATE_DATASHARE_SERVICE_ERROR" },
    { PasteboardError::QUERY_SETTING_NO_DATA_ERROR, "QUERY_SETTING_NO_DATA_ERROR" },
    { PasteboardError::GET_LOCAL_DEVICE_ID_ERROR, "GET_LOCAL_DEVICE_ID_ERROR" },
    { PasteboardError::LOCAL_SWITCH_NOT_TURNED_ON, "LOCAL_SWITCH_NOT_TURNED_ON" },
};

} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_ERROR_H