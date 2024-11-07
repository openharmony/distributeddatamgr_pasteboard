/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_EVENT_DFX_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_EVENT_DFX_H

#include "hisysevent.h"
namespace OHOS {
namespace MiscServices {
namespace RadarReporter {
using namespace OHOS::HiviewDFX;
enum BizScene : std::int32_t {
    DFX_SET_PASTEBOARD = 1,
    DFX_DISTRIBUTED_PASTEBOARD_BROADCAST_SEND = 2,
    DFX_DISTRIBUTED_PASTEBOARD_BROADCAST_RECEIVE = 3,
    DFX_DISTRIBUTED_PASTEBOARD_BROADCAST_PULL = 4,
    DFX_GET_PASTEBOARD = 5,
    DFX_CLEAR_PASTEBOARD = 6,
    DFX_OBSERVER = 7,
    DFX_PLUGIN_CREATE_DESTROY = 8,
};

enum BizStageSetPasteboard : std::int32_t {
    DFX_SET_BIZ_SCENE = 0,
    DFX_CHECK_SET_SERVER = 1,
    DFX_CHECK_SET_DELAY_COPY = 2,
    DFX_CHECK_SET_AUTHORITY = 3,
    DFX_CHECK_ONLINE_DEVICE = 4,
    DFX_LOAD_DISTRIBUTED_PLUGIN = 5,
};

enum BizStageBroadcastSend : std::int32_t {
    DFX_REGISTER_SEND_LISTEN = 1,
    DFX_RECORD_SEND_BROADCAST = 2,
};

enum BizStageBroadcastReceive : std::int32_t {
    DFX_REGISTER_RECEIVE_LISTEN = 1,
    DFX_RECORD_RECEIVE_BROADCAST = 2,
};

enum BizStageBroadcastPull : std::int32_t {
    DFX_PRE_SYNC = 1,
    DFX_GET_PASTE_DATA_SYNC = 2,
    DFX_SYNC_DETAIL = 3,
};

enum BizStageGetPasteboard : std::int32_t {
    DFX_GET_BIZ_SCENE = 0,
    DFX_CHECK_GET_SERVER = 1,
    DFX_CHECK_GET_DELAY_PASTE = 2,
    DFX_CHECK_GET_AUTHORITY = 3,
    DFX_GET_DATA_INFO = 4,
    DFX_LOCAL_PASTE_END = 5,
    DFX_DISTRIBUTED_PASTE_END = 6,
    DFX_DISTRIBUTED_FILE_START = 7,
    DFX_DISTRIBUTED_FILE_END = 8,
};

enum BizStageClearPasteboard : std::int32_t {
    DFX_MANUAL_CLEAR = 1,
    DFX_AUTO_CLEAR = 2,
};

enum BizStageObserver : std::int32_t {
    DFX_ADD_OBSERVER = 1,
    DFX_REMOVE_SINGLE_OBSERVER = 2,
    DFX_REMOVE_ALL_OBSERVER = 3,
};

enum BizStagePlugin : std::int32_t {
    DFX_PLUGIN_CREATE = 1,
    DFX_PLUGIN_DESTROY = 2,
};

enum StageRes : std::int32_t {
    DFX_IDLE = 0,
    DFX_SUCCESS = 1,
    DFX_FAILED = 2,
    DFX_CANCELLED = 3,
};

enum BizState : std::int32_t {
    DFX_BEGIN = 1,
    DFX_END = 2,
};

class PasteboardDfxUntil {
public:
    static std::string GetAnonymousID(const std::string &deviceId);
private:
    static constexpr int minIdLen = 10;
    static constexpr int maskIdLen = 5;
};

constexpr char DOMAIN[] = "DISTDATAMGR";
constexpr const char *EVENT_NAME = "DISTRIBUTED_PASTEBOARD_BEHAVIOR";
constexpr const char *ORG_PKG = "distributeddata";
constexpr const char *BIZ_STATE = "BIZ_STATE";
constexpr const char *ERROR_CODE = "ERROR_CODE";
constexpr const char *SET_DATA_APP = "SET_DATA_APP";
constexpr const char *SET_DATA_TYPE = "SET_DATA_TYPE";
constexpr const char *GET_DATA_APP = "GET_DATA_APP";
constexpr const char *GET_DATA_TYPE = "GET_DATA_TYPE";
constexpr const char *LOCAL_DEV_TYPE = "LOCAL_DEV_TYPE";
constexpr const char *COVER_DELAY_DATA = "COVER_DELAY_DATA";
constexpr const char *SEND_BROADCAST_TIME = "SEND_BROADCAST_TIME_64";
constexpr const char *RECEIVE_BROADCAST_TIME = "RECEIVE_BROADCAST_TIME_64";
constexpr const char *SEQ_ID = "SEQ_ID";
constexpr const char *CONCURRENT_ID = "CONCURRENT_ID";
constexpr const char *DIS_SYNC_TIME = "DIS_SYNC_TIME";
constexpr const char *PACKAGE_NAME = "PACKAGE_NAME";
constexpr const char *PEER_NET_ID = "PEER_NET_ID";
constexpr const char *PEER_UDID = "PEER_UDID";
constexpr HiviewDFX::HiSysEvent::EventType TYPE = HiviewDFX::HiSysEvent::EventType::BEHAVIOR;

#define RADAR_REPORT(bizScene, bizStage, stageRes, ...)                                    \
({                                                                                         \
    HiSysEventWrite(RadarReporter::DOMAIN, RadarReporter::EVENT_NAME, RadarReporter::TYPE, \
        "ORG_PKG", RadarReporter::ORG_PKG, "FUNC", __FUNCTION__,                           \
        "BIZ_SCENE", bizScene, "BIZ_STAGE", bizStage, "STAGE_RES", stageRes,               \
        ##__VA_ARGS__);                                                                    \
})
} // namespace RadarReporter
} // namespace MiscServices
} // namespace OHOS
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_EVENT_DFX_H
