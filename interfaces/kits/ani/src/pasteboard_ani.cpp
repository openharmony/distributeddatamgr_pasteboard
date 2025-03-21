/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <ani.h>
#include <array>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <thread>
#include "pasteboard_hilog.h"
#include "pasteboard_client.h"
#include "common/block_object.h"
#include "ani_common_want.h"
#include "image_ani_utils.h"
#include "pasteboard_ani_utils.h"

using namespace OHOS::MiscServices;

constexpr size_t SYNC_TIMEOUT = 3500;
ani_object g_systemboard_obj = nullptr;

ani_object Create([[maybe_unused]] ani_env *env, std::shared_ptr<PasteData> &ptrPasteData)
{
    static const char *className = "L@ohos/pasteboard/PasteDataImpl;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        std::cerr << "Not found '" << className << "'" << std::endl;
        return nullptr;
    }

    auto shareptrPasteData = new SharedPtrHolder<PasteData>(ptrPasteData);
    return NativeObjectWrapper<SharedPtrHolder<PasteData>>::Wrap(env, cls, shareptrPasteData);
}

PasteData* unwrapAndGetPasteDataPtr([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
{
    auto shareptrPasteData = NativeObjectWrapper<SharedPtrHolder<PasteData>>::Unwrap(env, object);
    return shareptrPasteData->Get().get();
}

ani_object CreateObjectFromClass([[maybe_unused]] ani_env *env, const char* className)
{
    ani_object obj = nullptr;
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreateObjectFromClass] Not found class. ");
        return obj;
    }
    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreateObjectFromClass] get ctor Failed. ");
        return obj;
    }
    if (ANI_OK != env->Object_New(cls, ctor, &obj)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreateObjectFromClass] Create Object Failed. ");
        return obj;
    }

    return obj;
}

std::string GetStdStringFromUnion([[maybe_unused]] ani_env *env, ani_object union_obj)
{
    UnionAccessor unionAccessor(env, union_obj);
    ani_string str;
    if (!unionAccessor.TryConvert<ani_string>(str)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI,
            "[GetStdStringFromUnion] try to convert union object to ani_string failed! ");
        return nullptr;
    }

    return ANIUtils_ANIStringToStdString(env, str);
}

static ani_double GetRecordCount([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
{
    PasteData* pPasteData = unwrapAndGetPasteDataPtr(env, object);
    if (pPasteData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetRecordCount] pPasteData is null");
        return 0;
    }

    return pPasteData->GetRecordCount();
}

static void AddRecord([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object, ani_object record)
{
    ani_ref uri;
    if (ANI_OK != env->Object_GetPropertyByName_Ref(static_cast<ani_object>(record), "uri", &uri)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[AddRecord] Object_GetPropertyByName_Ref Faild");
        return ;
    }

    auto uri_str = ANIUtils_ANIStringToStdString(env, static_cast<ani_string>(uri));

    PasteData* pPasteData = unwrapAndGetPasteDataPtr(env, object);
    if (pPasteData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[AddRecord] pPasteData is null");
        return;
    }

    PasteDataRecord::Builder builder("");
    builder.SetUri(std::make_shared<OHOS::Uri>(OHOS::Uri(uri_str)));
    std::shared_ptr<PasteDataRecord> result = builder.Build();
    if (result == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[AddRecord] result is null");
        return;
    }
    pPasteData->AddRecord(*(result.get()));
}

static ani_object GetRecord([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object,
    [[maybe_unused]] ani_double index)
{
    PasteData* pPasteData = unwrapAndGetPasteDataPtr(env, object);
    if (pPasteData == nullptr || index >= pPasteData->GetRecordCount()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetRecord] pPasteData is null or index is out of range.");
        return nullptr;
    }

    std::shared_ptr<PasteDataRecord> recordFromBottom = pPasteData->GetRecordAt(static_cast<std::size_t>(index));
    if (recordFromBottom == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetRecord] recordFromBottom is null.");
        return nullptr;
    }
    ani_string uri_string{};
    env->String_NewUTF8(recordFromBottom->GetUri()->ToString().c_str(),
        recordFromBottom->GetUri()->ToString().length(), &uri_string);

    ani_object record = CreateObjectFromClass(env, "L@ohos/pasteboard/PasteDataRecordImpl;");
    if (record == nullptr) {
        return nullptr;
    }

    ani_class cls;
    if (ANI_OK != env->FindClass("L@ohos/pasteboard/PasteDataRecordImpl;", &cls)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetRecord] Not found. ");
        return record;
    }

    ani_method uriSetter;
    if (ANI_OK != env->Class_FindMethod(cls, "<set>uri", nullptr, &uriSetter)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetRecord] Class_FindMethod Fail. ");
        return record;
    }
    if (ANI_OK != env->Object_CallMethod_Void(record, uriSetter, uri_string)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetRecord] Object_CallMethod_Void Fail. ");
        return record;
    }

    return record;
}

