/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef PASTEBOARD_PROGRESS_SIGNAL_NAPI
#define PASTEBOARD_PROGRESS_SIGNAL_NAPI

#include "common/block_object.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "pixel_map_napi.h"

namespace OHOS {
namespace MiscServicesNapi {
class ProgressSignalNapi {
public:
    static napi_value ProgressSignalNapiInit(napi_env env, napi_value exports);
    static napi_value New(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);
    ProgressSignalNapi();
    ~ProgressSignalNapi();
private:
    static napi_value Cancel(napi_env env, napi_callback_info info);
    napi_env env_;
};
} // namespace MiscServicesNapi
} // namespace OHOS

#endif // PASTEBOARD_PROGRESS_SIGNAL_NAPI