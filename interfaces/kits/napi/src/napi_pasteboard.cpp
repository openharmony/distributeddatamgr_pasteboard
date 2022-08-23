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
#include <uv.h>

#include "napi_common.h"
#include "pasteboard_common.h"
#include "pasteboard_hilog_wreapper.h"
#include "systempasteboard_napi.h"

using namespace OHOS::MiscServices;
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServicesNapi {
static thread_local napi_ref g_systemPasteboard = nullptr;
std::mutex SystemPasteboardNapi::pasteboardObserverInsMutex_;
std::map<napi_ref, std::shared_ptr<PasteboardObserverInstance>> SystemPasteboardNapi::observers_;
const size_t ARGC_TYPE_SET1 = 1;
const size_t ARGC_TYPE_SET2 = 2;
const int32_t STR_DATA_SIZE = 10;
const std::string STRING_UPDATE = "update";

PasteboardObserverInstance::PasteboardObserverInstance(const napi_env &env, const napi_ref &ref)
    : env_(env), ref_(ref), isOff_(false)
{
    stub_ = new (std::nothrow) PasteboardObserverInstance::PasteboardObserverImpl();
}

PasteboardObserverInstance::~PasteboardObserverInstance()
{
    napi_delete_reference(env_, ref_);
}

void PasteboardObserverInstance::setOff()
{
    isOff_ = true;
}

sptr<PasteboardObserverInstance::PasteboardObserverImpl> PasteboardObserverInstance::GetStub()
{
    return stub_;
}

void UvQueueWorkOnPasteboardChanged(uv_work_t *work, int status)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "UvQueueWorkOnPasteboardChanged start");
    if (work == nullptr) {
        return;
    }
    PasteboardDataWorker *pasteboardDataWorker = (PasteboardDataWorker *)work->data;
    if (pasteboardDataWorker == nullptr || pasteboardDataWorker->observer == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "pasteboardDataWorker or ref is null");
        delete work;
        work = nullptr;
        return;
    }

    auto env = pasteboardDataWorker->observer->GetEnv();
    auto ref = pasteboardDataWorker->observer->GetRef();

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "pasteboardDataWorker ref = %{public}p", ref);
    napi_get_reference_value(env, ref, &callback);
    napi_value result = NapiGetNull(env);
    napi_call_function(env, undefined, callback, 0, &result, &resultout);

    delete pasteboardDataWorker;
    pasteboardDataWorker = nullptr;
    delete work;
}

void PasteboardObserverInstance::OnPasteboardChanged()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "OnPasteboardChanged is called!");
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "loop instance is nullptr");
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "work is null");
        return;
    }
    PasteboardDataWorker *pasteboardDataWorker = new (std::nothrow) PasteboardDataWorker();
    if (pasteboardDataWorker == nullptr) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "pasteboardDataWorker is null");
        delete work;
        work = nullptr;
        return;
    }
    pasteboardDataWorker->observer = shared_from_this();

    work->data = (void *)pasteboardDataWorker;

    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {}, UvQueueWorkOnPasteboardChanged);
    if (ret != 0) {
        delete pasteboardDataWorker;
        pasteboardDataWorker = nullptr;
        delete work;
        work = nullptr;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "OnPasteboardChanged end");
}

napi_value JScreateHtmlTextRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateHtmlTextRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");

    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get data failed");
        return nullptr;
    }
    std::string str(buf.data());
    napi_value instance = nullptr;
    PasteDataRecordNapi::NewHtmlTextRecordInstance(env, str, instance);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "end.");
    return instance;
}

napi_value JScreateWantRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateWantRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
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
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");

    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get length failed");
        return nullptr;
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "ddd.");
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get data failed");
        return nullptr;
    }
    std::string str(buf.data());
    napi_value instance = nullptr;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "eee.");
    PasteDataRecordNapi::NewPlainTextRecordInstance(env, str, instance);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "end.");
    return instance;
}

