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
#include <thread>
#include <uv.h>

#include "common/block_object.h"
#include "napi_common.h"
#include "pasteboard_common.h"
#include "pasteboard_errcode.h"
#include "pasteboard_hilog_wreapper.h"
#include "systempasteboard_napi.h"
using namespace OHOS::MiscServices;
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServicesNapi {
static thread_local napi_ref g_systemPasteboard = nullptr;
std::mutex SystemPasteboardNapi::pasteboardObserverInsMutex_;
std::map<napi_ref, std::shared_ptr<PasteboardObserverInstance>> SystemPasteboardNapi::observers_;
constexpr size_t MAX_ARGS = 6;
constexpr int32_t STR_DATA_SIZE = 10;
const std::string STRING_UPDATE = "update";
constexpr int32_t MIMETYPE_MAX_SIZE = 1024;

PasteboardObserverInstance::PasteboardObserverInstance(const napi_env &env, const napi_ref &ref)
    : env_(env), ref_(ref)
{
    stub_ = new (std::nothrow) PasteboardObserverInstance::PasteboardObserverImpl();
}

PasteboardObserverInstance::~PasteboardObserverInstance()
{
    napi_delete_reference(env_, ref_);
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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");
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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

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

napi_value JScreateShareOption(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateShareOption is called!");

    napi_value jsShareOption = nullptr;
    napi_create_object(env, &jsShareOption);

    napi_value jsInApp = CreateNapiNumber(env, static_cast<int32_t>(ShareOption::InApp));
    NAPI_CALL(env, napi_set_named_property(env, jsShareOption, "InApp", jsInApp));

    napi_value jsLocalDevice = CreateNapiNumber(env, static_cast<int32_t>(ShareOption::LocalDevice));
    NAPI_CALL(env, napi_set_named_property(env, jsShareOption, "LocalDevice", jsLocalDevice));

    napi_value jsCrossDevice = CreateNapiNumber(env, static_cast<int32_t>(ShareOption::CrossDevice));
    NAPI_CALL(env, napi_set_named_property(env, jsShareOption, "CrossDevice", jsCrossDevice));

    return jsShareOption;
}

napi_value JScreatePlainTextRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreatePlainTextRecord is called!");
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");
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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

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

bool ParseKvData(napi_env env, napi_callback_info info, std::string &mimeType, std::vector<uint8_t> &arrayBuffer)
{
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = {0};
    napi_value thisVar = nullptr;

    NAPI_CALL_BASE(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL), false);
    NAPI_ASSERT_BASE(env, argc > 1, "Wrong number of arguments", false);

    napi_valuetype valueType = napi_undefined;
    bool result = false;
    NAPI_CALL_BASE(env, napi_typeof(env, argv[0], &valueType), false);
    NAPI_ASSERT_BASE(env, valueType == napi_string, "Wrong argument type", false);
    NAPI_CALL_BASE(env, napi_is_arraybuffer(env, argv[1], &result), false);
    NAPI_ASSERT_BASE(env, result, "Wrong argument type", false);

    void *data = nullptr;
    size_t dataLen = 0;
    bool ret = MiscServicesNapi::GetValue(env, argv[0], mimeType);
    if (ret != true || mimeType.size() > MIMETYPE_MAX_SIZE) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "GetValue failed, ret = %{public}d!", ret);
        return false;
    }
    NAPI_CALL_BASE(env, napi_get_arraybuffer_info(env, argv[1], &data, &dataLen), false);
    std::vector<uint8_t> arrayBuf(reinterpret_cast<uint8_t *>(data), reinterpret_cast<uint8_t *>(data) + dataLen);
    arrayBuffer = std::move(arrayBuf);
    return true;
}

napi_value JSCreateKvRecord(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JSCreateKvRecord is called!");

    std::string mimeType;
    std::vector<uint8_t> arrayBuffer;
    if (!ParseKvData(env, info, mimeType, arrayBuffer)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "parseKvData failed!");
        return nullptr;
    }
    napi_value instance = nullptr;
    PasteDataRecordNapi::NewKvRecordInstance(env, mimeType, arrayBuffer, instance);
    return instance;
}

napi_value JScreateHtmlData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JScreateHtmlData is called!");
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");
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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");
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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");
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

