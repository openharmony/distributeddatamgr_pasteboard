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

using namespace OHOS::MiscServices;

namespace OHOS {
namespace MiscServicesNapi {
static thread_local napi_ref g_systemPasteboard = nullptr;
const size_t ARGC_TYPE_SET1 = 1;
const size_t ARGC_TYPE_SET2 = 2;
const size_t ARGC_TYPE_SET4 = 4;

napi_value JScreateHtmlTextRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateHtmlTextRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[4] = {0};
    napi_value thisVar = nullptr;

    napi_create_string_utf8(env, "text/html", NAPI_AUTO_LENGTH, &argv[0]);
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[1]);
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[2]);
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[3]);

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv[1], &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. string expected.");

    napi_value instance = nullptr;
    argc = ARGC_TYPE_SET4;
    napi_status status = PasteDataRecordNapi::NewInstance(env, argv, argc, &instance); // 4 arguments
    if (status != napi_ok) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "JScreateHtmlTextRecord create instance failed");
        return nullptr;
    }
    return instance;
}

napi_value JScreatePlainTextRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreatePlainTextRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[4] = {0};
    napi_value thisVar = nullptr;

    napi_create_string_utf8(env, "text/plain", NAPI_AUTO_LENGTH, &argv[0]);
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[1]);
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[2]);
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[3]);

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv[2], &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[2], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. string expected.");

    napi_value instance = nullptr;
    argc = ARGC_TYPE_SET4;
    napi_status status = PasteDataRecordNapi::NewInstance(env, argv, argc, &instance); // 4 arguments
    if (status != napi_ok) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "JScreatePlainTextRecord create instance failed");
        return nullptr;
    }

    return instance;
}

napi_value JScreateUriRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateUriRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[4] = {0};
    napi_value thisVar = nullptr;

    napi_create_string_utf8(env, "text/uri", NAPI_AUTO_LENGTH, &argv[0]);
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[1]);
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[2]);
    napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &argv[3]);

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv[3], &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[3], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. string expected.");

    napi_value instance = nullptr;
    argc = ARGC_TYPE_SET4;
    napi_status status = PasteDataRecordNapi::NewInstance(env, argv, argc, &instance); // 4 arguments
    if (status != napi_ok) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "JScreateUriRecord create instance failed");
        return nullptr;
    }
    
    return instance;
}

napi_value JScreateHtmlData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateHtmlData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "aaa");
    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, &instance));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "bbb");

    // napi_value func = nullptr;
    // napi_status status = napi_get_named_property(env, instance, "AddHtmlRecord", &func);
    // NAPI_ASSERT(env, status == napi_ok, "napi get func failed!");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ccc");

    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, func, &valueType);
    NAPI_ASSERT(env, valueType == napi_function, "func is not napi_function!");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ddd");

    napi_value callbackResult = nullptr;
    status = napi_call_function(env, instance, func, 1, argv, &callbackResult);
    NAPI_ASSERT(env, status == napi_ok, "napi call func failed!");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "eee");

    return instance;
}

napi_value JScreatePlainTextData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreatePlainTextData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "111");

    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, &instance));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "aaa");

    napi_value func = nullptr;
    napi_status status = napi_get_named_property(env, instance, "AddTextRecord", &func);
    NAPI_ASSERT(env, status == napi_ok, "napi get func failed!");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "bbb");

    // napi_valuetype valueType = napi_undefined;
    // napi_typeof(env, func, &valueType);
    // NAPI_ASSERT(env, valueType == napi_function, "func is not napi_function!");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ccc");

    napi_value callbackResult = nullptr;
    status = napi_call_function(env, instance, func, 1, argv, &callbackResult);
    NAPI_ASSERT(env, status == napi_ok, "napi call func failed!");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ddd");

    return instance;
}

napi_value JScreateUriData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateUriData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "aaa");

    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, &instance));

    napi_value func = nullptr;
    napi_status status = napi_get_named_property(env, instance, "AddUriRecord", &func);
    NAPI_ASSERT(env, status == napi_ok, "napi get func failed!");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "bbb");

    // napi_valuetype valueType = napi_undefined;
    // napi_typeof(env, func, &valueType);
    // NAPI_ASSERT(env, valueType == napi_function, "func is not napi_function!");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ccc");

    napi_value callbackResult = nullptr;
    status = napi_call_function(env, instance, func, 1, argv, &callbackResult);
    NAPI_ASSERT(env, status == napi_ok, "napi call func failed!");
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ddd");

    return instance;
}

napi_value JSgetSystemPasteboard(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JSgetSystemPasteboard is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    napi_value instance = nullptr;
    napi_status status = SystemPasteboardNapi::NewInstance(env, &instance); // 0 arguments
    if (status != napi_ok) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "JSgetSystemPasteboard create instance failed");
        return nullptr;
    }
    
    return instance;
}

napi_value PasteBoardInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createHtmlData", JScreateHtmlData),
        DECLARE_NAPI_FUNCTION("createPlainTextData", JScreatePlainTextData),
        DECLARE_NAPI_FUNCTION("createUriData", JScreateUriData),
        DECLARE_NAPI_FUNCTION("createHtmlTextRecord", JScreateHtmlTextRecord),
        DECLARE_NAPI_FUNCTION("createPlainTextRecord", JScreatePlainTextRecord),
        DECLARE_NAPI_FUNCTION("createUriRecord", JScreateUriRecord),
        DECLARE_NAPI_FUNCTION("getSystemPasteboard", JSgetSystemPasteboard),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    return exports;
}

