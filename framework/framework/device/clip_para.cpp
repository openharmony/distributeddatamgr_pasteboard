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
#include "clip_para.cpp"

#include <chrono>
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
using namespace std::chrono;

ClipPara::ClipPara()
{
}

ClipPara &ClipPara::GetInstance()
{
    static ClipPara instance;
    return instance;
}

void ClipPara::InitMemberVariable()
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "InitMemberVariable");
    sendInformation_ = nullptr;
    remoteExpiration_ = 0;
    lastSyncNetworkId_ = "";
    isPullEvent_ = false;
    isPullEventResult_ = false;
    isPasted_ = false;
}

bool ClipPara::HasRemoteData()
{
    uint64_t curTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return sendInformation_ != nullptr && remoteExpiration_ > curTime;
}

std::string ClipPara::GetLastLocalSyncKey()
{
    std::lock_guard<decltype(mutex_)> lockGuard(mutex_);
    return lastLocalSyncKey_;
}

std::string ClipPara::GetLastRemoteSyncKey()
{
    return lastRemoteSyncKey_;
}

std::string ClipPara::GetLastSyncNetworkId()
{
    return lastSyncNetworkId_;
}

uint64_t ClipPara::GetLocalExpiration()
{
    return localExpiration_.load();
}

uint64_t ClipPara::GetRemoteExpiration()
{
    return remoteExpiration_;
}

std::shared_ptr<ClipPara::SendInformation> ClipPara::GetSendInformation()
{
    return sendInformation_;
}

uint32_t ClipPara::GetFirstStageValue()
{
    return firstStageValue_.load();
}

uint32_t ClipPara::GetSecondStageValue()
{
    return secondStageValue_.load();
}

bool ClipPara::GetPullEvent()
{
    return isPullEvent_.load();
}

bool ClipPara::GetPullEventResult()
{
    return isPullEventResult_.load();
}

bool ClipPara::GetPasted()
{
    return isPasted_.load();
}

void ClipPara::SetLastLocalSyncKey(const std::string lastSyncKey)
{
    std::lock_guard<decltype(mutex_)> lockGuard(mutex_);
    lastLocalSyncKey_ = lastSyncKey;
}

void ClipPara::SetLastRemoteSyncKey(const std::string lastRemoteSyncKey)
{
    lastRemoteSyncKey_ = lastRemoteSyncKey;
}

void ClipPara::SetLastSyncNetworkId(const std::string lastSyncNetworkId)
{
    lastSyncNetworkId_ = lastSyncNetworkId;
}

void ClipPara::SetLocalExpiration(const uint64_t expiration)
{
    localExpiration_.store(expiration);
}

void ClipPara::SetRemoteExpiration(const uint64_t remoteExpiration)
{
    remoteExpiration_.store(remoteExpiration);
}

void ClipPara::SetSendInformation(const ClipPara::SendInformation &senderInformation)
{
    sendInformation_ = std::make_shared<ClipPara::SendInformation>(senderInformation);
}

void ClipPara::SetFirstStageValue(const uint32_t firstStageValue)
{
    firstStageValue_.store(firstStageValue);
}

void ClipPara::SetSecondStageValue(const uint32_t secondStageValue)
{
    secondStageValue_.store(secondStageValue);
}

void ClipPara::SetPullEvent(const bool isPullEvent)
{
    isPullEvent_.store(isPullEvent);
}

void ClipPara::SetPullEventResult(const bool isPullEventResult)
{
    isPullEventResult_.store(isPullEventResult);
}

void ClipPara::SetPasted(const bool isPasted)
{
    isPasted_.store(isPasted);
}

void ClipPara::UpdateStageValue(const uint64_t &expiration, bool isPasting)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "start, firstStageValue = %{public}s, secondStageValue = %{public}s",
        std::to_string(firstStageValue_).c_str(), std::to_string(secondStageValue_).c_str());
    if (!isPullEvent_ && !isPasting) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "not do pre-establish");
        return;
    }

    if (isPasted_) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "stage value has been update");
        return;
    }
    uint64_t curTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    if (isPasting && expiration > curTime) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "UpdateStageValue");
        isPasted_ = true;
        if ((firstStageValue_ == MIN_STAGE_VALUE && secondStageValue_ == MAX_STAGE_VALUE) ||
            firstStageValue_ != MIN_STAGE_VALUE) {
            firstStageValue_ = MAX_STAGE_VALUE;
            secondStageValue_ = MIN_STAGE_VALUE;
        } else {
            secondStageValue_.fetch_add(1);
        }
    } else {
        if (firstStageValue_ == MIN_STAGE_VALUE) {
            auto diffValue = (secondStageValue_ != MIN_STAGE_VALUE) ? 1 : 0;
            secondStageValue_.fetch_sub(diffValue);
        } else {
            firstStageValue_.fetch_sub(1);
        }
    }
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "end, firstStageValue = %{public}s, secondStageValue = %{public}s",
        std::to_string(firstStageValue_).c_str(), std::to_string(secondStageValue_).c_str());
}
} // namespace MiscServices
} // namespace OHOS