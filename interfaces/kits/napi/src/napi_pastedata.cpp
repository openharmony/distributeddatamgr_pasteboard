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
#include <cstddef>
#include "pastedata_napi.h"
#include "pastedata_record_napi.h"
#include "pasteboard_common.h"
#include "pasteboard_hilog_wreapper.h"
#include "napi_common.h"

using namespace OHOS::MiscServices;

namespace OHOS {
namespace MiscServicesNapi {
namespace {
constexpr int ARGC_TYPE_SET1 = 1;
const int32_t STR_MAX_SIZE = 256;
}  // namespace
static thread_local napi_ref g_pasteData = nullptr;

PasteDataNapi::PasteDataNapi() : env_(nullptr), wrapper_(nullptr)
{
    value_ = std::make_shared<PasteData>();
}

PasteDataNapi::~PasteDataNapi()
{
    napi_delete_reference(env_, wrapper_);
}

napi_value PasteDataNapi::AddHtmlRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "AddHtmlRecord is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. string expected.");

    size_t len = 0;
    char str[STR_MAX_SIZE] = {0};
    napi_status status = napi_get_value_string_utf8(env, argv[0], str, STR_MAX_SIZE, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddHtmlRecord length failed");
        return nullptr;
    }
    std::string text = str;

    PasteDataNapi *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddHtmlRecord object failed");
        return nullptr;
    }
    obj->value_->AddHtmlRecord(text);
    return nullptr;
}

napi_value PasteDataNapi::AddTextRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "AddTextRecord is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. string expected.");

    size_t len = 0;
    char str[STR_MAX_SIZE] = {0};
    napi_status status = napi_get_value_string_utf8(env, argv[0], str, STR_MAX_SIZE, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddTextRecord length failed");
        return nullptr;
    }
    std::string text = str;

    PasteDataNapi *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddTextRecord object failed");
        return nullptr;
    }
    obj->value_->AddTextRecord(text);
    return nullptr;
}

napi_value PasteDataNapi::AddUriRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "AddUriRecord is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. string expected.");

    size_t len = 0;
    char str[STR_MAX_SIZE] = {0};
    napi_status status = napi_get_value_string_utf8(env, argv[0], str, STR_MAX_SIZE, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddUriRecord length failed");
        return nullptr;
    }
    std::string text = str;

    PasteDataNapi *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddUriRecord object failed");
        return nullptr;
    }
    obj->value_->AddUriRecord(OHOS::Uri(text));
    return nullptr;
}

napi_value PasteDataNapi::GetPrimaryHtml(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetPrimaryHtml is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetPrimaryHtml object failed");
        return nullptr;
    }

    std::shared_ptr<std::string> p = obj->value_->GetPrimaryHtml();
    napi_value result = nullptr;
    napi_create_string_utf8(env, p->c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value PasteDataNapi::GetPrimaryText(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetPrimaryText is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetPrimaryText object failed");
        return nullptr;
    }

    std::shared_ptr<std::string> p = obj->value_->GetPrimaryText();
    napi_value result = nullptr;
    napi_create_string_utf8(env, p->c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value PasteDataNapi::GetPrimaryUri(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetPrimaryUri is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetPrimaryUri object failed");
        return nullptr;
    }

    std::shared_ptr<OHOS::Uri> p = obj->value_->GetPrimaryUri();
    std::string text = p->ToString();
    napi_value result = nullptr;
    napi_create_string_utf8(env, text.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value PasteDataNapi::HasMimeType(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "HasMimeType is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. string expected.");

    size_t len = 0;
    char str[STR_MAX_SIZE] = {0};
    napi_status status = napi_get_value_string_utf8(env, argv[0], str, STR_MAX_SIZE, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddHtmlRecord length failed");
        return nullptr;
    }
    std::string mimeType = str;

    PasteDataNapi *obj = nullptr;
    status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddHtmlRecord object failed");
        return nullptr;
    }

    bool ret = obj->value_->HasMimeType(mimeType);
    napi_value result = nullptr;
    napi_get_boolean(env, ret, &result);

    return result;
}

napi_value PasteDataNapi::RemoveRecordAt(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "RemoveRecordAt is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. number expected.");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get RemoveRecordAt object failed");
        return nullptr;
    }

    int64_t number = 0;
    napi_get_value_int64(env, argv[0], &number);
    bool ret = obj->value_->RemoveRecordAt(number);
    napi_value result = nullptr;
    napi_get_boolean(env, ret, &result);

    return result;
}