napi_value JSCreateKvData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JSCreateKvData is called!");

    std::string mimeType;
    std::vector<uint8_t> arrayBuffer;
    if (!ParseKvData(env, info, mimeType, arrayBuffer)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "parseKvData failed!");
        return nullptr;
    }

    napi_value instance = nullptr;
    NAPI_CALL(env, PasteDataNapi::NewInstance(env, instance));
    PasteDataNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "unwrap failed!");
        return nullptr;
    }

    obj->value_ = PasteboardClient::GetInstance()->CreateKvData(mimeType, arrayBuffer);
    return instance;
}
napi_value JSgetSystemPasteboard(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "JSgetSystemPasteboard is called!");
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    napi_value instance = nullptr;
    napi_status status = SystemPasteboardNapi::NewInstance(env, instance); // 0 arguments
    if (status != napi_ok) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "JSgetSystemPasteboard create instance failed");
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
        DECLARE_NAPI_FUNCTION("createData", JSCreateKvData),
        DECLARE_NAPI_FUNCTION("createHtmlTextRecord", JScreateHtmlTextRecord),
        DECLARE_NAPI_FUNCTION("createWantRecord", JScreateWantRecord),
        DECLARE_NAPI_FUNCTION("createPlainTextRecord", JScreatePlainTextRecord),
        DECLARE_NAPI_FUNCTION("createPixelMapRecord", JScreatePixelMapRecord),
        DECLARE_NAPI_FUNCTION("createUriRecord", JScreateUriRecord),
        DECLARE_NAPI_FUNCTION("createRecord", JSCreateKvRecord),
        DECLARE_NAPI_FUNCTION("getSystemPasteboard", JSgetSystemPasteboard),
        DECLARE_NAPI_GETTER("ShareOption", JScreateShareOption),
        DECLARE_NAPI_PROPERTY("MAX_RECORD_NUM", CreateNapiNumber(env, MAX_RECORD_NUM)),
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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = 0;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    NAPI_ASSERT(env, argc > 1, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE] = { 0 };
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE, &strLen));
    NAPI_ASSERT(env, strLen == STRING_UPDATE.length(), "error type");

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    // 1: the callback function
    napi_typeof(env, argv[1], &valueType);
    NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
    auto observer = GetObserver(env, argv[1]);
    if (observer != nullptr) {
        return result;
    }

    napi_ref ref = nullptr;
    napi_create_reference(env, argv[1], 1, &ref);
    observer = std::make_shared<PasteboardObserverInstance>(env, ref);
    observer->GetStub()->SetObserverWrapper(observer);
    PasteboardClient::GetInstance()->AddPasteboardChangedObserver(observer->GetStub());
    SetObserver(ref, observer);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi on() is end!");
    return result;
}

napi_value SystemPasteboardNapi::Off(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SystemPasteboardNapi off () is called!");
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
    napi_value thisVar = 0;
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
    // 0: is event 1: is parameter
    NAPI_ASSERT(env, argc > 0, "Wrong number of arguments");

    size_t strLen = 0;
    char str[STR_DATA_SIZE] = { 0 };
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    NAPI_ASSERT(env, valueType == napi_string, "Wrong argument type. String expected.");
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], str, STR_DATA_SIZE, &strLen));
    NAPI_ASSERT(env, strLen == STRING_UPDATE.length(), "error type");
    std::shared_ptr<PasteboardObserverInstance> observer = nullptr;
    // 1: is the observer parameter
    if (argc > 1) {
        napi_typeof(env, argv[1], &valueType);
        NAPI_ASSERT(env, valueType == napi_function, "Wrong argument type. Function expected.");
        observer = GetObserver(env, argv[1]);
    }

    DeleteObserver(observer);
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
        // 0: clear has no args
        NAPI_ASSERT_BASE(env, argc == 0, " should 0 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec Clear");
        PasteboardClient::GetInstance()->Clear();
    };
    context->SetAction(std::move(input));
    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, context, 0);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::HasPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "HasPasteData is called!");
    auto context = std::make_shared<HasContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        // 0: hasPasteData has no args
        NAPI_ASSERT_BASE(env, argc == 0, " should 0 parameters!", napi_invalid_arg);
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->hasPasteData, result);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "napi_get_boolean status = %{public}d", status);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "exec HasPasteData");
        context->hasPasteData = PasteboardClient::GetInstance()->HasPasteData();
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "HasPasteData result = %{public}d", context->hasPasteData);
        context->status = napi_ok;
    };
    context->SetAction(std::move(input), std::move(output));
    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, context, 0);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::GetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "GetPasteData is called!");
    struct ExeContext {
        std::shared_ptr<PasteData> pasteData;
        std::shared_ptr<BlockObject<uint32_t>> block;
        uint32_t errCode = napi_ok;
    };
    auto context = std::make_shared<ExeContext>();
    context->pasteData = std::make_shared<PasteData>();
    auto input = [](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        // 0: getPasteData has no args
        NAPI_ASSERT_BASE(env, argc == 0, " should 0 parameters!", napi_invalid_arg);
        return napi_ok;
    };

    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_value instance = nullptr;
        PasteDataNapi::NewInstance(env, instance);
        PasteDataNapi *obj = nullptr;
        napi_status ret = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
        if ((ret == napi_ok) || (obj != nullptr)) {
            obj->value_ = context->pasteData;
        } else {
            return napi_generic_failure;
        }
        *result = instance;
        return napi_ok;
    };

    auto exec = [context](AsyncCall::Context *ctx) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetPasteData Begin");
        auto success = PasteboardClient::GetInstance()->GetPasteData(*context->pasteData);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "GetPasteData End");
        context->errCode = success ? E_SUCCESS : E_ERROR;
    };
    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, std::make_shared<AsyncCall::Context>(std::move(input), std::move(output)), 0);
    return asyncCall.Call(env, exec);
}

