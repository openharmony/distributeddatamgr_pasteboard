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

#ifndef OHOS_DATASHARE_DELEGATE_MOCK_H
#define OHOS_DATASHARE_DELEGATE_MOCK_H

#include <gmock/gmock.h>
#include <cstdint>
#include <string>

namespace OHOS {
namespace MiscServices {

// Forward declaration of DataShareDelegate
class DataShareDelegate;

class IDataShareDelegateMock {
public:
    virtual ~IDataShareDelegateMock() = default;
    virtual int32_t GetValue(const std::string &key, std::string &value) = 0;
};

class DataShareDelegateMock : public IDataShareDelegateMock {
public:
    static DataShareDelegateMock *GetMock()
    {
        return mock_;
    }
    static void SetMock(DataShareDelegateMock *mock)
    {
        mock_ = mock;
    }

    MOCK_METHOD(int32_t, GetValue, (const std::string &key, std::string &value), (override));

private:
    static inline DataShareDelegateMock *mock_ = nullptr;
};

} // namespace MiscServices
} // namespace OHOS

#endif // OHOS_DATASHARE_DELEGATE_MOCK_H
