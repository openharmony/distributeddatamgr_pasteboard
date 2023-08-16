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

#include "serializable/serializable.h"
namespace OHOS {
namespace DistributedData {
Serializable::json Serializable::Marshall() const
{
    json root;
    Marshal(root);
    return root;
}

bool Serializable::Unmarshall(const std::string &jsonStr)
{
    json *jsonObj = cJSON_Parse(jsonStr.c_str());
    if (jsonObj == nullptr) {
        // if the string size is less than 1, means the string is invalid.
        if (jsonStr.empty()) {
            return false;
        }
        jsonObj = cJSON_Parse(jsonStr.substr(1).c_str()); // drop first char to adapt A's value;
        if (jsonObj == nullptr) {
            return false;
        }
    }
    return Unmarshal(*jsonObj);
}

Serializable::json Serializable::ToJson(const std::string &jsonStr)
{
    json *jsonObj = cJSON_Parse(jsonStr.c_str());
    if (jsonObj == nullptr) {
        // if the string size is less than 1, means the string is invalid.
        if (jsonStr.empty()) {
            return *cJSON_CreateNull();
        }
        jsonObj = cJSON_Parse(jsonStr.substr(1).c_str()); // drop first char to adapt A's value;
        if (jsonObj == nullptr) {
            return *cJSON_CreateNull();
        }
    }
    return *jsonObj;
}

bool Serializable::GetValue(const json &node, const std::string &name, std::string &value)
{
    auto &subNode = GetSubNode(node, name);
    if (cJSON_IsNull(&subNode) || !cJSON_IsString(&subNode)) {
        return false;
    }
    value = cJSON_GetStringValue(&subNode);
    return true;
}

bool Serializable::GetValue(const json &node, const std::string &name, uint32_t &value)
{
    auto &subNode = GetSubNode(node, name);
    if (cJSON_IsNull(&subNode) || !cJSON_IsNumber(&subNode)) {
        return false;
    }
    value = cJSON_GetNumberValue(&subNode);
    return true;
}

bool Serializable::GetValue(const json &node, const std::string &name, int32_t &value)
{
    auto &subNode = GetSubNode(node, name);
    if (cJSON_IsNull(&subNode) || !cJSON_IsNumber(&subNode)) {
        return false;
    }
    value = cJSON_GetNumberValue(&subNode);
    return true;
}

bool Serializable::GetValue(const json &node, const std::string &name, int64_t &value)
{
    auto &subNode = GetSubNode(node, name);
    if (cJSON_IsNull(&subNode) || !cJSON_IsNumber(&subNode)) {
        return false;
    }
    value = cJSON_GetNumberValue(&subNode);
    return true;
}

bool Serializable::GetValue(const json &node, const std::string &name, bool &value)
{
    auto &subNode = GetSubNode(node, name);
    if (cJSON_IsNull(&subNode) || !cJSON_IsBool(&subNode)) {
        return false;
    }
    value = cJSON_IsTrue(&subNode);
    return true;
}

bool Serializable::GetValue(const json &node, const std::string &name, std::vector<uint8_t> &value)
{
    auto &subNode = GetSubNode(node, name);
    if (cJSON_IsNull(&subNode) || !cJSON_IsArray(&subNode)) {
        return false;
    }
    for (int i = 0; i < cJSON_GetArraySize(&subNode); i++) {
        value.emplace_back(cJSON_GetNumberValue(cJSON_GetArrayItem(&subNode, i)));
    }
    return true;
}

bool Serializable::GetValue(const json &node, const std::string &name, Serializable &value)
{
    auto &subNode = GetSubNode(node, name);
    if (cJSON_IsNull(&subNode) || !cJSON_IsObject(&subNode)) {
        return false;
    }
    return value.Unmarshal(subNode);
}

bool Serializable::SetValue(json &node, const std::string &value, const std::string &name, const bool &isObject)
{
    json *subNode = cJSON_CreateString(value.c_str());
    if (!isObject) {
        node = *subNode;
        delete subNode;
        return true;
    }
    SetValueCommon(node, *subNode, name);
    return true;
}

bool Serializable::SetValue(json &node, const uint32_t &value, const std::string &name, const bool &isObject)
{
    json *subNode = cJSON_CreateNumber(value);
    if (!isObject) {
        node = *subNode;
        delete subNode;
        return true;
    }
    SetValueCommon(node, *subNode, name);
    return true;
}

bool Serializable::SetValue(json &node, const int32_t &value, const std::string &name, const bool &isObject)
{
    json *subNode = cJSON_CreateNumber(value);
    if (!isObject) {
        node = *subNode;
        delete subNode;
        return true;
    }
    SetValueCommon(node, *subNode, name);
    return true;
}

bool Serializable::SetValue(json &node, const int64_t &value, const std::string &name, const bool &isObject)
{
    json *subNode = cJSON_CreateNumber(value);
    if (!isObject) {
        node = *subNode;
        delete subNode;
        return true;
    }
    SetValueCommon(node, *subNode, name);
    return true;
}

bool Serializable::SetValue(json &node, const bool &value, const std::string &name, const bool &isObject)
{
    json *subNode = cJSON_CreateBool(value);
    if (!isObject) {
        node = *subNode;
        delete subNode;
        return true;
    }
    SetValueCommon(node, *subNode, name);
    return true;
}

bool Serializable::SetValue(
    json &node, const std::vector<uint8_t> &value, const std::string &name, const bool &isObject)
{
    json *subNode = cJSON_CreateArray();
    for (const uint8_t &item : value) {
        cJSON_AddItemToArray(subNode, cJSON_CreateNumber(item));
    }
    if (!isObject) {
        node = *subNode;
        delete subNode;
        return true;
    }
    SetValueCommon(node, *subNode, name);
    return true;
}

bool Serializable::SetValue(json &node, const Serializable &value, const std::string &name, const bool &isObject)
{
    json *subNode = cJSON_CreateNull();
    if (!value.Marshal(*subNode)) {
        return false;
    }
    if (!isObject) {
        node = *subNode;
        delete subNode;
        return true;
    }
    SetValueCommon(node, *subNode, name);
    return true;
}

const Serializable::json &Serializable::GetSubNode(const json &node, const std::string &name)
{
    static const json *jsonNull = cJSON_CreateNull();
    if (cJSON_IsNull(&node)) {
        return *jsonNull;
    }

    if (name.empty()) {
        return node;
    }
    if (!cJSON_HasObjectItem(&node, name.c_str())) {
        return *jsonNull;
    }
    return *cJSON_GetObjectItem(&node, name.c_str());
}
} // namespace DistributedData
} // namespace OHOS