/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#include "napi_common_want.h"
#include "paste_data_record.h"
#include "pasteboard_hilog.h"
#include "pasteboard_js_err.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace MiscServicesNapi {
using namespace OHOS::MiscServices;
const size_t ARGC_TYPE_SET2 = 2;
constexpr size_t STR_TAIL_LENGTH = 1;
constexpr int32_t MIMETYPE_MAX_SIZE = 1024;

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

/* napi_value <-> std::string */
bool GetValue(napi_env env, napi_value in, std::string &out)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, in, &type), false);
    NAPI_ASSERT_BASE(env, type == napi_string, "Wrong argument type. String expected.", false);

    size_t len = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, in, nullptr, 0, &len), false);
    if (len < 0) {
        return false;
    }

    size_t length = 0;
    out.resize(len + STR_TAIL_LENGTH, 0);
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, in, out.data(), len + STR_TAIL_LENGTH, &length), false);
    out.resize(len);

    return true;
}

/* napi_value <-> std::set<Pattern> */
bool GetValue(napi_env env, napi_value in, std::set<MiscServices::Pattern> &out)
{
    bool isArray = false;
    NAPI_CALL_BASE(env, napi_is_array(env, in, &isArray), false);
    if (!isArray) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Wrong argument type. pattern/uint32 array expected.");
        return false;
    }
    
    uint32_t len = 0;
    napi_status status = napi_get_array_length(env, in, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "napi_get_array_length status = %{public}d", status);
        return false;
    }

    for (uint32_t i = 0; i < len; i++) {
        napi_value element;
        napi_status status = napi_get_element(env, in, i, &element);
        if (status != napi_ok) {
            PASTEBOARD_HILOGE(
                PASTEBOARD_MODULE_JS_NAPI, "napi_get_element%{public}d err status = %{public}d", i, status);
            return false;
        }
        uint32_t pattern;
        status = napi_get_value_uint32(env, element, &pattern);
        if (status != napi_ok) {
            PASTEBOARD_HILOGE(
                PASTEBOARD_MODULE_JS_NAPI, "napi_get_value_uint32 err status = %{public}d", status);
            return false;
        }
        if (pattern >= Pattern::PatternCount) {
            PASTEBOARD_HILOGE(
                PASTEBOARD_MODULE_JS_NAPI, "Unsurportted pattern value: %{public}d", pattern);
            return false; 
        }
        out.insert(static_cast<Pattern>(pattern));
    }
    return true;
}

/* napi_value <-> std::set<Pattern> */
napi_status SetValue(napi_env env, std::set<Pattern> &in, napi_value &result)
{
    napi_status status = napi_create_array_with_length(env, in.size(), &result);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "napi_create_array_with_length error status = %{public}d", status);
        return status;
    }
    int i = 0;
    for (auto &pattern : in) {
        napi_value element;
        status = napi_create_uint32(env, static_cast<uint32_t>(pattern), &element);
        if (status != napi_ok) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "napi_create_uint32 error status = %{public}d", status);
            return status;
        }
        status = napi_set_element(env, result, i, element);
        if (status != napi_ok) {
            PASTEBOARD_HILOGE(
                PASTEBOARD_MODULE_JS_NAPI, "napi_set_element %{public}d err status = %{public}d", i, status);
            return status;
        }
        ++i;
    }
    return status;
}

bool CheckArgsType(napi_env env, napi_value in, napi_valuetype expectedType, const char *message)
{
    napi_valuetype type = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, in, &type), false);
    int32_t errCode = static_cast<int32_t>(JSErrorCode::INVALID_PARAMETERS);
    if (type != expectedType) {
        napi_throw_error(env, std::to_string(errCode).c_str(), message);
        return false;
    }
    return true;
}

bool CheckExpression(napi_env env, bool flag, MiscServices::JSErrorCode errCode, const char *message)
{
    if (!flag) {
        NAPI_CALL_BASE(
            env, napi_throw_error(env, std::to_string(static_cast<int32_t>(errCode)).c_str(), message), false);
        return false;
    }
    return true;
}

// Check Parameters of CreateData, CreateRecord and AddRecord
bool CheckArgs(napi_env env, napi_value *argv, size_t argc, std::string &mimeType)
{
    // 2: CreateRecord, CreateRecord and AddRecord has 2 args.
    if (!CheckExpression(env, argc >= ARGC_TYPE_SET2, JSErrorCode::INVALID_PARAMETERS,
        "Parameter error. The number of arguments cannot be less than two.") ||
        !CheckArgsType(env, argv[0], napi_string, "Parameter error. The type of mimeType must be string.")) {
        return false;
    }

    bool ret = GetValue(env, argv[0], mimeType);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "GetValue failed");
        return false;
    }
    if (!CheckExpression(
        env, mimeType != "", JSErrorCode::INVALID_PARAMETERS, "Parameter error. mimeType cannot be empty.") ||
        !CheckExpression(env, mimeType.size() <= MIMETYPE_MAX_SIZE, JSErrorCode::INVALID_PARAMETERS,
        "Parameter error. The length of mimeType cannot be greater than 1024 bytes.")) {
        return false;
    }

    if (mimeType == MIMETYPE_TEXT_URI || mimeType == MIMETYPE_TEXT_PLAIN || mimeType == MIMETYPE_TEXT_HTML) {
        if (!CheckArgsType(env, argv[1], napi_string, "Parameter error. The type of mimeType must be string.")) {
            return false;
        }
    } else if (mimeType == MIMETYPE_PIXELMAP) {
        if (!CheckExpression(env, Media::PixelMapNapi::GetPixelMap(env, argv[1]) != nullptr,
            JSErrorCode::INVALID_PARAMETERS, "Parameter error. Actual mimeType is not mimetype_pixelmap.")) {
            return false;
        }
    } else if (mimeType == MIMETYPE_TEXT_WANT) {
        AAFwk::Want want;
        ret = OHOS::AppExecFwk::UnwrapWant(env, argv[1], want);
        if (!CheckExpression(env, ret, JSErrorCode::INVALID_PARAMETERS,
            "Parameter error. Actual mimeType is not mimetype_text_want.")) {
            return false;
        }
    } else {
        bool isArrayBuffer = false;
        NAPI_CALL_BASE(env, napi_is_arraybuffer(env, argv[1], &isArrayBuffer), false);
        if (!CheckExpression(env, isArrayBuffer, JSErrorCode::INVALID_PARAMETERS,
            "Parameter error. The mimeType is not an arraybuffer.")) {
            return false;
        }
    }
    return true;
}
} // namespace MiscServicesNapi
} // namespace OHOS