napi_value PasteDataNapi::GetPrimaryMimeType(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetPrimaryMimeType is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetPrimaryMimeType object failed");
        return nullptr;
    }
    std::shared_ptr<std::string> mimeType = obj->value_->GetPrimaryMimeType();
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, mimeType->c_str(), NAPI_AUTO_LENGTH, &result);

    return result;
}

napi_value PasteDataNapi::GetRecordCount(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetRecordCount is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetRecordCount object failed");
        return nullptr;
    }

    size_t count = obj->value_->GetRecordCount();
    napi_value result = nullptr;
    status = napi_create_int64(env, count, &result);

    return result;
}

napi_value PasteDataNapi::GetTag(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetTag is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetTag object failed");
        return nullptr;
    }
    std::string tag = obj->value_->GetTag();
    napi_value result = nullptr;
    status = napi_create_string_utf8(env, tag.c_str(), NAPI_AUTO_LENGTH, &result);

    return result;
}

napi_value PasteDataNapi::GetMimeTypes(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetMimeTypes is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetMimeTypes object failed");
        return nullptr;
    }
    std::vector<std::string> mimeTypes = obj->value_->GetMimeTypes();
    if (mimeTypes.size() == 0) {
        return nullptr;
    }

    napi_value nMimeTypes = nullptr;
    if (napi_create_array(env, &nMimeTypes) != napi_ok) {
        return nullptr;
    }
    size_t index = 0;
    napi_value value = nullptr;
    for (auto type : mimeTypes) {
        napi_create_string_utf8(env, type.c_str(), NAPI_AUTO_LENGTH, &value);
        napi_set_element(env, nMimeTypes, index, value);
        index++;
    }
    return nMimeTypes;
}

napi_value PasteDataNapi::AddRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "AddRecord is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddRecord object failed");
        return nullptr;
    }

    PasteDataRecordNapi *record = nullptr;
    status = napi_unwrap(env, argv[0], reinterpret_cast<void **>(&record));
    if ((status != napi_ok) || (record == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get PasteDataRecord object failed");
        return nullptr;
    }

    obj->value_->AddRecord(record->value_);
    return nullptr;
}

napi_value PasteDataNapi::ReplaceRecordAt(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ReplaceRecordAt is called!");
    size_t argc = 1;
    napi_value argv[2] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 2, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. number expected.");
    NAPI_CALL(env, napi_typeof(env, argv[1], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    napi_value result = nullptr;
    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get ReplaceRecordAt object failed");
        napi_get_boolean(env, false, &result);
        return result;
    }

    int64_t number = 0;
    napi_get_value_int64(env, argv[0], &number);

    PasteDataRecordNapi *record = nullptr;
    status = napi_unwrap(env, argv[0], reinterpret_cast<void **>(&record));
    if ((status != napi_ok) || (record == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get PasteDataRecord object failed");
        return nullptr;
    }

    bool ret = obj->value_->ReplaceRecordAt(number, record->value_);
    napi_get_boolean(env, ret, &result);

    return result;
}

napi_value PasteDataNapi::AddWantRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "AddWantRecord is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");
    OHOS::AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, argv[0], want)) {
        return nullptr;
    }

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get AddWantRecord object failed");
        return nullptr;
    }

    obj->value_->AddWantRecord(std::make_shared<OHOS::AAFwk::Want>(want));
    return nullptr;
}

napi_value PasteDataNapi::GetPrimaryWant(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetPrimaryWant is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetPrimaryWant object failed");
        return nullptr;
    }

    std::shared_ptr<OHOS::AAFwk::Want> want = obj->value_->GetPrimaryWant();
    if (!want) {
        return nullptr;
    }
    return OHOS::AppExecFwk::WrapWant(env, *want);
}

bool PasteDataNapi::SetNapiProperty(napi_env env, const PasteDataProperty &property, napi_value &NProperty)
{
    napi_value value = nullptr;
    napi_value arr = nullptr;
    int count = 0;

    //additions : {[key: string]: object}
    value = OHOS::AppExecFwk::WrapWantParams(env, property.additions);
    napi_set_named_property(env, NProperty, "additions", value);

    //mimeTypes: Array<string>
    napi_create_array(env, &arr);
    for (auto vec : property.mimeTypes) {
        napi_create_string_utf8(env, vec.c_str(), NAPI_AUTO_LENGTH, &value);
        napi_set_element(env, arr, count, value);
        count++;
    }
    if (count > 0) {
        napi_set_named_property(env, NProperty, "mimeTypes", arr);
    }

    //tag: string
    napi_create_string_utf8(env, property.tag.c_str(), NAPI_AUTO_LENGTH, &value);
    napi_set_named_property(env, NProperty, "tag", value);

    //timestamp: number
    napi_create_int64(env, property.timestamp, &value);
    napi_set_named_property(env, NProperty, "timestamp", value);

    return true;
}

