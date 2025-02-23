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
#ifndef ENTITY_RECOGNITION_OBSERVER_PROXY_H
#define ENTITY_RECOGNITION_OBSERVER_PROXY_H

#include "i_entity_recognition_observer.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {
class EntityRecognitionObserverProxy : public IRemoteProxy<IEntityRecognitionObserver> {
public:
    explicit EntityRecognitionObserverProxy(const sptr<IRemoteObject> &object);
    ~EntityRecognitionObserverProxy() = default;
    DISALLOW_COPY_AND_MOVE(EntityRecognitionObserverProxy);
    void OnRecognitionEvent(EntityType entityType, std::string &entity) override;

private:
    static inline BrokerDelegator<EntityRecognitionObserverProxy> delegator_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // ENTITY_RECOGNITION_OBSERVER_PROXY_H
