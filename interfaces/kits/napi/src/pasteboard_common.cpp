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
#include "pasteboard_common.h"

namespace OHOS {
namespace MiscServicesNapi {
const size_t ARGC_TYPE_SET2 = 2;
constexpr size_t STR_TAIL_LENGTH = 1;

napi_value GetCallbackErrorValue(napi_env env, int32_t errorCode)
{
    napi_value result = nullptr;
    napi_value eCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errorCode, &eCode));
    NAPI_CALL(env, napi_create_object(env, &result));
    NAPI_CALL(env, napi_set_named_property(env, result, "code", eCode));
    return result;
}

void SetCallback(const napi_env &env, const napi_ref &callbackIn, const napi_value *results)
{
    if (results == nullptr) {
        return;
    }
    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_call_function(env, nullptr, callback, ARGC_TYPE_SET2, results, &resultout);
}

napi_value NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value CreateNapiNumber(napi_env env, int32_t num)
{
    napi_value value = nullptr;
    napi_create_int32(env, num, &value);
    return value;
}

napi_value CreateNapiString(napi_env env, std::string str)
{
    napi_value value = nullptr;
    napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &value);
    return value;
}

bool GetValue(napi_env env, napi_value in, std::string& out)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, in, &type), false);
    NAPI_ASSERT_BASE(env, type == napi_string, "Wrong argument type. String expected.", false);

    size_t len = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, in, nullptr, 0, &len), false);
    if (len <= 0) {
        return false;
    }

    size_t length = 0;
    out.resize(len + STR_TAIL_LENGTH, 0);
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, in, out.data(), len + STR_TAIL_LENGTH, &length), false);

    return true;
}
} // namespace MiscServicesNapi
} // namespace OHOS