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

#include "clip/clip_plugin.h"

#include "default_clip.h"
namespace OHOS::MiscServices {
std::map<std::string, ClipPlugin::Factory *> ClipPlugin::factories_;
DefaultClip g_defaultClip;
bool ClipPlugin::RegCreator(const std::string &name, Factory *factory)
{
    auto it = factories_.find(name);
    if (it != factories_.end()) {
        return false;
    }
    factories_[name] = factory;
    return true;
}

ClipPlugin *ClipPlugin::CreatePlugin(const std::string &name)
{
    auto it = factories_.find(name);
    if (it == factories_.end() || it->second == nullptr) {
        return &g_defaultClip;
    }
    return it->second->Create();
}

bool ClipPlugin::DestroyPlugin(const std::string &name, ClipPlugin *plugin)
{
    if (plugin == &g_defaultClip) {
        return true;
    }

    auto it = factories_.find(name);
    if (it == factories_.end() || it->second == nullptr) {
        return false;
    }
    return it->second->Destroy(plugin);
}

ClipPlugin::~ClipPlugin()
{
}

std::vector<ClipPlugin::GlobalEvent> ClipPlugin::GetTopEvents(uint32_t topN)
{
    (void)topN;
    return std::vector<GlobalEvent>();
}

std::vector<ClipPlugin::GlobalEvent> ClipPlugin::GetTopEvents(uint32_t topN, int32_t user)
{
    (void)user;
    (void)topN;
    return std::vector<GlobalEvent>();
}

void ClipPlugin::Clear()
{
}

void ClipPlugin::Clear(int32_t user)
{
    (void)user;
}
} // namespace OHOS::MiscServices