napi_value SystemPasteboardNapi::SetPasteData(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "SetPasteData is called!");
    auto context = std::make_shared<SetContextInfo>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        // 1: setPasteData has 1 arg
        NAPI_ASSERT_BASE(env, argc == 1, " should 1 parameters!", napi_invalid_arg);
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[0], &valueType);
        NAPI_ASSERT_BASE(env, valueType == napi_object, "Wrong argument type. Object expected.", napi_invalid_arg);
        PasteDataNapi *pasteData = nullptr;
        napi_unwrap(env, argv[0], reinterpret_cast<void **>(&pasteData));
        if (pasteData != nullptr) {
            context->obj = pasteData->value_;
        }
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "exec SetPasteData");
        if (context->obj != nullptr) {
            PasteboardClient::GetInstance()->SetPasteData(*(context->obj));
            context->obj = nullptr;
        }
        context->status = napi_ok;
    };
    context->SetAction(std::move(input));
    // 1: the AsyncCall at the second position
    AsyncCall asyncCall(env, info, context, 1);
    return asyncCall.Call(env, exec);
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
    size_t argc = MAX_ARGS;
    napi_value argv[MAX_ARGS] = { 0 };
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

std::shared_ptr<PasteboardObserverInstance> SystemPasteboardNapi::GetObserver(napi_env env, napi_value observer)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "GetObserver start");
    std::lock_guard<std::mutex> lock(pasteboardObserverInsMutex_);
    for (auto &[refKey, observerValue] : observers_) {
        napi_value callback = nullptr;
        napi_get_reference_value(env, refKey, &callback);
        bool isEqual = false;
        napi_strict_equals(env, observer, callback, &isEqual);
        if (isEqual) {
            return observerValue;
        }
    }
    return nullptr;
}

void SystemPasteboardNapi::SetObserver(napi_ref ref, std::shared_ptr<PasteboardObserverInstance> observer)
{
    std::lock_guard<std::mutex> lock(pasteboardObserverInsMutex_);
    observers_[ref] = observer;
}

void SystemPasteboardNapi::DeleteObserver(const std::shared_ptr<PasteboardObserverInstance> &observer)
{
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "DeleteObserver start");
    std::vector<std::shared_ptr<PasteboardObserverInstance>> observers;
    {
        std::lock_guard<std::mutex> lock(pasteboardObserverInsMutex_);
        for (auto it = observers_.begin(); it != observers_.end(); ++it) {
            if (it->second == observer) {
                observers.push_back(observer);
                observers_.erase(it);
                break;
            }
            if (observer == nullptr) {
                observers.push_back(it->second);
                it = observers_.erase(it);
            }
        }
    }
    for (auto &delObserver : observers) {
        PasteboardClient::GetInstance()->RemovePasteboardChangedObserver(delObserver->GetStub());
    }
}

void PasteboardObserverInstance::PasteboardObserverImpl::OnPasteboardChanged()
{
    std::shared_ptr<PasteboardObserverInstance> observerInstance(wrapper_.lock());
    if (observerInstance == nullptr) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_JS_NAPI, "expired callback");
        return;
    }
    observerInstance->OnPasteboardChanged();
}

void PasteboardObserverInstance::PasteboardObserverImpl::SetObserverWrapper(
    const std::shared_ptr<PasteboardObserverInstance>& observerInstance)
{
    wrapper_ = observerInstance;
}
} // namespace MiscServicesNapi
} // namespace OHOS