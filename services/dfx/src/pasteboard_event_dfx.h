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
};

enum BizStageSetPasteboard : std::int32_t {
    DFX_SET_BIZ_SCENE = 0,
    DFX_CHECK_SET_SERVER = 1,
    DFX_CHECK_SET_DELAY_COPY = 2,
    DFX_CHECK_SET_DATA_HTML_TYPE = 3,
    DFX_CHECK_SET_AUTHORITY = 4,
    DFX_CONSTRUCT_PASTEDATA = 5,
    DFX_CHECK_ONLINE_DEVICE = 6,
    DFX_CHECK_DEVICE_DISTRIBUTED = 7,
    DFX_LOAD_DISTRIBUTED_PLUGIN = 8,
    DFX_PERSIST_DISTRIBUTED_DATA = 9,
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
    DFX_INIT_BROADCAST_PULL = 1,
};

enum BizStageGetPasteboard : std::int32_t {
    DFX_GET_BIZ_SCENE = 0,
    DFX_CHECK_GET_SERVER = 1,
    DFX_CHECK_GET_DELAY_PASTE = 2,
    DFX_CHECK_GET_AUTHORITY = 3,
    DFX_CHECK_GET_URI_AUTHORITY = 4,
    DFX_GET_CACHE_DATA = 5,
    DFX_HANDLE_GET_DATA = 6,
};


enum StageRes : std::int32_t {
    DFX_IDLE = 0,
    DFX_SUCCESS = 1,
    DFX_FAILED = 2,
    DFX_CANCELLED = 3,
};

enum BizState : std::int32_t {
    DFX_BEGIN = 0,
    DFX_NORMAL_END = 1,
    DFX_ABNORMAL_END = 2,
};

enum ErrorCode : std::int32_t {
    INVALID_RETURN_VALUE_ERROR = 27787265,
    INVALID_PARAM_ERROR = 27787266,
    SERIALIZATION_ERROR = 27787267,
    DESERIALIZATION_ERROR = 27787268,
    OBTAIN_SERVER_SA_ERROR = 27787269,
    OTHER_ERROR = 27787270,
    CROSS_BORDER_ERROR = 27787271,
    PERMISSION_VERIFICATION_ERROR = 27787272,
    PARAM_ERROR = 27787273,
    TIMEOUT_ERROR = 27787274,
    CANCELED = 27787275,
    EXCEEDING_LIMIT_EXCEPTION = 27787276,
    TASK_PROCESSING = 27787277,
    PROHIBIT_COPY = 27787278,
    UNKNOWN_ERROR = 27787279,
    BACKUP_EXCEPTION = 27787280,
    REMOTE_EXCEPTION = 27787281,
};

static constexpr char DOMAIN[] = "DISTDATAMGR";
const std::string EVENT_NAME = "DISTRIBUTED_PASTEBOARD_BEHAVIOUR";
const std::string ORG_PKG = "distributeddata";
const std::string BIZ_STATE = "BIZ_STATE";
const std::string SET_DATA_APP = "SET_DATA_APP";
const std::string SET_DATA_TYPE = "SET_DATA_TYPE";
const std::string GET_DATA_APP = "GET_DATA_APP";
const std::string GET_DATA_TYPE = "GET_DATA_TYPE";
const std::string LOCAL_DEV_TYPE = "LOCAL_DEV_TYPE";
const std::string COVER_DELAY_DATA = "COVER_DELAY_DATA";
static constexpr HiviewDFX::HiSysEvent::EventType TYPE = HiviewDFX::HiSysEvent::EventType::BEHAVIOR;

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
#endif //DISTRIBUTEDDATAMGR_PASTEBOARD_EVENT_DFX_H
