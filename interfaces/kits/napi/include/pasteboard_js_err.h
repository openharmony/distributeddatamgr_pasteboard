//
// Created by qimeng on 2022/9/28.
//

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_JS_ERR_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_JS_ERR_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS::MiscServices {
enum class JSErrorCode : int32_t {
    SUCCESS = 0,
    NO_PERMISSION = 201,
    INVALID_PARAMETERS = 401,
    DEVICE_NOT_SUPPORT = 801,
    OUT_OF_RANGE = 12900001,
    RECORD_EXCEEDS_LIMIT = 12900002,
    OTHER_COPY_OR_PASTE_IN_PROCESSING = 12900003,
    COPY_FORBIDDEN = 12900004,
};

void ThrowErr(napi_env env, JSErrorCode errCode, const std::string &errMsg);
} // namespace OHOS::MiscServices

#endif //DISTRIBUTEDDATAMGR_PASTEBOARD_JS_ERR_H
