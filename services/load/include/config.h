/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_PASTEBOARD_SERVICES_LOAD_CONFIG_H
#define OHOS_PASTEBOARD_SERVICES_LOAD_CONFIG_H
#include "serializable/serializable.h"
namespace OHOS::MiscServices {
class Config final : public DistributedData::Serializable {
public:
    class Component final : public DistributedData::Serializable {
    public:
        std::string description = "";
        std::string lib = "";
        std::string constructor = "";
        std::string destructor = "";
        std::string params = "";
        bool Marshal(json &node) const override;
        bool Unmarshal(const json &node) override;
    };
    std::string processLabel;
    std::string version;
    std::vector<std::string> features;
    std::vector<std::string> plugins;
    std::vector<Component> components;
    bool Marshal(json &node) const override;
    bool Unmarshal(const json &node) override;
};
} // namespace OHOS::MiscServices
#endif // OHOS_PASTEBOARD_SERVICES_LOAD_CONFIG_H
