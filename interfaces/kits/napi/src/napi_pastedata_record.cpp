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

using namespace OHOS::MiscServices;

namespace OHOS {
namespace MiscServicesNapi {
static thread_local napi_ref g_pasteDataRecord = nullptr;
std::shared_ptr<MiscServices::PasteDataRecord> PasteDataRecordNapi::value_ = nullptr;

const size_t ARGC_TYPE_SET1 = 1;
const size_t ARGC_TYPE_SET4 = 4;

PasteDataRecordNapi::PasteDataRecordNapi(std::string mimeType,
                                         std::shared_ptr<std::string> htmlText,
                                         std::shared_ptr<std::string> plainText,
                                         std::shared_ptr<std::string> uri) : env_(nullptr), wrapper_(nullptr)
{
    std::string cstring = *uri;
    std::shared_ptr<OHOS::Uri> uuri = std::make_shared<OHOS::Uri>(cstring);//std::make_shared<OHOS::Uri>(*uri);
    value_ = std::make_shared<PasteDataRecord>(mimeType, htmlText, nullptr, plainText, uuri);
}

PasteDataRecordNapi::~PasteDataRecordNapi()
{
    napi_delete_reference(env_, wrapper_);
}

void SetNamedPropertyByStr(napi_env env, napi_value dstObj, const std::string &objName, const char *propName)
{
    napi_value prop = nullptr;
    if (napi_create_string_utf8(env, objName.c_str(), NAPI_AUTO_LENGTH, &prop) == napi_ok) {
        napi_set_named_property(env, dstObj, propName, prop);
    }
}

napi_value PasteDataRecordNapi::ConvertToText(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. string expected.");

    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get ConvertToText length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get ConvertToText failed");
        return nullptr;
    }
    std::string text(buf.data());

    PasteDataNapi *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get ConvertToText object failed");
        return nullptr;
    }
    return nullptr;
}

napi_value PasteDataRecordNapi::PasteDataRecordInit(napi_env env, napi_value exports)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    SetNamedPropertyByStr(env, obj, "htmlText", "");
    SetNamedPropertyByStr(env, obj, "want", "");
    SetNamedPropertyByStr(env, obj, "mimeType", "");
    SetNamedPropertyByStr(env, obj, "plainText", "");
    SetNamedPropertyByStr(env, obj, "uri", "");
    
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_PROPERTY("pasteDataRecord", obj),
        DECLARE_NAPI_FUNCTION("convertToText", ConvertToText),
    };
    
    napi_status status = napi_ok;
    
    napi_value constructor;
    status = napi_define_class(env, "PasteDataRecord", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
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
    size_t argc = ARGC_TYPE_SET4;
    napi_value argv[4] = {0};
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get record new length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get record new failed");
        return nullptr;
    }
    std::string mimeType(buf.data());

    status = napi_get_value_string_utf8(env, argv[1], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get record new1 length failed");
        return nullptr;
    }
    std::vector<char> buf1(len + 1);
    status = napi_get_value_string_utf8(env, argv[1], buf1.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get record new1 failed");
        return nullptr;
    }
    std::string htmlText(buf1.data());

    status = napi_get_value_string_utf8(env, argv[2], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get record new2 length failed");
        return nullptr;
    }
    std::vector<char> buf2(len + 1);
    status = napi_get_value_string_utf8(env, argv[2], buf2.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get record new2 failed");
        return nullptr;
    }
    std::string plainText(buf2.data());

    status = napi_get_value_string_utf8(env, argv[3], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get record new3 length failed");
        return nullptr;
    }
    std::vector<char> buf3(len + 1);
    status = napi_get_value_string_utf8(env, argv[3], buf3.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get record new3 failed");
        return nullptr;
    }
    std::string uri(buf3.data());

    // get native object
    PasteDataRecordNapi *obj = new PasteDataRecordNapi(mimeType,
	                                                   std::make_shared<std::string>(htmlText),
                                                       std::make_shared<std::string>(plainText),
					                                   std::make_shared<std::string>(uri));
    obj->env_ = env;
    NAPI_CALL(env, napi_wrap(env, thisVar, obj, PasteDataRecordNapi::Destructor, nullptr, &obj->wrapper_));
    return thisVar;
}

napi_status PasteDataRecordNapi::NewInstance(napi_env env, napi_value *argv, size_t &argc, napi_value *instance)
{
    napi_status status = napi_invalid_arg;

    if (argc != ARGC_TYPE_SET4) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Wrong number of arguments");
        return status;
    }
    napi_value constructor;
    status = napi_get_reference_value(env, g_pasteDataRecord, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "get referece failed");
        return status;
    }

    status = napi_new_instance(env, constructor, argc, argv, instance);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "new instance failed");
        return status;
    }

    return napi_ok;
}
}  // namespace MiscServicesNapi
}  // namespace OHOS
