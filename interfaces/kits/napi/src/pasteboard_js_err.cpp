//
// Created by qimeng on 2022/9/28.
//
#include "pasteboard_js_err.h"
#include <string>

namespace OHOS::MiscServices {
void ThrowErr(napi_env env, JSErrorCode errCode, const std::string &errMsg)
{
    napi_value code = nullptr;
    std::string errNum = std::to_string(static_cast<int32_t>(errCode));
    napi_create_string_utf8(env, errNum.c_str(), errNum.length(), &code);
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), errMsg.length(), &message);
    napi_value error = nullptr;
    napi_create_error(env, code, message, &error);
    napi_throw(env, error);
}
} // namespace OHOS::MiscServices