static void SetProperty([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object, ani_object property)
{
    PasteData* pPasteData = unwrapAndGetPasteDataPtr(env, object);
    if (pPasteData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[SetProperty] pPasteData is null. ");
        return;
    }

    ani_double shareOptionValue;
    if (ANI_OK != env->Object_GetPropertyByName_Double(static_cast<ani_object>(property),
        "shareOption", &shareOptionValue)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[SetProperty] Object_GetPropertyByName_Int Faild. ");
        return;
    }

    ani_int localOnlyValue = shareOptionValue == ShareOption::CrossDevice ? false : true;
    pPasteData->SetLocalOnly(localOnlyValue);
    pPasteData->SetShareOption(static_cast<ShareOption>(shareOptionValue));

    return;
}

static ani_object GetProperty([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
{
    PasteData* pPasteData = unwrapAndGetPasteDataPtr(env, object);
    if (pPasteData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetProperty] pPasteData is null. ");
        return nullptr;
    }

    PasteDataProperty property = pPasteData->GetProperty();
    ani_double shareOpionValue = property.shareOption;

    ani_object propertyToAbove = CreateObjectFromClass(env, "L@ohos/pasteboard/PasteDataPropertyImpl;");
    if (propertyToAbove == nullptr) {
        return nullptr;
    }

    ani_class cls;
    if (ANI_OK != env->FindClass("L@ohos/pasteboard/PasteDataPropertyImpl;", &cls)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetProperty] Not found class. ");
        return propertyToAbove;
    }

    ani_method shareOptionSetter;
    if (ANI_OK != env->Class_FindMethod(cls, "<set>shareOption", nullptr, &shareOptionSetter)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetProperty] Class_FindMethod Fail. ");
        return propertyToAbove;
    }
    if (ANI_OK != env->Object_CallMethod_Void(propertyToAbove, shareOptionSetter, shareOpionValue)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetProperty] Object_CallMethod_Void Fail. ");
        return propertyToAbove;
    }

    return propertyToAbove;
}

static ani_object CreateHtmlData([[maybe_unused]] ani_env *env, ani_object union_obj)
{
    auto value_str = GetStdStringFromUnion(env, union_obj);
    auto ptr = PasteboardClient::GetInstance()->CreateHtmlData(value_str);
    if (ptr == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreateHtmlData] CreateHtmlData failed ");
        return nullptr;
    }
    ani_object PasteDataImpl = Create(env, ptr);

    return PasteDataImpl;
}

static ani_object CreatePlainTextData([[maybe_unused]] ani_env *env, ani_object union_obj)
{
    auto value_str = GetStdStringFromUnion(env, union_obj);
    auto ptr = PasteboardClient::GetInstance()->CreatePlainTextData(value_str);
    if (ptr == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreatePlainTextData] CreatePlainTextData failed. ");
        return nullptr;
    }
    ani_object PasteDataImpl = Create(env, ptr);

    return PasteDataImpl;
}

static ani_object CreateUriData([[maybe_unused]] ani_env *env, ani_object union_obj)
{
    auto value_str = GetStdStringFromUnion(env, union_obj);
    auto ptr = PasteboardClient::GetInstance()->CreateUriData(OHOS::Uri(value_str));
    if (ptr == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreateUriData] CreateUriData failed. ");
        return nullptr;
    }
    ani_object PasteDataImpl = Create(env, ptr);

    return PasteDataImpl;
}

