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

#include "clip/clip_plugin.h"

#include "default_clip.h"
#include "pasteboard_event_dfx.h"
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
    RADAR_REPORT(RadarReporter::DFX_PLUGIN_CREATE_DESTROY, RadarReporter::DFX_PLUGIN_CREATE,
        RadarReporter::DFX_SUCCESS);
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
    RADAR_REPORT(RadarReporter::DFX_PLUGIN_CREATE_DESTROY, RadarReporter::DFX_PLUGIN_DESTROY,
        RadarReporter::DFX_SUCCESS);
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

int32_t ClipPlugin::PublishServiceState(const std::string &networkId, ServiceStatus status)
{
    return 0;
}

void ClipPlugin::Clear(int32_t user)
{
    (void)user;
}

void ClipPlugin::RegisterDelayCallback(const DelayCallback &callback)
{
    (void)callback;
}

bool ClipPlugin::GlobalEvent::Marshal(Serializable::json &node) const
{
    bool ret = true;
    ret = ret && SetValue(node, version, GET_NAME(version));
    ret = ret && SetValue(node, frameNum, GET_NAME(frameNum));
    ret = ret && SetValue(node, user, GET_NAME(user));
    ret = ret && SetValue(node, seqId, GET_NAME(seqId));
    ret = ret && SetValue(node, expiration, GET_NAME(expiration));
    ret = ret && SetValue(node, status, GET_NAME(status));
    ret = ret && SetValue(node, deviceId, GET_NAME(deviceId));
    ret = ret && SetValue(node, account, GET_NAME(account));
    ret = ret && SetValue(node, dataType, GET_NAME(dataType));
    ret = ret && SetValue(node, syncTime, GET_NAME(syncTime));
    return ret;
}

bool ClipPlugin::GlobalEvent::Unmarshal(const Serializable::json &node)
{
    bool ret = true;
    ret = ret && GetValue(node, GET_NAME(version), version);
    ret = ret && GetValue(node, GET_NAME(frameNum), frameNum);
    ret = ret && GetValue(node, GET_NAME(user), user);
    ret = ret && GetValue(node, GET_NAME(seqId), seqId);
    ret = ret && GetValue(node, GET_NAME(expiration), expiration);
    ret = ret && GetValue(node, GET_NAME(status), status);
    ret = ret && GetValue(node, GET_NAME(deviceId), deviceId);
    ret = ret && GetValue(node, GET_NAME(account), account);
    ret = ret && GetValue(node, GET_NAME(dataType), dataType);
    ret = ret && GetValue(node, GET_NAME(syncTime), syncTime);
    return ret;
}
} // namespace OHOS::MiscServices
