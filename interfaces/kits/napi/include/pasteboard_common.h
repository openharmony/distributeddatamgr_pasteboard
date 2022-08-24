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
namespace OHOS {
namespace MiscServicesNapi {
napi_value GetCallbackErrorValue(napi_env env, int32_t errorCode);
void SetCallback(const napi_env &env, const napi_ref &callbackIn, const napi_value *result);
napi_value NapiGetNull(napi_env env);
napi_value CreateNapiNumber(napi_env env, int32_t num);
napi_value CreateNapiString(napi_env env, std::string str);
}  // namespace MiscServicesNapi
}  // namespace OHOS
#endif