napi_value JScreatePixelMapRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreatePixelMapRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == ARGC_TYPE_SET1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    std::shared_ptr<PixelMap> pixelMap = PixelMapNapi::GetPixelMap(env, argv[0]);
    if (pixelMap == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to get pixelMap!");
        return nullptr;
    }
    napi_value instance = nullptr;
    PasteDataRecordNapi::NewPixelMapRecordInstance(env, pixelMap, instance);

    return instance;
}

napi_value JScreateUriRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateUriRecord is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get data failed");
        return nullptr;
    }
    std::string str(buf.data());
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
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get data failed");
        return nullptr;
    }
    std::string str(buf.data());
    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi *obj = nullptr;
    status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
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
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get JScreatePlainTextData length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get JScreatePlainTextData data failed");
        return nullptr;
    }
    std::string str(buf.data());
    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi *obj = nullptr;
    status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return nullptr;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreatePlainTextData(str);

    return instance;
}

napi_value JScreatePixelMapData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreatePixelMapData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc == ARGC_TYPE_SET1, "Wrong number of arguments");

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    std::shared_ptr<PixelMap> pixelMap = PixelMapNapi::GetPixelMap(env, argv[0]);
    if (pixelMap == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Failed to get pixelMap!");
        return nullptr;
    }

    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "unwrap Failed!");
        return nullptr;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMap);
    return instance;
}

napi_value JScreateUriData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateUriData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[0], nullptr, 0, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get length failed");
        return nullptr;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, argv[0], buf.data(), len + 1, &len);
    if (status != napi_ok) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get data failed");
        return nullptr;
    }
    std::string str(buf.data());
    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi *obj = nullptr;
    status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return nullptr;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateUriData(OHOS::Uri(str));

    return instance;
}

napi_value JSgetSystemPasteboard(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JSgetSystemPasteboard is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[1] = { 0 };
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
    napi_property_descriptor desc[] = { DECLARE_NAPI_FUNCTION("createHtmlData", JScreateHtmlData),
        DECLARE_NAPI_FUNCTION("createWantData", JScreateWantData),
        DECLARE_NAPI_FUNCTION("createPlainTextData", JScreatePlainTextData),
        DECLARE_NAPI_FUNCTION("createPixelMapData", JScreatePixelMapData),
        DECLARE_NAPI_FUNCTION("createUriData", JScreateUriData),
        DECLARE_NAPI_FUNCTION("createHtmlTextRecord", JScreateHtmlTextRecord),
        DECLARE_NAPI_FUNCTION("createWantRecord", JScreateWantRecord),
        DECLARE_NAPI_FUNCTION("createPlainTextRecord", JScreatePlainTextRecord),
        DECLARE_NAPI_FUNCTION("createPixelMapRecord", JScreatePixelMapRecord),
        DECLARE_NAPI_FUNCTION("createUriRecord", JScreateUriRecord),
        DECLARE_NAPI_FUNCTION("getSystemPasteboard", JSgetSystemPasteboard),
        DECLARE_NAPI_PROPERTY("MAX_RECORD_NUM", CreateNapiNumber(env, 128)),
        DECLARE_NAPI_PROPERTY("MIMETYPE_PIXELMAP", CreateNapiString(env, MIMETYPE_PIXELMAP)),
        DECLARE_NAPI_PROPERTY("MIMETYPE_TEXT_HTML", CreateNapiString(env, MIMETYPE_TEXT_HTML)),
        DECLARE_NAPI_PROPERTY("MIMETYPE_TEXT_WANT", CreateNapiString(env, MIMETYPE_TEXT_WANT)),
        DECLARE_NAPI_PROPERTY("MIMETYPE_TEXT_PLAIN", CreateNapiString(env, MIMETYPE_TEXT_PLAIN)),
        DECLARE_NAPI_PROPERTY("MIMETYPE_TEXT_URI", CreateNapiString(env, MIMETYPE_TEXT_URI)) };

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
    bool result = false;
};

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
    char str[STR_DATA_SIZE] = { 0 };
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE, &strLen));
    NAPI_ASSERT(env, strLen == STRING_UPDATE.length(), "error type");

    napi_typeof(env, argv[ARGC_TYPE_SET1], &valueType);
    NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");

    napi_ref ref = nullptr;
    napi_create_reference(env, argv[ARGC_TYPE_SET1], 1, &ref);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "byy1111");
    auto observer = std::make_shared<PasteboardObserverInstance>(env, ref);
    observer->GetStub()->outer_ = observer;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "byy2222");
    PasteboardClient::GetInstance()->AddPasteboardChangedObserver(observer->GetStub());
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "byy3333");
    std::lock_guard<std::mutex> lock(pasteboardObserverInsMutex_);
    observers_[ref] = observer;
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
    char str[STR_DATA_SIZE] = { 0 };
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE, &strLen));
    NAPI_ASSERT(env, strLen == STRING_UPDATE.length(), "error type");
    std::shared_ptr<PasteboardObserverInstance> observer = nullptr;
    if (argc > ARGC_TYPE_SET1) {
        napi_typeof(env, argv[ARGC_TYPE_SET1], &valueType);
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        observer = GetPasteboardObserverIns(env, argv[ARGC_TYPE_SET1]);
    }
    if (!observer) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "Callback is invalid");
        return nullptr;
    }
    observer->setOff();
    PasteboardClient::GetInstance()->RemovePasteboardChangedObserver(observer->GetStub());
    DeletePasteboardObserverIns(observer);
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi off () is called!");
    return result;
}

