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

#ifndef DISTRIBUTED_PASTEBOARD_FRAMEWORK_DISTRIBUTED_CLIP_H
#define DISTRIBUTED_PASTEBOARD_FRAMEWORK_DISTRIBUTED_CLIP_H
#include <mutex>
#include <shared_mutex>
#include "device/dm_adapter.h"

namespace OHOS::MiscServices {
class DistributedClip : public DMAdapter::DMObserver {
public:
    void Online(const std::string &device) override;
    void Offline(const std::string &device) override;
    void OnReady(const std::string &device) override;
};

void DistributedClip::Online(const std::string &device)
{
}

void DistributedClip::Offline(const std::string &device)
{
}

void DistributedClip::OnReady(const std::string &device)
{
}
} // namespace OHOS::MiscServices
#endif // DISTRIBUTED_PASTEBOARD_FRAMEWORK_DISTRIBUTED_CLIP_H
