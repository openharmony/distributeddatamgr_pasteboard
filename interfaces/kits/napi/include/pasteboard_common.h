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
#ifndef N_NAPI_PASTEBOARD_COMMON_H
#define N_NAPI_PASTEBOARD_COMMON_H

#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_want.h"
#include "paste_data_record.h"
#include "pasteboard_js_err.h"
#include "pasteboard_pattern.h"
#include "pixel_map_napi.h"
#include "tlv_object.h"
#include "napi_data_utils.h"
namespace OHOS {
namespace MiscServicesNapi {
napi_value GetCallbackErrorValue(napi_env env, int32_t errorCode);
void SetCallback(const napi_env &env, const napi_ref &callbackIn, const napi_value *result);
napi_value NapiGetNull(napi_env env);
napi_value CreateNapiNumber(napi_env env, int32_t num);
napi_value CreateNapiString(napi_env env, std::string str);
bool GetValue(napi_env env, napi_value in, std::string &out);
bool GetValue(napi_env env, napi_value in, std::set<MiscServices::Pattern> &out);
napi_status SetValue(napi_env env, std::set<MiscServices::Pattern> &in, napi_value &result);
bool CheckArgsType(napi_env env, napi_value in, napi_valuetype expectedType, const char *message);
bool CheckExpression(napi_env env, bool expression, MiscServices::JSErrorCode errCode, const char *message);
bool CheckArgs(napi_env env, napi_value *argv, size_t argc, std::string &mimeType);
bool CheckArgsMimeType(napi_env env, napi_value in, std::string &mimeType);
bool CheckArgsArray(napi_env env, napi_value in, std::vector<std::string> &mimeTypes);
bool CheckArgsFunc(napi_env env, napi_value in, napi_ref &provider);
bool CheckArgsVector(napi_env env, napi_value in,
    std::shared_ptr<std::vector<std::pair<std::string, std::shared_ptr<MiscServices::EntryValue>>>> result);
napi_status ConvertEntryValue(napi_env env, napi_value *result, std::string &udtType,
    std::shared_ptr<MiscServices::PasteDataEntry> value);
bool GetNativeValue(napi_env env, std::string type, napi_value valueNapi, MiscServices::EntryValue &value);
} // namespace MiscServicesNapi
} // namespace OHOS
#endif