napi_value PasteDataNapi::GetProperty(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetProperty is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 0, "Wrong number of arguments");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetProperty object failed");
        return nullptr;
    }
    PasteDataProperty property = obj->value_->GetProperty();
    napi_value NProperty = nullptr;
    napi_create_object(env, &NProperty);
    if (!SetNapiProperty(env, property, NProperty)) {
        return nullptr;
    }
    return NProperty;
}

napi_value PasteDataNapi::GetRecordAt(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetRecordAt is called!");
    size_t argc = 1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Wrong argument type. number expected.");

    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get GetRecordAt object failed");
        return nullptr;
    }
    int64_t number = 0;
    napi_get_value_int64(env, argv[0], &number);
    std::shared_ptr<PasteDataRecord> record = obj->value_->GetRecordAt(number);

    napi_value instance = nullptr;
    PasteDataRecordNapi::NewInstanceByRecord(env, instance, record);
    return instance;
}

napi_value PasteDataNapi::PasteDataInit(napi_env env, napi_value exports)
{
    napi_status status = napi_ok;
    napi_property_descriptor descriptors[] = {
        DECLARE_NAPI_FUNCTION("addHtmlRecord", AddHtmlRecord),
        DECLARE_NAPI_FUNCTION("addWantRecord", AddWantRecord),
        DECLARE_NAPI_FUNCTION("addRecord", AddRecord),
        DECLARE_NAPI_FUNCTION("addTextRecord", AddTextRecord),
        DECLARE_NAPI_FUNCTION("addUriRecord", AddUriRecord),
        DECLARE_NAPI_FUNCTION("getMimeTypes", GetMimeTypes),
        DECLARE_NAPI_FUNCTION("getPrimaryHtml", GetPrimaryHtml),
        DECLARE_NAPI_FUNCTION("getPrimaryWant", GetPrimaryWant),
        DECLARE_NAPI_FUNCTION("getPrimaryMimeType", GetPrimaryMimeType),
        DECLARE_NAPI_FUNCTION("getPrimaryText", GetPrimaryText),
        DECLARE_NAPI_FUNCTION("getPrimaryUri", GetPrimaryUri),
        DECLARE_NAPI_FUNCTION("getProperty", GetProperty),
        DECLARE_NAPI_FUNCTION("getRecordAt", GetRecordAt),
        DECLARE_NAPI_FUNCTION("getRecordCount", GetRecordCount),
        DECLARE_NAPI_FUNCTION("getTag", GetTag),
        DECLARE_NAPI_FUNCTION("hasMimeType", HasMimeType),
        DECLARE_NAPI_FUNCTION("removeRecordAt", RemoveRecordAt),
        DECLARE_NAPI_FUNCTION("replaceRecordAt", ReplaceRecordAt),
    };

    napi_value constructor;
    napi_define_class(env, "PasteData", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(descriptors) / sizeof(napi_property_descriptor), descriptors, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to define class at Init");
        return nullptr;
    }

    status = napi_create_reference(env, constructor, 1, &g_pasteData);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "PasteDataNapi Init create referece failed");
        return nullptr;
    }
    napi_set_named_property(env, exports, "PasteData", constructor);
    return exports;
}

void PasteDataNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    PasteDataNapi *obj = static_cast<PasteDataNapi *>(nativeObject);
    delete obj;
}

napi_value PasteDataNapi::New(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = {0};
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    // get native object
    PasteDataNapi *obj = new PasteDataNapi();
    obj->env_ = env;
    NAPI_CALL(env, napi_wrap(env, thisVar, obj, PasteDataNapi::Destructor,
        nullptr, // finalize_hint
        &obj->wrapper_));
    return thisVar;
}

napi_status PasteDataNapi::NewInstance(napi_env env, napi_value &instance)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "enter");
    napi_status status;
    napi_value constructor;
    status = napi_get_reference_value(env, g_pasteData, &constructor);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "get referece failed");
        return status;
    }

    status = napi_new_instance(env, constructor, 0, nullptr, &instance);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "new instance failed");
        return status;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "new instance ok");
    return napi_ok;
}

}  // namespace MiscServicesNapi
}  // namespace OHOS
