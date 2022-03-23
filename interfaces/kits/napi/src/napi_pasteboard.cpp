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
#include "systempasteboard_napi.h"
#include "pasteboard_common.h"
#include "napi_common.h"
#include "pasteboard_hilog_wreapper.h"

using namespace OHOS::MiscServices;

namespace OHOS {
namespace MiscServicesNapi {
static thread_local napi_ref g_systemPasteboard = nullptr;
const size_t ARGC_TYPE_SET1 = 1;
const size_t ARGC_TYPE_SET2 = 2;
const int32_t STR_DATA_SIZE = 10;
const int32_t STR_DATA_SIZE_MAX = 500 * 1024;
const std::string STRING_UPDATE = "update";

napi_value JScreateHtmlTextRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateHtmlTextRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv[1], &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE_MAX] = {0};
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE_MAX, &strLen));

    napi_value instance = nullptr;
    PasteDataRecordNapi::NewHtmlTextRecordInstance(env, str, instance);

    return instance;
}

napi_value JScreateWantRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateWantRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv[1], &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");
    AAFwk::Want want;
    bool ret = OHOS::AppExecFwk::UnwrapWant(env, argv[0], want);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to unwrap want!");
        return nullptr;
    }

    napi_value instance = nullptr;
    PasteDataRecordNapi::NewWantRecordInstance(env, std::make_shared<Want>(want), instance);

    return instance;
}

napi_value JScreatePlainTextRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreatePlainTextRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv[1], &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE_MAX] = {0};
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE_MAX, &strLen));

    napi_value instance = nullptr;
    PasteDataRecordNapi::NewPlainTextRecordInstance(env, str, instance);

    return instance;
}

napi_value JScreateUriRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateUriRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv[1], &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE_MAX] = {0};
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE_MAX, &strLen));

    napi_value instance = nullptr;
    PasteDataRecordNapi::NewUriRecordInstance(env, str, instance);

    return instance;
}

napi_value JScreateHtmlData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateHtmlData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE_MAX] = {0};
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE_MAX, &strLen));

    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return nullptr;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateHtmlData(str);

    return instance;
}

napi_value JScreateWantData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateWantData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");
    AAFwk::Want want;
    bool ret = OHOS::AppExecFwk::UnwrapWant(env, argv[0], want);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to unwrap want!");
        return nullptr;
    }

    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return nullptr;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateWantData(std::make_shared<Want>(want));

    return instance;
}

napi_value JScreatePlainTextData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreatePlainTextData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE_MAX] = {0};
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE_MAX, &strLen));

    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return nullptr;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreatePlainTextData(str);

    return instance;
}

napi_value JScreateUriData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateUriData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE_MAX] = {0};
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE_MAX, &strLen));

    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return nullptr;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateUriData(OHOS::Uri(str));

    return instance;
}

napi_value JSgetSystemPasteboard(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JSgetSystemPasteboard11 is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    napi_value instance = nullptr;
    napi_status status = SystemPasteboardNapi::NewInstance(env, instance); // 0 arguments
    if (status != napi_ok) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "JSgetSystemPasteboard create instance failed");
        return NapiGetNull(env);
    }

    return instance;
}

napi_value PasteBoardInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createHtmlData", JScreateHtmlData),
        DECLARE_NAPI_FUNCTION("createWantData", JScreateWantData),
        DECLARE_NAPI_FUNCTION("createPlainTextData", JScreatePlainTextData),
        DECLARE_NAPI_FUNCTION("createUriData", JScreateUriData),
        DECLARE_NAPI_FUNCTION("createHtmlTextRecord", JScreateHtmlTextRecord),
        DECLARE_NAPI_FUNCTION("createWantRecord", JScreateWantRecord),
        DECLARE_NAPI_FUNCTION("createPlainTextRecord", JScreatePlainTextRecord),
        DECLARE_NAPI_FUNCTION("createUriRecord", JScreateUriRecord),
        DECLARE_NAPI_FUNCTION("getSystemPasteboard", JSgetSystemPasteboard),
        DECLARE_NAPI_PROPERTY("MAX_RECORD_NUM", CreateNapiNumber(env, 128)),
        DECLARE_NAPI_PROPERTY("MIMETYPE_TEXT_HTML", CreateNapiString(env, MIMETYPE_TEXT_HTML)),
        DECLARE_NAPI_PROPERTY("MIMETYPE_TEXT_WANT", CreateNapiString(env, MIMETYPE_TEXT_WANT)),
        DECLARE_NAPI_PROPERTY("MIMETYPE_TEXT_PLAIN", CreateNapiString(env, MIMETYPE_TEXT_PLAIN)),
        DECLARE_NAPI_PROPERTY("MIMETYPE_TEXT_URI", CreateNapiString(env, MIMETYPE_TEXT_URI))
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}

