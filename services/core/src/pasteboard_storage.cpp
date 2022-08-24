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
#include "pasteboard_storage.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace nlohmann {
template <typename T>
    struct adl_serializer<std::shared_ptr<T>> {
        static void to_json(json& j, const std::shared_ptr<T>& ptr)
        {
            if (ptr.get()) {
                j = *ptr;
            } else {
                j = nullptr;
            }
        }
        static void from_json(const json& j, std::shared_ptr<T>& ptr)
        {
            if (j.is_null()) {
                ptr = nullptr;
            } else {
                ptr = std::make_shared<T>(j.get<T>());
            }
        }
    };
}
namespace OHOS {
namespace MiscServices {
void to_json(nlohmann::json& j, const PasteDataRecord& r)
{
    j["mime_type"]= r.GetMimeType();
    j["html_text"]= r.GetHtmlText();
    j["plain_text"]= r.GetPlainText();
    j["uri"]= r.GetUri()->ToString();
    auto want = r.GetWant();
    if (want) {
        j["want"] = want->ToUri();
    } else {
        j["want"] = nullptr;
    }
}

void from_json(const nlohmann::json& j, PasteDataRecord& r)
{
    std::string mimeType = j.at("mime_type");
    std::shared_ptr<std::string> htmlText = j.at("html_text");
    std::shared_ptr<std::string> plainText = j.at("plain_text");
    std::shared_ptr<OHOS::Uri> uri = std::make_shared<OHOS::Uri>(j.at("uri"));

    auto wantJson = j.at("want");
    OHOS::AAFwk::Want want;
}

void to_json(nlohmann::json& j, const PasteData& p)
{
    j = p.AllRecords();
}

void from_json(const nlohmann::json& j, PasteData& p)
{
    auto records = j.get<std::vector<std::shared_ptr<PasteDataRecord>>>();
    p = PasteData(records);
}

std::shared_ptr<PasteboardStorage> PasteboardStorage::Create(const std::string &file)
{
    return std::make_shared<PasteboardStorage>(file);
}

PasteboardStorage::PasteboardStorage(std::string file) : file { std::move(file) }
{
}

void PasteboardStorage::SaveData(std::map<int32_t, std::shared_ptr<PasteData>> data)
{
    try {
        nlohmann::json jsonData = data;
        auto str = jsonData.dump();
        std::ofstream outstream(this->file);
        if (outstream.is_open()) {
            outstream<< str;
        }
    }catch(const std::exception& e) {
        std::cout<<"save passteboard failed "<<e.what()<<std::endl;
    }
}

std::map<int32_t, std::shared_ptr<PasteData>> PasteboardStorage::LoadData()
{
    std::map<int32_t, std::shared_ptr<PasteData>> data;
    try {
        std::ifstream instream(this->file);
        if (instream.is_open()) {
            return nlohmann::json::parse(instream);
        }
    }catch(const std::exception& e) {
        std::cout<<"load passteboard failed "<<e.what()<<std::endl;
    }
    return data;
}

PasteboardStorage::~PasteboardStorage() {
}
} // MiscServices
} // OHOS
