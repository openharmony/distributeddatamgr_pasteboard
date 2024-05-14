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

#include <string>
#include "def.h"
#include "hisysevent_c.h"
#include "pasteboard_hilog.h"
namespace OHOS {
namespace MiscServices {
enum BizScene : std::int32_t {
    DFX_SET_PASTEBOARD = 1,
    DFX_DISTRIBUTED_PASTEBOARD_BROADCAST_SEND = 2,
    DFX_DISTRIBUTED_PASTEBOARD_BROADCAST_RECEIVE = 3,
    DFX_DISTRIBUTED_PASTEBOARD_BROADCAST_PULL = 4,
    DFX_GET_PASTEBOARD = 5
};

enum BizStageSetPasteboard : std::int32_t {
    DFX_CHECK_GET_SERVER = 1,
    DFX_CHECK_GET_DELAY_COPY = 2,
    DFX_CHECK_GET_DATA_HTML_TYPE = 3,
    DFX_CHECK_GET_AUTHORITY = 4,
    DFX_CONSTRUCT_PASTEDATA = 5,
    DFX_CHECK_ONLINE_DEVICE = 6,
    DFX_CHECK_DEVICE_DISTRIBUTED = 7,
    DFX_LOAD_DISTRIBUTED_PLUGIN = 8,
    DFX_PERSIST_DISTRIBUTED_DATA = 9
};

enum BizStageBroadcastSend : std::int32_t {
    DFX_REGISTER_SEND_LISTEN = 1,
    DFX_RECORD_SEND_BROADCAST = 2
};

enum BizStageBroadcastReceive : std::int32_t {
    DFX_REGISTER_RECEIVE_LISTEN = 1,
    DFX_RECORD_RECEIVE_BROADCAST = 2
};

enum BizStageBroadcastPull : std::int32_t {
    DFX_INIT_BROADCAST_PULL = 1
};

enum BizStageGetPasteboard : std::int32_t {
    DFX_CHECK_SET_SERVER = 1,
    DFX_CHECK_SET_AUTHORITY = 2,
    DFX_CHECK_SET_URI_AUTHORITY = 3,
    DFX_GET_CACHE_DATA = 4,
    DFX_HANDLE_GET_DATA = 5
};


enum StageRes : std::int32_t {
    DFX_IDLE = 0,
    DFX_SUCCESS = 1,
    DFX_FAILED = 2,
    DFX_CANCELLED = 3
};

enum BizState : std::int32_t {
    DFX_BEGIN = 0,
    DFX_NORMAL_END = 1,
    DFX_ABNORMAL_END = 2
};

constexpr const char *PASTEBOARD_DOMAIN = "DISTDATAMGRORG_PKG";
constexpr const char *PASTEBOARD_BEHAVIOUR = "DISTRIBUTED_PASTEBOARD_BEHAVIOUR";
constexpr const char *ORG_PKG = "ORG_PKG";
constexpr const char *ORG_PKG_VALUE = "distributeddata";
constexpr const char *BIZ_SCENE = "BIZ_SCENE";
constexpr const char *BIZ_STATE = "BIZ_STATE";
constexpr const char *BIZ_STAGE = "BIZ_STAGE";
constexpr const char *STAGE_RES = "STAGE_RES";

constexpr const HiSysEventParam BIZ_SCENE_SET_PASTEBOARD_PARAM = {
    .name = {*BIZ_SCENE},
    .t = HISYSEVENT_INT32,
    .v = { .i32 = DFX_SET_PASTEBOARD },
    .arraySize = 0,
};
constexpr const HiSysEventParam BIZ_SCENE_GET_PASTEBOARD_PARAM = {
    .name = {*BIZ_SCENE},
    .t = HISYSEVENT_INT32,
    .v = { .i32 = DFX_GET_PASTEBOARD },
    .arraySize = 0,
};

constexpr const HiSysEventParam ORG_PKG_PARAM = {
    .name = {*ORG_PKG},
    .t = HISYSEVENT_STRING,
    .v = { .s = const_cast<char *>(ORG_PKG_VALUE) },
    .arraySize = 0,
};

constexpr const HiSysEventParam BIZ_STATE_BEGIN_PARAM = {
    .name = {*BIZ_STATE},
    .t = HISYSEVENT_INT32,
    .v = { .i32 = DFX_BEGIN },
    .arraySize = 0,
};
constexpr const HiSysEventParam BIZ_STATE_NORMAL_END_PARAM = {
    .name = {*BIZ_STATE},
    .t = HISYSEVENT_INT32,
    .v = { .i32 = DFX_NORMAL_END },
    .arraySize = 0,
};
constexpr const HiSysEventParam BIZ_STATE_ABNORMAL_END_PARAM = {
    .name = {*BIZ_STATE},
    .t = HISYSEVENT_INT32,
    .v = { .i32 = DFX_ABNORMAL_END },
    .arraySize = 0,
};

constexpr const HiSysEventParam STAGE_RES_SUCCESS_PARAM = {
    .name = {*STAGE_RES},
    .t = HISYSEVENT_INT32,
    .v = { .i32 = DFX_SUCCESS },
    .arraySize = 0,
};

constexpr const HiSysEventParam STAGE_RES_FAILED_PARAM = {
    .name = {*STAGE_RES},
    .t = HISYSEVENT_INT32,
    .v = { .i32 = DFX_FAILED },
    .arraySize = 0,
};


class PasteboardDfxUntil {
public:
    static std::string GetAnonymousID(std:: string deviceId);
private:
    static constexpr int HALF = 2;
    static constexpr int MIN_ID_LEN = 10;
    static constexpr int MASK_ID_LEN = 5;
};

#define PASTEBOARD_DFX_EVENT(params, len)                                                                             \
do {                                                                                                                  \
    int ret = OH_HiSysEvent_Write(PASTEBOARD_DOMAIN, PASTEBOARD_BEHAVIOUR, HISYSEVENT_BEHAVIOR, (params), (len));     \
    if (ret != OHOS::HiviewDFX::SUCCESS) {                                                                                  \
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "pasteboard dfx hisysevent write failed! ret %{public}d.", ret); \
    }                                                                                                                 \
} while (0)

} // namespace MiscServices
} // namespace OHOS
#endif //DISTRIBUTEDDATAMGR_PASTEBOARD_EVENT_DFX_H