using AsyncContext = struct AsyncContext {
    napi_env env = nullptr;
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    PasteDataNapi *obj = nullptr;
    int32_t status = 0;
};

napi_ref SystemPasteboardNapi::callback_ = nullptr;
napi_env SystemPasteboardNapi::callbackEnv_;

void SystemPasteboardNapi::CallbackOn(void)
{
    napi_value callback = nullptr;
    napi_get_reference_value(callbackEnv_, callback_, &callback);
    napi_call_function(callbackEnv_, nullptr, callback, 0, nullptr, nullptr);
}

napi_value SystemPasteboardNapi::On(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi on() is called!");
    size_t argc = ARGC_TYPE_SET2;
    napi_value argv[ARGC_TYPE_SET2] = { 0 };
    napi_value thisVar = 0;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    NAPI_ASSERT(env, argc >= ARGC_TYPE_SET2, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE] = {0};
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE, &strLen));
    NAPI_ASSERT(env, strLen == STRING_UPDATE.length(), "error type");

    napi_typeof(env, argv[1], &valueType);
    NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");

    callbackEnv_ = env;
    napi_create_reference(env, argv[1], 1, &callback_);

    PasteboardClient::GetInstance()->AddPasteboardChangedObserver(CallbackOn);
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi on() is end!");
    return result;
}

napi_value SystemPasteboardNapi::Off(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi off () is called!");
    size_t argc = ARGC_TYPE_SET2;
    napi_value argv[ARGC_TYPE_SET2] = { 0 };
    napi_value thisVar = 0;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE] = {0};
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE, &strLen));
    NAPI_ASSERT(env, strLen == STRING_UPDATE.length(), "error type");

    PasteboardClient::GetInstance()->RemovePasteboardChangedObserver(CallbackOn);
    callback_ = nullptr;
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi off () is called!");
    return result;
}

napi_value SystemPasteboardNapi::Clear(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "Clear is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    AsyncContext *asyncContext = new (std::nothrow) AsyncContext {.env = env, .work = nullptr};
    if (!asyncContext) {
        return NapiGetNull(env);
    }

    if (argc >= ARGC_TYPE_SET1) {
        napi_valuetype valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &asyncContext->callbackRef);
    }

    napi_value promise = nullptr;
    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "Clear", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env,
        nullptr,
        resource,
        [](napi_env env, void* data) {
            AsyncContext* asyncContext = (AsyncContext*)data;
            PasteboardClient::GetInstance()->Clear();
            asyncContext->status = 0;
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncContext* asyncContext = (AsyncContext*)data;
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, result);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, result);
                }
            } else {
                SetCallback(env, asyncContext->callbackRef, asyncContext->status, result);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
            asyncContext = nullptr;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);

    return promise;
}

napi_value SystemPasteboardNapi::HasPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "HasPasteData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    AsyncContext *asyncContext = new (std::nothrow) AsyncContext {.env = env, .work = nullptr};
    if (!asyncContext) {
        return NapiGetNull(env);
    }

    if (argc >= ARGC_TYPE_SET1) {
        napi_valuetype valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &asyncContext->callbackRef);
    }

    napi_value promise = nullptr;
    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "HasPasteData", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env,
        nullptr,
        resource,
        [](napi_env env, void* data) {
            AsyncContext* asyncContext = (AsyncContext*)data;
            asyncContext->status = PasteboardClient::GetInstance()->HasPasteData() ? 0 : -1;
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncContext* asyncContext = (AsyncContext*)data;
            napi_value result = nullptr;
            if (!asyncContext->status) {
                napi_get_boolean(env, true, &result);
            } else {
                napi_get_boolean(env, false, &result);
            }
            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, result);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, result);
                }
            } else {
                SetCallback(env, asyncContext->callbackRef, asyncContext->status, result);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
            asyncContext = nullptr;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);

    return promise;
}

