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

#include "config.h"
namespace OHOS::MiscServices {
bool Config::Component::Marshal(Serializable::json &node) const
{
    SetValue(node[GET_NAME(description)], description);
    SetValue(node[GET_NAME(lib)], lib);
    SetValue(node[GET_NAME(constructor)], constructor);
    SetValue(node[GET_NAME(destructor)], destructor);
    SetValue(node[GET_NAME(params)], params);
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
    SetValue(node[GET_NAME(processLabel)], processLabel);
    SetValue(node[GET_NAME(version)], version);
    SetValue(node[GET_NAME(features)], features);
    SetValue(node[GET_NAME(plugins)], plugins);
    SetValue(node[GET_NAME(components)], components);
    return true;
}

bool Config::Unmarshal(const Serializable::json &node)
{
    GetValue(node, GET_NAME(processLabel), processLabel);
    GetValue(node, GET_NAME(version), version);
    GetValue(node, GET_NAME(features), features);
    GetValue(node, GET_NAME(plugins), plugins);
    GetValue(node, GET_NAME(components), components);
    return true;
}
} // namespace OHOS::MiscServices