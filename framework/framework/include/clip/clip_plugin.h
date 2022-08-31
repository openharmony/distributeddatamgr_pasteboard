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

#ifndef OHOS_DISTRIBUTED_DATA_PASTEBOARD_SERVICES_FRAMEWORK_CLIPS_PLUGIN_H
#define OHOS_DISTRIBUTED_DATA_PASTEBOARD_SERVICES_FRAMEWORK_CLIPS_PLUGIN_H
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include "api/visibility.h"
namespace OHOS::MiscServices {
class API_EXPORT ClipPlugin {
public:
    enum EventStatus : uint32_t {
        EVT_UNKNOWN,
        EVT_INVALID,
        EVT_NORMAL,
        EVT_BUTT
    };

    struct GlobalEvent {
        uint8_t version = 0;
        uint8_t frameNum  = 0;
        uint16_t user  = 0;
        uint32_t seqId = 0;
        uint64_t expiration = 0;
        uint16_t status = EVT_UNKNOWN;
        std::string deviceId;
        std::string account;
        std::vector<uint8_t> addition;
    };
    class Factory {
    public:
        virtual ClipPlugin *Create() = 0;
        virtual bool Destroy(ClipPlugin *) = 0;
    };
    static bool RegCreator(const std::string &name, Factory *factory);
    static ClipPlugin *CreatePlugin(const std::string &name);
    static bool DestroyPlugin(const std::string &name, ClipPlugin *plugin);

    virtual ~ClipPlugin();
    virtual int32_t SetPasteData(const GlobalEvent &event, const std::vector<uint8_t> &data) = 0;
    virtual int32_t GetPasteData(const GlobalEvent &event, std::vector<uint8_t> &data) = 0;
    virtual std::vector<GlobalEvent> GetTopEvents(uint32_t topN);
    virtual std::vector<GlobalEvent> GetTopEvents(uint32_t topN, int32_t user);
    virtual void Clear();
    virtual void Clear(int32_t user);

private:
    static std::map<std::string, Factory *> factories_;
};
} // namespace OHOS::MiscServices
#endif // OHOS_DISTRIBUTED_DATA_PASTEBOARD_SERVICES_FRAMEWORK_CLIPS_PLUGIN_H