napi_value SystemPasteboardNapi::GetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetPasteData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    AsyncContext *asyncContext = new (std::nothrow) AsyncContext {.env = env, .work = nullptr,};
    if (!asyncContext) {
        return NapiGetNull(env);
    }

    if (argc >= ARGC_TYPE_SET1) {
        napi_valuetype valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &asyncContext->callbackRef);
    }

    napi_value promise = nullptr;
    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "GetPasteData", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env,
        nullptr,
        resource,
        [](napi_env env, void *data) { },
        [](napi_env env, napi_status status, void *data) {
            AsyncContext* asyncContext = (AsyncContext*)data;
            napi_value instance = nullptr;
            PasteDataNapi::NewInstance(env, instance);

            asyncContext->status = 0;
            PasteDataNapi *obj = nullptr;
            napi_status ret = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
            if ((ret == napi_ok) || (obj != nullptr)) {
                PasteData pasteData;
                PasteboardClient::GetInstance()->GetPasteData(pasteData);
                obj->value_ = std::make_shared<PasteData>(pasteData);
                if (!obj->value_) {
                    asyncContext->status = -1;
                }
            } else {
                asyncContext->status = -1;
            }

            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, instance);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, instance);
                }
            } else {
                SetCallback(env, asyncContext->callbackRef, asyncContext->status, instance);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
            asyncContext = nullptr;
        },
        (void *)asyncContext,
        &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);

    return promise;
}

napi_value SystemPasteboardNapi::SetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SetPasteData is called!");
    size_t argc = ARGC_TYPE_SET2;
    napi_value argv[ARGC_TYPE_SET2] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    AsyncContext *asyncContext = new (std::nothrow) AsyncContext {.env = env, .work = nullptr};
    if (!asyncContext) {
        return NapiGetNull(env);
    }

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");
    PasteDataNapi *obj = nullptr;
    napi_unwrap(env, argv[0], reinterpret_cast<void **>(&obj));
    if (!obj) {
        return NapiGetNull(env);
    }
    asyncContext->obj = obj;

    if (argc >= ARGC_TYPE_SET2) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[1], 1, &asyncContext->callbackRef);
    }

    napi_value promise = nullptr;
    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "SetPasteData", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env,
        nullptr,
        resource,
        [](napi_env env, void* data) {
            AsyncContext* asyncContext = (AsyncContext*)data;
            PasteboardClient::GetInstance()->SetPasteData(*(asyncContext->obj->value_));
            asyncContext->status = 0;
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncContext* asyncContext = (AsyncContext*)data;
            napi_value result = nullptr;
            napi_get_undefined(env, &result);
            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, result);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, result);
                }
            } else {
                SetCallback(env, asyncContext->callbackRef, asyncContext->status, result);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
            asyncContext = nullptr;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);

    return promise;
}

napi_value SystemPasteboardNapi::SystemPasteboardInit(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("clear", Clear),
        DECLARE_NAPI_FUNCTION("getPasteData", GetPasteData),
        DECLARE_NAPI_FUNCTION("hasPasteData", HasPasteData),
        DECLARE_NAPI_FUNCTION("setPasteData", SetPasteData),
    };
    napi_value constructor;
    napi_define_class(env, "SystemPasteboard", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(descriptors) / sizeof(napi_property_descriptor), descriptors, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to define class at SystemPasteboardInit");
        return nullptr;
    }
    napi_create_reference(env, constructor, 1, &g_systemPasteboard);
    status = napi_set_named_property(env, exports, "SystemPasteboard", constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Set property failed when SystemPasteboardInit");
        return nullptr;
    }
    return exports;
}

SystemPasteboardNapi::SystemPasteboardNapi() : env_(nullptr), wrapper_(nullptr)
{
    value_ = std::make_shared<PasteDataNapi>();
}

SystemPasteboardNapi::~SystemPasteboardNapi()
{
    napi_delete_reference(env_, wrapper_);
}

void SystemPasteboardNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    SystemPasteboardNapi *obj = static_cast<SystemPasteboardNapi *>(nativeObject);
    delete obj;
}

napi_value SystemPasteboardNapi::New(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi::New start1");
    // get native object
    SystemPasteboardNapi *obj = new (std::nothrow) SystemPasteboardNapi();
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi::New start2");
    if (!obj) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi::New obj is null");
        return nullptr;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi::New start3");
    // get native object
    obj->env_ = env;
    NAPI_CALL(env,
        napi_wrap(env,
            thisVar,
            obj,
            SystemPasteboardNapi::Destructor,
            nullptr,  // finalize_hint
            &obj->wrapper_));
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi::New start4");
    return thisVar;
}

napi_status SystemPasteboardNapi::NewInstance(napi_env env, napi_value &instance)
{
    napi_status status;

    napi_value constructor;
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi::NewInstance start");
    status = napi_get_reference_value(env, g_systemPasteboard, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "get referece failed");
        return status;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi::NewInstance start2");
    status = napi_new_instance(env, constructor, 0, nullptr, &instance);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "new instance failed");
        return status;
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi::NewInstance start3");
    return napi_ok;
}
} // namespace MiscServicesNapi
} // namespace OHOS