napi_value SystemPasteboardNapi::Clear(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "Clear is called!");
    auto context = std::make_shared<AsyncCall::Context>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        NAPI_ASSERT_BASE(env, argc == 0 || argc == 1, " should 0 or 1 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec ---- Clear");
        PasteboardClient::GetInstance()->Clear();
    };
    context->SetAction(std::move(input));
    AsyncCall asyncCall(env, info, context, 0);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::HasPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "HasPasteData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    AsyncContext *asyncContext = new (std::nothrow) AsyncContext{ .env = env, .work = nullptr };
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
    napi_create_string_latin1(env, "HasPasteData", NAPI_AUTO_LENGTH, &resource);
    napi_status asyncWork = napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void *data) {
            AsyncContext *asyncContext = (AsyncContext *)data;
            asyncContext->result = PasteboardClient::GetInstance()->HasPasteData();
            asyncContext->status = 0;
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncContext *asyncContext = (AsyncContext *)data;
            napi_value result = nullptr;
            napi_get_boolean(env, asyncContext->result, &result);
            napi_value results[2] = { 0 };
            if (!asyncContext->status) {
                napi_get_undefined(env, &results[0]);
                results[1] = result;
            } else {
                napi_value message = nullptr;
                napi_create_string_utf8(env, "HasPasteData Fail", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &results[0]);
                napi_get_undefined(env, &results[1]);
            }
            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, results[1]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, results[0]);
                }
            } else {
                SetCallback(env, asyncContext->callbackRef, results);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
            asyncContext = nullptr;
        },
        (void *)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);
    if (asyncWork != napi_ok) {
        delete asyncContext;
        asyncContext = nullptr;
    }

    return promise;
}

napi_value SystemPasteboardNapi::GetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetPasteData is called!");
    size_t argc = ARGC_TYPE_SET1;
    napi_value argv[ARGC_TYPE_SET1] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));

    AsyncContext *asyncContext = new (std::nothrow) AsyncContext{
        .env = env,
        .work = nullptr,
    };
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
    napi_create_string_latin1(env, "GetPasteData", NAPI_AUTO_LENGTH, &resource);
    napi_status asyncWork = napi_create_async_work(
        env, nullptr, resource, [](napi_env env, void *data) {},
        [](napi_env env, napi_status status, void *data) {
            AsyncContext *asyncContext = (AsyncContext *)data;
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
            napi_value results[2] = { 0 };
            if (!asyncContext->status) {
                napi_get_undefined(env, &results[0]);
                results[1] = instance;
            } else {
                napi_value message = nullptr;
                napi_create_string_utf8(env, "GetPasteData Fail", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &results[0]);
                napi_get_undefined(env, &results[1]);
            }
            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, results[1]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, results[0]);
                }
            } else {
                SetCallback(env, asyncContext->callbackRef, results);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
            asyncContext = nullptr;
        },
        (void *)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);
    if (asyncWork != napi_ok) {
        delete asyncContext;
        asyncContext = nullptr;
    }

    return promise;
}

