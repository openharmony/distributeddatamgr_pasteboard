/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "datashare_delegate_mock.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {

// Simple stub implementation for testing
DataShareDelegate &DataShareDelegate::GetInstance()
{
    static DataShareDelegate instance;
    return instance;
}

int32_t DataShareDelegate::GetValue(const std::string &key, std::string &value)
{
    // Check if mock is set for testing
    if (DataShareDelegateMock::GetMock() != nullptr) {
        return DataShareDelegateMock::GetMock()->GetValue(key, value);
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void DataShareDelegate::SetUserId(int32_t userId)
{
    (void)userId;
}

int32_t DataShareDelegate::RegisterObserver(const std::string &key, sptr<AAFwk::IDataAbilityObserver> observer)
{
    (void)key;
    (void)observer;
    return ERR_OK;
}

int32_t DataShareDelegate::UnregisterObserver(const std::string &key, sptr<AAFwk::IDataAbilityObserver> observer)
{
    (void)key;
    (void)observer;
    return ERR_OK;
}

} // namespace MiscServices
} // namespace OHOS
