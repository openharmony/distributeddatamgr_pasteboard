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
#include "pasteboard_hilog.h"
#include "paste_data_record.h"
#include "async_call.h"
#include "pasteboard_js_err.h"

using namespace OHOS::MiscServices;
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServicesNapi {
static thread_local napi_ref g_pasteDataRecord = nullptr;
const int ARGC_TYPE_SET0 = 0;
const int ARGC_TYPE_SET1 = 1;
constexpr int32_t  MIMETYPE_MAX_SIZE = 1024;

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
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
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
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
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
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreatePlainTextRecord(text);
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewPixelMapRecordInstance(
    napi_env env, const std::shared_ptr<PixelMap> pixelMap, napi_value &instance)
{
    if (pixelMap == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "pixelMap is nullptr.");
        return false;
    }

    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "unwrap failed.");
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreatePixelMapRecord(pixelMap);
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewUriRecordInstance(napi_env env, const std::string &text, napi_value &instance)
{
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
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

    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateWantRecord(want);
    obj->JSFillInstance(env, instance);
    return true;
}

bool PasteDataRecordNapi::NewKvRecordInstance(
    napi_env env, const std::string &mimeType, const std::vector<uint8_t> &arrayBuffer, napi_value &instance)
{
    NAPI_CALL_BASE(env, PasteDataRecordNapi::NewInstance(env, instance), false);
    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, instance, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "unwrap failed");
        return false;
    }
    obj->value_ = PasteboardClient::GetInstance()->CreateKvRecord(mimeType, arrayBuffer);
    obj->JSFillInstance(env, instance);
    return true;
}

void PasteDataRecordNapi::SetNamedPropertyByStr(
    napi_env env, napi_value &instance, const char *propName, const char *propValue)
{
    if (propName == nullptr || propValue == nullptr) {
        return;
    }
    napi_value prop = nullptr;
    if (napi_create_string_utf8(env, propValue, NAPI_AUTO_LENGTH, &prop) == napi_ok) {
        napi_set_named_property(env, instance, propName, prop);
    }
}

napi_value PasteDataRecordNapi::SetNapiKvData(napi_env env, std::shared_ptr<MineCustomData> customData)
{
    if (customData == nullptr) {
        return nullptr;
    }
    napi_value jsCustomData = nullptr;
    napi_create_object(env, &jsCustomData);
    auto itemData = customData->GetItemData();
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "size = %{public}d.", static_cast<uint32_t>(itemData.size()));
    for (auto &item : itemData) {
        void *data = nullptr;
        napi_value arrayBuffer = nullptr;
        size_t len = item.second.size();
        NAPI_CALL(env, napi_create_arraybuffer(env, len, &data, &arrayBuffer));
        if (memcpy_s(data, len, reinterpret_cast<const void *>(item.second.data()), len) != 0) {
            return nullptr;
        }
        NAPI_CALL(env, napi_set_named_property(env, jsCustomData, item.first.c_str(), arrayBuffer));
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "mimeType = %{public}s.", item.first.c_str());
    }
    return jsCustomData;
}

std::shared_ptr<MineCustomData> PasteDataRecordNapi::GetNativeKvData(napi_env env, napi_value napiValue)
{
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, napiValue, &valueType));
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type. Object expected.");

    napi_value mimeTypes = nullptr;
    NAPI_CALL(env, napi_get_property_names(env, napiValue, &mimeTypes));
    uint32_t mimeTypesNum = 0;
    NAPI_CALL(env, napi_get_array_length(env, mimeTypes, &mimeTypesNum));

    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "mimeTypesNum = %{public}u", mimeTypesNum);
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    for (uint32_t i = 0; i < mimeTypesNum; i++) {
        napi_value mimeTypeNapi = nullptr;
        NAPI_CALL(env, napi_get_element(env, mimeTypes, i, &mimeTypeNapi));

        std::string mimeType;
        bool ret = GetValue(env, mimeTypeNapi, mimeType);
        if (!ret || (mimeType.size() > MIMETYPE_MAX_SIZE) || mimeType == "") {
            return nullptr;
        }
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "mimeType = %{public}s,", mimeType.c_str());

        napi_value napiArrayBuffer = nullptr;
        NAPI_CALL(env, napi_get_property(env, napiValue, mimeTypeNapi, &napiArrayBuffer));
        void *data = nullptr;
        size_t dataLen;
        NAPI_CALL(env, napi_get_arraybuffer_info(env, napiArrayBuffer, &data, &dataLen));
        customData->AddItemData(mimeType,
            std::vector<uint8_t>(reinterpret_cast<uint8_t *>(data), reinterpret_cast<uint8_t *>(data) + dataLen));
    }
    return customData;
}