napi_value SystemPasteboardNapi::SetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SetPasteData is called!");
    size_t argc = ARGC_TYPE_SET2;
    napi_value argv[ARGC_TYPE_SET2] = { 0 };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

    AsyncContext *asyncContext = new (std::nothrow) AsyncContext{ .env = env, .work = nullptr };
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
    napi_create_string_latin1(env, "SetPasteData", NAPI_AUTO_LENGTH, &resource);
    napi_status asyncWork = napi_create_async_work(
        env, nullptr, resource,
        [](napi_env env, void *data) {
            AsyncContext *asyncContext = (AsyncContext *)data;
            PasteboardClient::GetInstance()->SetPasteData(*(asyncContext->obj->value_));
            asyncContext->status = 0;
        },
        [](napi_env env, napi_status status, void *data) {
            AsyncContext *asyncContext = (AsyncContext *)data;
            napi_value results[2] = { 0 };
            if (!asyncContext->status) {
                napi_get_undefined(env, &results[0]);
                napi_create_string_utf8(env, "SetPasteData Success", NAPI_AUTO_LENGTH, &results[1]);
            } else {
                napi_value message = nullptr;
                napi_create_string_utf8(env, "SetPasteData Fail", NAPI_AUTO_LENGTH, &message);
                napi_create_error(env, nullptr, message, &results[0]);
                napi_get_undefined(env, &results[1]);
            }
            if (asyncContext->deferred) {
                if (!asyncContext->status) {
                    napi_resolve_deferred(env, asyncContext->deferred, results[1]);
                } else {
                    napi_reject_deferred(env, asyncContext->deferred, results[0]);
                }
            } else {
                SetCallback(env, asyncContext->callbackRef, results);
                napi_delete_reference(env, asyncContext->callbackRef);
            }
            napi_delete_async_work(env, asyncContext->work);
            delete asyncContext;
            asyncContext = nullptr;
        },
        (void *)asyncContext, &asyncContext->work);
    napi_queue_async_work(env, asyncContext->work);
    if (asyncWork != napi_ok) {
        delete asyncContext;
        asyncContext = nullptr;
    }

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
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "proc.");
    // get native object
    SystemPasteboardNapi *obj = new (std::nothrow) SystemPasteboardNapi();
    if (!obj) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "New obj is null");
        return nullptr;
    }
    obj->env_ = env;
    NAPI_CALL(env, napi_wrap(env, thisVar, obj, SystemPasteboardNapi::Destructor,
                       nullptr, // finalize_hint
                       &obj->wrapper_));
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "end.");
    return thisVar;
}

napi_status SystemPasteboardNapi::NewInstance(napi_env env, napi_value &instance)
{
    napi_status status;

    napi_value constructor;
    status = napi_get_reference_value(env, g_systemPasteboard, &constructor);
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

std::shared_ptr<PasteboardObserverInstance> SystemPasteboardNapi::GetPasteboardObserverIns(const napi_env &env, const napi_value &currCallback)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "GetPasteboardObserverIns start");
    std::lock_guard<std::mutex> lock(pasteboardObserverInsMutex_);
    for (auto &[refKey, observerValue]: observers_) {
        napi_value callback = nullptr;
        napi_get_reference_value(env, refKey, &callback);
        bool isEqual = false;
        napi_strict_equals(env, currCallback, callback, &isEqual);
        if (isEqual) {
            return observerValue;
        }
    }
    return nullptr;
}

void SystemPasteboardNapi::DeletePasteboardObserverIns(const std::shared_ptr<PasteboardObserverInstance> &observer)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "DeletePasteboardObserverIns start");
    std::lock_guard<std::mutex> lock(pasteboardObserverInsMutex_);
    for (auto it = observers_.begin(); it != observers_.end(); ++it) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "DeletePasteboardObserverIns start111");
        if(it->second == observer) {
            observers_.erase(it);
            break;
        }
    }
}
} // namespace MiscServicesNapi
} // namespace OHOS