static ani_object CreatePixelMapData([[maybe_unused]] ani_env *env, ani_object union_obj)
{
    OHOS::Media::PixelMap* rawPixelMap = OHOS::Media::ImageAniUtils::GetPixelMapFromEnv(env, union_obj);
    if (rawPixelMap == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreatePixelMapData] GetPixelMapFromEnv failed. ");
        return nullptr;
    }

    auto pixelMap = std::shared_ptr<OHOS::Media::PixelMap>(rawPixelMap);
    auto ptr = PasteboardClient::GetInstance()->CreatePixelMapData(pixelMap);
    ani_object PasteDataImpl = Create(env, ptr);

    return PasteDataImpl;
}

static ani_object CreateWantData([[maybe_unused]] ani_env *env, ani_object union_obj)
{
    OHOS::AAFwk::Want want;
    bool ret = OHOS::AppExecFwk::UnwrapWant(env, union_obj, want);
    if (!ret) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreateWantData] UnwrapWant failed. ");
        return nullptr;
    }

    auto ptr = PasteboardClient::GetInstance()->CreateWantData(std::make_shared<OHOS::AAFwk::Want>(want));
    if (ptr == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreateWantData] CreateWantData failed. ");
        return nullptr;
    }
    ani_object PasteDataImpl = Create(env, ptr);

    return PasteDataImpl;
}

ani_object processSpecialMimeType([[maybe_unused]] ani_env *env, ani_object union_obj, std::string type)
{
    if (type == "text/html") {
        return CreateHtmlData(env, union_obj);
    } else if (type == "text/plain") {
        return CreatePlainTextData(env, union_obj);
    } else if (type == "text/uri") {
        return CreateUriData(env, union_obj);
    } else if (type == "pixelMap") {
        return CreatePixelMapData(env, union_obj);
    } else if (type == "text/want") {
        return CreateWantData(env, union_obj);
    }

    return nullptr;
}

static ani_object CreateDataTypeValue([[maybe_unused]] ani_env *env, ani_string type, ani_object union_obj)
{
    auto type_str = ANIUtils_ANIStringToStdString(env, static_cast<ani_string>(type));
    if (type_str == "text/html" || type_str == "text/plain" ||
        type_str == "text/uri" || type_str == "pixelMap" || type_str == "text/want") {
        return processSpecialMimeType(env, union_obj, type_str);
    }

    std::string classname("Lescompat/ArrayBuffer;");
    bool isArrayBuffer = ANIUtils_UnionIsInstanceOf(env, union_obj, classname);
    if (!isArrayBuffer) {
        return nullptr;
    }

    void* data;
    size_t length;
    if (ANI_OK != env->ArrayBuffer_GetInfo(static_cast<ani_arraybuffer>(union_obj), &data, &length)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI,
            "[CreateDataTypeValue] Failed: env->ArrayBuffer_GetInfo(). ");
        return nullptr;
    }

    auto pVal = static_cast<uint8_t*>(data);
    std::vector<uint8_t> vec(pVal, pVal + length);

    auto ptr = PasteboardClient::GetInstance()->CreateKvData(type_str, vec);
    if (ptr == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[CreateDataTypeValue] CreateKvData failed. ");
        return nullptr;
    }
    ani_object PasteDataImpl = Create(env, ptr);

    return PasteDataImpl;
}

static ani_object CreateDataRecord([[maybe_unused]] ani_env *env, ani_object record)
{
    return nullptr;
}

static ani_object GetSystemPasteboard([[maybe_unused]] ani_env *env)
{
    if (g_systemboard_obj != nullptr) {
        return g_systemboard_obj;
    }

    static const char *className = "L@ohos/pasteboard/SystemPasteboardImpl;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetSystemPasteboard] Not found classname. ");
        return g_systemboard_obj;
    }

    ani_method ctor;
    if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetSystemPasteboard] get ctor Failed. ");
        return g_systemboard_obj;
    }

    if (ANI_OK != env->Object_New(cls, ctor, &g_systemboard_obj)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[GetSystemPasteboard] Create Object Failed. ");
        return g_systemboard_obj;
    }

    return g_systemboard_obj;
}

