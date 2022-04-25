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
#include "pastedata_record_napi.h"
#include "pastedata_napi.h"
#include "pasteboard_common.h"
#include "napi_common.h"
#include "pasteboard_hilog_wreapper.h"

using namespace OHOS::MiscServices;

namespace OHOS {
namespace MiscServicesNapi {
static thread_local napi_ref g_pasteDataRecord = nullptr;
const size_t ARGC_TYPE_SET1 = 1;

PasteDataRecordNapi::PasteDataRecordNapi() : env_(nullptr), wrapper_(nullptr)
{
}

PasteDataRecordNapi::~PasteDataRecordNapi()
{
    napi_delete_reference(env_, wrapper_);
}

bool PasteDataRecordNapi::NewInstanceByRecord(
    napi_env env, napi_value &instance, const std::shared_ptr<MiscServices::PasteDataRecord> &record)
{
    NAPI_CALL(env, PasteDataRecordNapi::NewInstance(env, instance));
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = record;
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewHtmlTextRecordInstance(napi_env env, const std::string &text, napi_value &instance)
{
    NAPI_CALL(env, PasteDataRecordNapi::NewInstance(env, instance));
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateHtmlTextRecord(text);
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewPlainTextRecordInstance(napi_env env, const std::string &text, napi_value &instance)
{
    NAPI_CALL(env, PasteDataRecordNapi::NewInstance(env, instance));
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreatePlainTextRecord(text);
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewUriRecordInstance(napi_env env, const std::string &text, napi_value &instance)
{
    NAPI_CALL(env, PasteDataRecordNapi::NewInstance(env, instance));
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateUriRecord(OHOS::Uri(text));
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewWantRecordInstance(
    napi_env env, const std::shared_ptr<OHOS::AAFwk::Want> want, napi_value &instance)
{
    if (!want) {
        return false;
    }

    NAPI_CALL(env, PasteDataRecordNapi::NewInstance(env, instance));
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateWantRecord(want);
    obj->JSFillInstance(env, instance);
    return true;
}

void PasteDataRecordNapi::SetNamedPropertyByStr(napi_env env, napi_value &dstObj,
    const std::string &objName, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_string_utf8(env, objName.c_str(), NAPI_AUTO_LENGTH, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

bool PasteDataRecordNapi::JSFillInstance(napi_env env, napi_value &instance)
{
    if (!instance || !value_) {
        return false;
    }
    auto mimeType = value_->GetMimeType();
    SetNamedPropertyByStr(env, instance, "mimeType", mimeType.c_str());
    if (mimeType == MIMETYPE_TEXT_PLAIN) {
        auto plainText = value_->GetPlainText();
        if (plainText != nullptr) {
            SetNamedPropertyByStr(env, instance, "plainText", plainText->c_str());
        }
    } else if (mimeType == MIMETYPE_TEXT_HTML) {
        auto htmlText = value_->GetHtmlText();
        if (htmlText != nullptr) {
            SetNamedPropertyByStr(env, instance, "htmlText", htmlText->c_str());
        }
    } else if (mimeType == MIMETYPE_TEXT_URI) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "ggg.");
        auto uri = value_->GetUri();
        if (uri != nullptr) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "hhh.");
            SetNamedPropertyByStr(env, instance, "uri", uri->ToString().c_str());
        }
    } else if (mimeType == MIMETYPE_TEXT_WANT) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "iii.");
        auto want = value_->GetWant();
        if (want != nullptr) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "jjj.");
            napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, *want);
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "kkk.");
            napi_set_named_property(env, instance, "want", jsWant);
        }
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "Unkonw MimeType: %{public}s.", mimeType.c_str());
        return false;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "end.");
    return true;
}

using AsyncText = struct AsyncText {
    napi_env env = nullptr;
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    PasteDataRecordNapi *obj = nullptr;
    int32_t status = 0;
    std::string text;
};

void AsyncCompleteCallbackConvertToText(napi_env env, napi_status status, void *data)
{
    if (!data) {
        return;
    }
    AsyncText* asyncText = (AsyncText*)data;
    napi_value result = nullptr;
    napi_create_string_utf8(env, asyncText->text.c_str(), NAPI_AUTO_LENGTH, &result);
    if (asyncText->deferred) {
        if (!asyncText->status) {
            napi_resolve_deferred(env, asyncText->deferred, result);
        } else {
            napi_reject_deferred(env, asyncText->deferred, result);
        }
    } else {
        SetCallback(env, asyncText->callbackRef, asyncText->status, result);
        napi_delete_reference(env, asyncText->callbackRef);
    }
    napi_delete_async_work(env, asyncText->work);
    delete asyncText;
    asyncText = nullptr;
}

napi_value PasteDataRecordNapi::ConvertToText(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
   
    PasteDataRecordNapi *obj = nullptr;
    auto status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get ConvertToText object failed");
        return nullptr;
    }
    AsyncText *asyncText = new (std::nothrow) AsyncText {.env = env, .work = nullptr, .obj = obj};
    if (!asyncText) {
        return NapiGetNull(env);
    }
    if (argc >= ARGC_TYPE_SET1) {
        napi_valuetype valueType = napi_undefined;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        napi_create_reference(env, argv[0], 1, &asyncText->callbackRef);
    }
    napi_value promise = nullptr;
    if (asyncText->callbackRef == nullptr) {
        napi_create_promise(env, &asyncText->deferred, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "ConvertToText", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env,
        nullptr, resource,
        [](napi_env env, void *data) {
            AsyncText* asyncText = (AsyncText*)data;
            if (!asyncText->obj->value_) {
                asyncText->status = -1;
            }
            asyncText->text = asyncText->obj->value_->ConvertToText();
        },
        AsyncCompleteCallbackConvertToText,
        (void *)asyncText, &asyncText->work);
    napi_queue_async_work(env, asyncText->work);
    return promise;
}

napi_value PasteDataRecordNapi::PasteDataRecordInit(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("convertToText", ConvertToText),
    };

    napi_status status = napi_ok;

    napi_value constructor;
    status = napi_define_class(env,
        "PasteDataRecord",
        NAPI_AUTO_LENGTH,
        New,
        nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor),
        properties,
        &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to define class at PasteDataRecordInit");
        return nullptr;
    }

    status = napi_create_reference(env, constructor, 1, &g_pasteDataRecord);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "PasteDataRecordInit create referece failed");
        return nullptr;
    }
    napi_set_named_property(env, exports, "PasteDataRecord", constructor);
    return exports;
}

void PasteDataRecordNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    PasteDataRecordNapi *obj = static_cast<PasteDataRecordNapi *>(nativeObject);
    obj->~PasteDataRecordNapi();
}

napi_value PasteDataRecordNapi::New(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    // get native object
    PasteDataRecordNapi *obj = new PasteDataRecordNapi();
    obj->env_ = env;
    NAPI_CALL(env, napi_wrap(env, thisVar, obj, PasteDataRecordNapi::Destructor, nullptr, &obj->wrapper_));
    return thisVar;
}

napi_status PasteDataRecordNapi::NewInstance(napi_env env, napi_value &instance)
{
    napi_status status = napi_invalid_arg;

    napi_value constructor;
    status = napi_get_reference_value(env, g_pasteDataRecord, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "get referece failed");
        return status;
    }

    status = napi_new_instance(env, constructor, 0, nullptr, &instance);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "new instance failed");
        return status;
    }

    return napi_ok;
}
} // namespace MiscServicesNapi
} // namespace OHOS
