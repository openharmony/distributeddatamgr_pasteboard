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
#ifndef N_NAPI_PASTEDATA_H
#define N_NAPI_PASTEDATA_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "paste_data.h"
#include "paste_data_record.h"
#include "pasteboard_client.h"
#include "uri.h"

namespace OHOS {
namespace MiscServicesNapi {
class PasteDataNapi {
public:
    PasteDataNapi();
    ~PasteDataNapi();
    static napi_value PasteDataInit(napi_env env, napi_value exports);
    static napi_value New(napi_env env, napi_callback_info info);
    static napi_status NewInstance(napi_env env, napi_value *instance);
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);
    static napi_value GetSystemPasteboard(napi_env env, napi_callback_info info);
    std::shared_ptr<MiscServices::PasteData> value_;

private:
    static napi_value AddHtmlRecord(napi_env env, napi_callback_info info);
    static napi_value AddTextRecord(napi_env env, napi_callback_info info);
    static napi_value AddUriRecord(napi_env env, napi_callback_info info);
    static napi_value GetPrimaryHtml(napi_env env, napi_callback_info info);
    static napi_value GetPrimaryText(napi_env env, napi_callback_info info);
    static napi_value GetPrimaryUri(napi_env env, napi_callback_info info);

    static napi_value HasMimeType(napi_env env, napi_callback_info info);
    static napi_value RemoveRecordAt(napi_env env, napi_callback_info info);
    static napi_value GetPrimaryMimeType(napi_env env, napi_callback_info info);
    static napi_value GetRecordCount(napi_env env, napi_callback_info info);
    static napi_value GetTag(napi_env env, napi_callback_info info);
    static napi_value GetMimeTypes(napi_env env, napi_callback_info info);
    static napi_value AddRecord(napi_env env, napi_callback_info info);
    static napi_value ReplaceRecordAt(napi_env env, napi_callback_info info);
    static napi_value AddWantRecord(napi_env env, napi_callback_info info);
    static napi_value GetPrimaryWant(napi_env env, napi_callback_info info);
    napi_env env_;
    napi_ref wrapper_;
};
} // MiscServicesNapi
} // OHOS
#endif