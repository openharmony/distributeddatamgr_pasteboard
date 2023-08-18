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

#ifndef OHOS_DISTRIBUTED_SERIALIZABLE_SERIALIZABLE_H
#define OHOS_DISTRIBUTED_SERIALIZABLE_SERIALIZABLE_H
#include <string>
#include <vector>

#include "api/visibility.h"
#ifndef JSON_NOEXCEPTION
#define JSON_NOEXCEPTION
#endif
#include "cJSON.h"
namespace OHOS {
namespace DistributedData {
#ifndef GET_NAME
#define GET_NAME(value) #value
#endif
struct Serializable {
public:
    using json = cJSON;
    API_EXPORT json Marshall() const;
    template<typename T>
    static std::string Marshall(T &values)
    {
        json *root;
        SetValue(*root, values);
        return cJSON_PrintUnformatted(root);
    }

    API_EXPORT bool Unmarshall(const std::string &jsonStr);
    template<typename T>
    static bool Unmarshall(const std::string &body, T &values)
    {
        return GetValue(ToJson(body), "", values);
    }
    API_EXPORT static json ToJson(const std::string &jsonStr);
    virtual bool Marshal(json &node) const = 0;
    virtual bool Unmarshal(const json &node) = 0;
    API_EXPORT static bool GetValue(const json &node, const std::string &name, std::string &value);
    API_EXPORT static bool GetValue(const json &node, const std::string &name, uint32_t &value);
    API_EXPORT static bool GetValue(const json &node, const std::string &name, int32_t &value);
    API_EXPORT static bool GetValue(const json &node, const std::string &name, int64_t &value);
    API_EXPORT static bool GetValue(const json &node, const std::string &name, bool &value);
    API_EXPORT static bool GetValue(const json &node, const std::string &name, std::vector<uint8_t> &value);
    API_EXPORT static bool GetValue(const json &node, const std::string &name, Serializable &value);
    API_EXPORT static bool SetValue(
        json &node, const std::string &value, const std::string &name = "", const bool &isObject = true);
    API_EXPORT static bool SetValue(
        json &node, const uint32_t &value, const std::string &name = "", const bool &isObject = true);
    API_EXPORT static bool SetValue(
        json &node, const int32_t &value, const std::string &name = "", const bool &isObject = true);
    API_EXPORT static bool SetValue(
        json &node, const int64_t &value, const std::string &name = "", const bool &isObject = true);
    API_EXPORT static bool SetValue(
        json &node, const bool &value, const std::string &name = "", const bool &isObject = true);
    API_EXPORT static bool SetValue(
        json &node, const std::vector<uint8_t> &value, const std::string &name = "", const bool &isObject = true);
    API_EXPORT static bool SetValue(
        json &node, const Serializable &value, const std::string &name = "", const bool &isObject = true);

protected:
    API_EXPORT ~Serializable() = default;

    template<typename T>
    static bool GetValue(const json &node, const std::string &name, T *&value)
    {
        auto &subNode = GetSubNode(node, name);
        if (cJSON_IsNull(&subNode)) {
            return false;
        }
        value = new (std::nothrow) T();
        if (value == nullptr) {
            return false;
        }
        bool result = GetValue(subNode, "", *value);
        if (!result) {
            delete value;
            value = nullptr;
        }
        return result;
    }
    template<typename T>
    static bool GetValue(const json &node, const std::string &name, std::vector<T> &values)
    {
        auto &subNode = GetSubNode(node, name);
        if (cJSON_IsNull(&subNode) || !cJSON_IsArray(&subNode)) {
            return false;
        }
        bool result = true;
        values.resize(cJSON_GetArraySize(&subNode));
        for (int i = 0; i < cJSON_GetArraySize(&subNode); ++i) {
            result = GetValue(*cJSON_GetArrayItem(&subNode, i), "", values[i]) && result;
        }
        return result;
    }

    template<typename T>
    static bool SetValue(
        json &node, const std::vector<T> &values, const std::string &name = "", const bool &isObject = true)
    {
        bool result = true;
        int i = 0;
        auto subNode = cJSON_CreateArray();
        for (const auto &value : values) {
            json *item = cJSON_CreateNull();
            result = SetValue(*item, value, "", false) && result;
            cJSON_AddItemToArray(subNode, item);
            i++;
        }

        if (!isObject) {
            node = *subNode;
            delete subNode;
            return result;
        }
        SetValueCommon(node, *subNode, name);
        return result;
    }

    API_EXPORT static const json &GetSubNode(const json &node, const std::string &name);
    static void SetValueCommon(json &node, json &subNode, const std::string &name)
    {
        if (!cJSON_IsObject(&node)) {
            node = *cJSON_CreateObject();
        }
        cJSON_AddItemToObject(&node, name.c_str(), &subNode);
    }
};
} // namespace DistributedData
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_SERIALIZABLE_SERIALIZABLE_H