static ani_boolean HasDataType([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object, ani_string mimeType)
{
    auto mimeType_str = ANIUtils_ANIStringToStdString(env, static_cast<ani_string>(mimeType));

    auto block = std::make_shared<OHOS::BlockObject<std::shared_ptr<int>>>(SYNC_TIMEOUT);
    std::thread thread([block, mimeType_str]() {
        auto ret = PasteboardClient::GetInstance()->HasDataType(mimeType_str);
        std::shared_ptr<int> value = std::make_shared<int>(static_cast<int>(ret));
        block->SetValue(value);
    });
    thread.detach();
    auto value = block->GetValue();

    return *value;
}

static ani_object GetDataSync([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object object)
{
    return nullptr;
}

ANI_EXPORT ani_status ANI_Constructor_Namespace(ani_env *env)
{
    ani_namespace ns;
    static const char *nameSpaceName = "L@ohos/pasteboard/pasteboard;";
    if (ANI_OK != env->FindNamespace(nameSpaceName, &ns)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI,
            "[ANI_Constructor_Namespace] Not found namespace: %s", nameSpaceName);
        return ANI_NOT_FOUND;
    }

    std::array methods = {
        ani_native_function {"createDataTypeValue", nullptr, reinterpret_cast<void *>(CreateDataTypeValue)},
        ani_native_function {"createDataRecord", nullptr, reinterpret_cast<void *>(CreateDataRecord)},
        ani_native_function {"getSystemPasteboard", nullptr, reinterpret_cast<void *>(GetSystemPasteboard)},
    };

    if (ANI_OK != env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI,
            "[ANI_Constructor_Namespace] Cannot bind native methods to %s", nameSpaceName);
        return ANI_ERROR;
    };

    return ANI_OK;
}

ANI_EXPORT ani_status ANI_Constructor_PasteData(ani_env *env)
{
    static const char *className = "L@ohos/pasteboard/PasteDataImpl;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[ANI_Constructor_PasteData] Not found %s", className);
        return ANI_NOT_FOUND;
    }

    std::array methods = {
        ani_native_function {"addRecord", nullptr, reinterpret_cast<void *>(AddRecord)},
        ani_native_function {"getRecordCount", nullptr, reinterpret_cast<void *>(GetRecordCount)},
        ani_native_function {"getRecord", nullptr, reinterpret_cast<void *>(GetRecord)},
        ani_native_function {"setProperty", nullptr, reinterpret_cast<void *>(SetProperty)},
        ani_native_function {"getProperty", nullptr, reinterpret_cast<void *>(GetProperty)},
    };

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI,
            "[ANI_Constructor_PasteData] Cannot bind native methods to %s", className);
        return ANI_ERROR;
    };

    return ANI_OK;
}

ANI_EXPORT ani_status ANI_Constructor_SystemPasteboard(ani_env *env)
{
    static const char *className = "L@ohos/pasteboard/SystemPasteboardImpl;";
    ani_class cls;
    if (ANI_OK != env->FindClass(className, &cls)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[ANI_Constructor_SystemPasteboard] Not found %s", className);
        return ANI_NOT_FOUND;
    }

    std::array methods = {
        ani_native_function {"hasDataType", nullptr, reinterpret_cast<void *>(HasDataType)},
        ani_native_function {"getDataSync", nullptr, reinterpret_cast<void *>(GetDataSync)},
    };

    if (ANI_OK != env->Class_BindNativeMethods(cls, methods.data(), methods.size())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI,
            "[ANI_Constructor_SystemPasteboard] Cannot bind native methods to %s", className);
        return ANI_ERROR;
    };

    return ANI_OK;
}

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "[ANI_Constructor] Unsupported ANI_VERSION_1");
        std::cerr << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_ERROR;
    }

    ani_status namespaceStatus = ANI_Constructor_Namespace(env);
    if (namespaceStatus != ANI_OK) {
        return namespaceStatus;
    }

    ani_status pasteDataStatus = ANI_Constructor_PasteData(env);
    if (pasteDataStatus != ANI_OK) {
        return pasteDataStatus;
    }

    ani_status systemboardStatus = ANI_Constructor_SystemPasteboard(env);
    if (systemboardStatus != ANI_OK) {
        return systemboardStatus;
    }

    *result = ANI_VERSION_1;
    return ANI_OK;
}