void PasteDataRecordNapi::JSFillInstance(napi_env env, napi_value &instance)
{
    if (value_ == nullptr) {
        return;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "mimeType = %{public}s.", value_->GetMimeType().c_str());

    auto mimeType = value_->GetMimeType();
    SetNamedPropertyByStr(env, instance, "mimeType", mimeType.c_str());

    auto plainText = value_->GetPlainText();
    if (plainText != nullptr) {
        SetNamedPropertyByStr(env, instance, "plainText", plainText->c_str());
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill plainText.");
    }

    auto htmlText = value_->GetHtmlText();
    if (htmlText != nullptr) {
        SetNamedPropertyByStr(env, instance, "htmlText", htmlText->c_str());
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill htmlText.");
    }

    auto uri = value_->GetUri();
    if (uri != nullptr) {
        SetNamedPropertyByStr(env, instance, "uri", uri->ToString().c_str());
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill uri.");
    }

    auto want = value_->GetWant();
    if (want != nullptr) {
        napi_value jsWant = OHOS::AppExecFwk::WrapWant(env, *want);
        napi_set_named_property(env, instance, "want", jsWant);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill want.");
    }

    auto pixelMap = value_->GetPixelMap();
    if (pixelMap != nullptr) {
        napi_value jsPixelMap = OHOS::Media::PixelMapNapi::CreatePixelMap(env, pixelMap);
        napi_set_named_property(env, instance, "pixelMap", jsPixelMap);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill pixelMap.");
    }

    auto customData = value_->GetCustomData();
    if (customData != nullptr) {
        napi_value jsCustomData = PasteDataRecordNapi::SetNapiKvData(env, customData);
        napi_set_named_property(env, instance, "data", jsCustomData);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "fill data.");
    }
}

napi_value PasteDataRecordNapi::ConvertToText(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ConvertToText is called!");

    struct ExeContext {
        std::string text;
        PasteDataRecordNapi *obj = nullptr;
    };
    auto exeContext = std::make_shared<ExeContext>();
    auto input = [exeContext](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        // convertToText(callback: AsyncCallback<string>) has 1 args
        if (argc > 0
            && !CheckArgsType(env, argv[0], napi_function, "Parameter error. The type of callback must be function.")) {
            return napi_invalid_arg;
        }
        PasteDataRecordNapi *obj = nullptr;
        napi_status status = napi_unwrap(env, self, reinterpret_cast<void **>(&obj));
        if (status == napi_ok || obj != nullptr) {
            exeContext->obj = obj;
        }
        return napi_ok;
    };

    auto exec = [exeContext](AsyncCall::Context *ctx) {
        if ((exeContext->obj != nullptr) && (exeContext->obj->value_ != nullptr)) {
            exeContext->text = exeContext->obj->value_->ConvertToText();
        }
    };
    auto output = [exeContext](napi_env env, napi_value *result) -> napi_status {
        napi_status status =
            napi_create_string_utf8(env, (exeContext->text).c_str(), (exeContext->text).length(), result);
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "napi_create_string_utf8 status = %{public}d", status);
        return status;
    };

    // 0: the AsyncCall at the first position;
    AsyncCall asyncCall(env, info, std::make_shared<AsyncCall::Context>(std::move(input), std::move(output)), 0);
    return asyncCall.Call(env, exec);
}

napi_value PasteDataRecordNapi::ConvertToTextV9(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ConvertToTextV9 is called!");
    return ConvertToText(env, info);
}

napi_value PasteDataRecordNapi::ToPlainText(napi_env env, napi_callback_info info)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "ToPlainText is called!");
    size_t argc = 1;
    napi_value argv[1] = { 0 };
    napi_value thisVar = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc >= ARGC_TYPE_SET0, "Wrong number of arguments");

    PasteDataRecordNapi *obj = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&obj));
    if ((status != napi_ok) || (obj == nullptr)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "Get ToPlainText object failed");
        return nullptr;
    }

    std::string str = obj->value_->ConvertToText();
    napi_value result = nullptr;
    napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &result);
    return result;
}

napi_value PasteDataRecordNapi::PasteDataRecordInit(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("convertToText", ConvertToText),
        DECLARE_NAPI_FUNCTION("convertToTextV9", ConvertToTextV9),
        DECLARE_NAPI_FUNCTION("toPlainText", ToPlainText)
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
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "PasteDataRecordInit create reference failed");
        return nullptr;
    }
    napi_set_named_property(env, exports, "PasteDataRecord", constructor);
    return exports;
}

void PasteDataRecordNapi::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    PasteDataRecordNapi *obj = static_cast<PasteDataRecordNapi *>(nativeObject);
    delete obj;
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
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "get reference failed");
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