using AsyncContext = struct AsyncContext {
    napi_env env = nullptr;
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    napi_value obj = nullptr;
    int32_t status = 0;
    napi_value result;
};

static const std::int32_t SystemPasteboard_MAX_PARA = 1;
napi_value SystemPasteboardParamGet(const napi_env &env, const napi_value (&argv)[SystemPasteboard_MAX_PARA],
    const size_t &argc, napi_ref &callback)
{
    NAPI_ASSERT(env, argc >= SystemPasteboard_MAX_PARA, "Wrong number of arguments");
    napi_valuetype valueType = napi_undefined;
    
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
    napi_create_reference(env, argv[0], 1, &callback);

    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

void SetCallback(const napi_env &env, const napi_ref &callbackIn, const napi_value *result)
{
    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_call_function(env, nullptr, callback, 1, result, &resultout);
}
napi_value SystemPasteboardNapi::GetCallback(void)
{
    return callback_;
}

napi_value SystemPasteboardNapi::callback_ = nullptr;
napi_env SystemPasteboardNapi::callbackEnv;
void SystemPasteboardNapi::CallbackOn(void)
{
    napi_value callback = GetCallback();
    napi_call_function(callbackEnv, nullptr, callback, 0, nullptr, nullptr);
}

napi_value SystemPasteboardNapi::On(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi on() is called!");
    size_t argc = ARGC_TYPE_SET2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = 0;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    napi_valuetype valueType;
    napi_typeof(env, argv[1], &valueType);
    NAPI_ASSERT(env, valueType == napi_function, "type mismatch for parameter");
    callbackEnv = env;
    callback_ = argv[1];
    
    PasteboardClient::GetInstance()->AddPasteboardChangedObserver(CallbackOn);
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi on() is end!");
    return result;
}

napi_value SystemPasteboardNapi::Off(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi off () is called!");
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = 0;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);

    callbackEnv = env;
    
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
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    AsyncContext* asyncContext = new (std::nothrow)AsyncContext {
        .env = env,
        .work = nullptr,
    };

    if (argc == ARGC_TYPE_SET1) {
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
            asyncContext->status = 1;
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
                SetCallback(env, asyncContext->callbackRef, &result);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);
    
    return promise;
}

napi_value SystemPasteboardNapi::HasPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "HasPasteData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    AsyncContext* asyncContext = new (std::nothrow)AsyncContext {
        .env = env,
        .work = nullptr,
    };

    if (argc == ARGC_TYPE_SET1) {
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
            if (PasteboardClient::GetInstance()->HasPasteData() == false) {
                asyncContext->status = 1;
            } else {
                asyncContext->status = 0;
            }
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
                SetCallback(env, asyncContext->callbackRef, &result);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);
    
    return promise;
}

napi_value SystemPasteboardNapi::GetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetPasteData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    AsyncContext* asyncContext = new (std::nothrow)AsyncContext {
        .env = env,
        .work = nullptr,
    };

    if (argc == ARGC_TYPE_SET1) {
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
        [](napi_env env, void* data) {
            AsyncContext* asyncContext = (AsyncContext*)data;
            napi_value instance = nullptr;
            PasteDataNapi::NewInstance(env, &instance);
            napi_value func = nullptr;
            napi_get_named_property(env, instance, "GetSystemPasteboard", &func);
            napi_valuetype valueType = napi_undefined;
            napi_typeof(env, func, &valueType);
            napi_value callbackResult = nullptr;
            napi_call_function(env, instance, func, 0, nullptr, &callbackResult);
            napi_get_value_int32(env, callbackResult, &asyncContext->status);
            asyncContext->result = instance;
        },
        [](napi_env env, napi_status status, void* data) {
            AsyncContext* asyncContext = (AsyncContext*)data;
            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, asyncContext->result);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, asyncContext->result);
                }
            } else {
                napi_value callback = nullptr;
                napi_value callbackResult = nullptr;
                napi_get_reference_value(env, asyncContext->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, 1, &asyncContext->result, &callbackResult);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
        },
        (void*)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);
    
    return promise;
}

napi_value SystemPasteboardNapi::SetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SetPasteData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[2] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
    AsyncContext* asyncContext = new (std::nothrow)AsyncContext {
        .env = env,
        .work = nullptr,
    };
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Table name should be an object.");
    asyncContext->obj = argv[0];
    if (argc == ARGC_TYPE_SET2) {
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
            PasteDataNapi *obj = nullptr;
            napi_unwrap(env, asyncContext->obj, reinterpret_cast<void **>(&obj));
            PasteboardClient::GetInstance()->SetPasteData(*(obj->value_));
            asyncContext->status = 1;
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
                SetCallback(env, asyncContext->callbackRef, &result);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
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
    PasteboardClient::GetInstance()->GetPasteData(*pasteData_);
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

    // get native object
    SystemPasteboardNapi *obj = new SystemPasteboardNapi();
    obj->env_ = env;
    NAPI_CALL(env, napi_wrap(env, thisVar, obj, SystemPasteboardNapi::Destructor,
        nullptr, // finalize_hint
        &obj->wrapper_));
    return thisVar;
}

napi_status SystemPasteboardNapi::NewInstance(napi_env env, napi_value *instance)
{
    napi_status status;

    napi_value constructor;
    status = napi_get_reference_value(env, g_systemPasteboard, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "get referece failed");
        return status;
    }

    status = napi_new_instance(env, constructor, 0, nullptr, instance);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "new instance failed");
        return status;
    }

    return napi_ok;
}
} // namespace MiscServicesNapi
} // namespace OHOS