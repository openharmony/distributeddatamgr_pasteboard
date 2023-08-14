/*
* Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef PASTE_BOARD_CLIP_PARA_H
#define PASTE_BOARD_CLIP_PARA_H

#include <atomic>
#include <mutex>
#include <string>

#include "api/visibility.h"

namespace OHOS {
namespace MiscServices {
class API_EXPORT ClipPara {
public:
    struct SendInformation {
        uint64_t timeStamp = 0;
        uint8_t numType = 0;
        uint8_t version = 0;
        uint16_t userId = 0;
        uint32_t seqId = 0;
        uint32_t currentIndex = 0;
    };

    constexpr static uint32_t PASTEBOARD_SEND_LEN = 26;
    constexpr static uint8_t TIMESTAMP_PARA_TYPE = 0x00;
    constexpr static uint8_t DATA_PARA_TYPE = 0x01;

    static ClipPara &GetInstance();
    void InitMemberVariable();
    bool HasRemoteData();
    std::string GetLastLocalSyncKey();
    std::string GetLastRemoteSyncKey();
    std::string GetLastSyncNetworkId();
    uint64_t GetLocalExpiration();
    uint64_t GetRemoteExpiration();
    std::shared_ptr<ClipPara::SendInformation> GetSendInformation();
    uint32_t GetFirstStageValue();
    uint32_t GetSecondStageValue();
    bool GetPullEvent();
    bool GetPullEventResult();
    bool GetPasted();

    void SetLastLocalSyncKey(const std::string lastSyncKey);
    void SetLastRemoteSyncKey(const std::string lastRemoteSyncKey);
    void SetLastSyncNetworkId(const std::string lastSyncNetworkId);
    void SetLocalExpiration(const uint64_t expiration);
    void SetRemoteExpiration(const uint64_t remoteExpiration);
    void SetSendInformation(const ClipPara::SendInformation &senderInformation);
    void SetFirstStageValue(const uint32_t firstStageValue);
    void SetSecondStageValue(const uint32_t secondStageValue);
    void SetPullEvent(const bool isPullEvent);
    void SetPullEventResult(const bool isPullEventResult);
    void SetPasted(const bool isPasted);
    void UpdateStageValue(const uint64_t &expiration, bool isPasting);

private:
    ClipPara();
    ~ClipPara() = default;
    static constexpr uint32_t MAX_STAGE_VALUE = 2;
    static constexpr uint32_t MIN_STAGE_VALUE = 0;
    std::mutex mutex_;
    std::string lastLocalSyncKey_;
    std::string lastRemoteSyncKey_;
    std::string lastSyncNetworkId_;
    std::atomic<bool> isPullEvent_ = false;
    std::atomic<bool> isPullEventResult_ = false;
    std::atomic<bool> isPasted_ = false;
    std::atomic<uint64_t> localExpiration_ = 0;
    std::atomic<uint64_t> remoteExpiration_ = 0;
    std::atomic<uint32_t> firstStageValue_ = 2;
    std::atomic<uint32_t> secondStageValue_ = 0;
    std::shared_ptr<ClipPara::SendInformation> sendInformation_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_CLIP_PARA_H