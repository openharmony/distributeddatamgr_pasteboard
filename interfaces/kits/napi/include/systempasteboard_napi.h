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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "pastedata_napi.h"
#include "pastedata_record_napi.h"
#include "pasteboard_observer.h"
#include "uri.h"

namespace OHOS {
namespace MiscServicesNapi {
napi_value PasteBoardInit(napi_env env, napi_value exports);

class PasteboardObserverInstance : public MiscServices::PasteboardObserver {
public:
    explicit PasteboardObserverInstance(const napi_env &env, const napi_ref &ref);
    ~PasteboardObserverInstance();

    virtual void OnPasteboardChanged() override;
    void setOff();
private:
    napi_env env_ = nullptr;
    napi_ref ref_ = nullptr;
    bool isOff_;
};

struct PasteboardDataWorker {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    bool isOff_;
};

class SystemPasteboardNapi {
public:
    static napi_value SystemPasteboardInit(napi_env env, napi_value exports);
    static napi_value New(napi_env env, napi_callback_info info);
    static napi_status NewInstance(napi_env env, napi_value &instance);
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);
    static void DeletePasteboardObserverIns(const napi_env &env, const napi_ref &ref);
    SystemPasteboardNapi();
    ~SystemPasteboardNapi();

private:
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value Clear(napi_env env, napi_callback_info info);
    static napi_value GetPasteData(napi_env env, napi_callback_info info);
    static napi_value SetPasteData(napi_env env, napi_callback_info info);
    static napi_value HasPasteData(napi_env env, napi_callback_info info);
    static std::shared_ptr<PasteboardObserverInstance> GetPasteboardObserverIns(const napi_ref &ref);

    std::shared_ptr<PasteDataNapi> value_;
    std::shared_ptr<MiscServices::PasteData> pasteData_;
    napi_env env_;
    napi_ref wrapper_;
    static std::map<napi_ref, std::shared_ptr<PasteboardObserverInstance>> observers_;
    static std::mutex pasteboardObserverInsMutex_;
};
} // MiscServicesNapi
} // OHOS
#endif