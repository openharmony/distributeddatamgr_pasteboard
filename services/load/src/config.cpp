/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "config.h"
namespace OHOS::MiscServices {
bool Config::Component::Marshal(Serializable::json &node) const
{
    SetValue(node, description, GET_NAME(description));
    SetValue(node, lib, GET_NAME(lib));
    SetValue(node, constructor, GET_NAME(constructor));
    SetValue(node, destructor, GET_NAME(destructor));
    SetValue(node, params, GET_NAME(params));
    return true;
}

bool Config::Component::Unmarshal(const Serializable::json &node)
{
    GetValue(node, GET_NAME(description), description);
    GetValue(node, GET_NAME(lib), lib);
    GetValue(node, GET_NAME(constructor), constructor);
    GetValue(node, GET_NAME(destructor), destructor);
    GetValue(node, GET_NAME(params), params);
    return true;
}

bool Config::Marshal(Serializable::json &node) const
{
    SetValue(node, processLabel, GET_NAME(processLabel));
    SetValue(node, version, GET_NAME(version));
    SetValue(node, features, GET_NAME(features));
    SetValue(node, plugins, GET_NAME(plugins));
    SetValue(node, components, GET_NAME(components));
    SetValue(node, bundles, GET_NAME(bundles));
    return true;
}

bool Config::Unmarshal(const Serializable::json &node)
{
    auto ret = GetValue(node, GET_NAME(processLabel), processLabel);
    ret = GetValue(node, GET_NAME(version), version) && ret;
    GetValue(node, GET_NAME(features), features);
    GetValue(node, GET_NAME(plugins), plugins);
    GetValue(node, GET_NAME(components), components);
    GetValue(node, GET_NAME(bundles), bundles);
    return ret;
}
} // namespace OHOS::MiscServices