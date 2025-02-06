/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <memory>
#include <string>

#include "pasteboard_progress.h"
#include "iservice_registry.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"
#include "pasteboard_time.h"
#include "paste_data_entry.h"
#include "udmf_client.h"

using namespace OHOS::UDMF;
namespace OHOS::MiscServices {
constexpr const int32_t PASTEBOARD_SA_ID = 3701;

std::mutex PasteBoardProgress::mutex_;
sptr<IRemoteObject> PasteBoardProgress::remoteObj_ = nullptr;
PasteBoardProgress *PasteBoardProgress::instance_ = nullptr;
PasteBoardProgress &PasteBoardProgress::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = new PasteBoardProgress();
            Initialize();
        }
    }
    return *instance_;
}

void PasteBoardProgress::Initialize()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "get sa manager return nullptr");
        return;
    }
    auto remoteObj = samgr->GetSystemAbility(PASTEBOARD_SA_ID);
    if (remoteObj == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "get system ability failed, id=%{public}d", PASTEBOARD_SA_ID);
        return;
    }
    remoteObj_ = remoteObj;
}

int32_t PasteBoardProgress::InsertValue(std::string &key, std::string &value)
{
    CustomOption option = {.intention = Intention::UD_INTENTION_DATA_HUB};
    UnifiedData data;
    auto udsObject = std::make_shared<Object>();
    if (udsObject == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "udsObject is nullptr");
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::PLAIN_TEXT);
    udsObject->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    udsObject->value_[UDMF::CONTENT] = value;
    udsObject->value_[UDMF::ABSTRACT] = std::to_string(PasteBoardTime::GetCurrentTimeMicros());
    std::shared_ptr<UnifiedRecord> record = std::make_shared<UnifiedRecord>();
    record->AddEntry(utdId, std::move(udsObject));
    data.AddRecord(record);
    UdmfClient::GetInstance().SetData(option, data, key);
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteBoardProgress::UpdateValue(std::string &key, std::string value)
{
    QueryOption queryOption = { .key = key };
    UnifiedData data;
    auto udsObject = std::make_shared<Object>();
    if (udsObject == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "udsObject is nullptr");
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::PLAIN_TEXT);
    udsObject->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    udsObject->value_[UDMF::CONTENT] = value;
    udsObject->value_[UDMF::ABSTRACT] = std::to_string(PasteBoardTime::GetCurrentTimeMicros());
    std::shared_ptr<UnifiedRecord> record = std::make_shared<UnifiedRecord>();
    record->AddEntry(utdId, std::move(udsObject));
    data.AddRecord(record);
    UdmfClient::GetInstance().UpdateData(queryOption, data);
    return static_cast<int32_t>(PasteboardError::E_OK);
}
} // namespace OHOS::MiscServices