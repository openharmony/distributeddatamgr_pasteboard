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
#ifndef N_NAPI_PASTE_H
#define N_NAPI_PASTE_H

#include "async_call.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "pasteboard_observer.h"
#include "pastedata_napi.h"
#include "pastedata_record_napi.h"
#include "pixel_map_napi.h"
#include "uri.h"

namespace OHOS {
namespace MiscServicesNapi {
class PasteboardObserverInstance : public std::enable_shared_from_this<PasteboardObserverInstance> {
public:
    class PasteboardObserverImpl : public MiscServices::PasteboardObserver {
    public:
        explicit PasteboardObserverImpl() = default;
        void OnPasteboardChanged() override;
        void SetObserverWrapper(const std::shared_ptr<PasteboardObserverInstance> &observerInstance);

    private:
        std::weak_ptr<PasteboardObserverInstance> wrapper_;
    };

    explicit PasteboardObserverInstance(const napi_env &env, const napi_ref &ref);
    ~PasteboardObserverInstance();

    void OnPasteboardChanged();
    napi_env GetEnv()
    {
        return env_;
    }
    napi_ref GetRef()
    {
        return ref_;
    }
    sptr<PasteboardObserverImpl> GetStub();

private:
    napi_env env_ = nullptr;
    napi_ref ref_ = nullptr;
    sptr<PasteboardObserverImpl> stub_ = nullptr;
};

struct PasteboardDataWorker {
    std::shared_ptr<PasteboardObserverInstance> observer = nullptr;
};

struct HasContextInfo : public AsyncCall::Context {
    bool hasPasteData;
    napi_status status = napi_generic_failure;
    HasContextInfo() : Context(nullptr, nullptr){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct SetContextInfo : public AsyncCall::Context {
    std::shared_ptr<MiscServices::PasteData> obj;
    napi_status status = napi_generic_failure;
    SetContextInfo() : Context(nullptr, nullptr){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

struct GetContextInfo : public AsyncCall::Context {
    std::shared_ptr<MiscServices::PasteData> pasteData;
    napi_status status = napi_generic_failure;
    GetContextInfo() : Context(nullptr, nullptr){};

    napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
    {
        NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
        return Context::operator()(env, argc, argv, self);
    }
    napi_status operator()(napi_env env, napi_value *result) override
    {
        if (status != napi_ok) {
            return status;
        }
        return Context::operator()(env, result);
    }
};

class SystemPasteboardNapi {
public:
    static napi_value SystemPasteboardInit(napi_env env, napi_value exports);
    static napi_value New(napi_env env, napi_callback_info info);
    static napi_status NewInstance(napi_env env, napi_value &instance);
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);
    static void DeleteObserver(const std::shared_ptr<PasteboardObserverInstance> &observer);
    SystemPasteboardNapi();
    ~SystemPasteboardNapi();

private:
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value Clear(napi_env env, napi_callback_info info);
    static napi_value GetPasteData(napi_env env, napi_callback_info info);
    static napi_value SetPasteData(napi_env env, napi_callback_info info);
    static napi_value HasPasteData(napi_env env, napi_callback_info info);
    static napi_value ClearData(napi_env env, napi_callback_info info);
    static napi_value GetData(napi_env env, napi_callback_info info);
    static napi_value SetData(napi_env env, napi_callback_info info);
    static napi_value HasData(napi_env env, napi_callback_info info);
    static bool CheckAgrsOfOnAndOff(napi_env env, bool CheckArgsCount, napi_value *argv, size_t argc);
    static void SetObserver(napi_ref ref, std::shared_ptr<PasteboardObserverInstance> observer);
    static std::shared_ptr<PasteboardObserverInstance> GetObserver(napi_env env, napi_value observer);
    static void GetDataCommon(std::shared_ptr<GetContextInfo> &context);
    static void SetDataCommon(std::shared_ptr<SetContextInfo> &context);

    std::shared_ptr<PasteDataNapi> value_;
    std::shared_ptr<MiscServices::PasteData> pasteData_;
    napi_env env_;
    napi_ref wrapper_;
    static thread_local std::map<napi_ref, std::shared_ptr<PasteboardObserverInstance>> observers_;
};
} // namespace MiscServicesNapi
} // namespace OHOS
#endif