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

#include "datashare_delegate.h"
#include "pasteboard_hilog.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "iservice_registry.h"
#include "third_party/vixl/src/utils-vixl.h"
#include <memory>
#include <string>

namespace OHOS::MiscServices {
const constexpr char* SETTING_COLUMN_KEYWORD = "KEYWORD";
const constexpr char* SETTING_COLUMN_VALUE = "VALUE";
const constexpr char* SETTING_URI_PROXY =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
constexpr const char* SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
constexpr const int32_t PASTEBOARD_SA_ID = 3701;

std::mutex DataShareDelegate::mutex_;
sptr<IRemoteObject> DataShareDelegate::remoteObj_ = nullptr;
DataShareDelegate* DataShareDelegate::instance_ = nullptr;
DataShareDelegate& DataShareDelegate::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = new DataShareDelegate();
            Initialize();
        }
    }
    return *instance_;
}

void DataShareDelegate::Initialize()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get sa manager return nullptr");
        return;
    }
    auto remoteObj = samgr->GetSystemAbility(PASTEBOARD_SA_ID);
    if (remoteObj == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get system ability failed, id=%{public}d", PASTEBOARD_SA_ID);
        return;
    }
    remoteObj_ = remoteObj;
}

std::shared_ptr<DataShare::DataShareHelper> DataShareDelegate::CreateDataShareHelper()
{
    auto [ret, helper] = DataShare::DataShareHelper::Create(remoteObj_, SETTING_URI_PROXY, SETTINGS_DATA_EXT_URI);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "create helper failed ret %{public}d", ret);
        return nullptr;
    }
    return helper;
}

bool DataShareDelegate::ReleaseDataShareHelper(std::shared_ptr<DataShare::DataShareHelper> helper)
{
    if (!helper->Release()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "release helper fail");
        return false;
    }
    return true;
}

int32_t DataShareDelegate::GetValue(const std::string& key, std::string& value)
{
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        return -1;
    }
    std::vector<std::string> columns = {SETTING_COLUMN_VALUE};
    DataShare::DataSharePredicates predicates;
    predicates.EqualTo(SETTING_COLUMN_KEYWORD, key);
    Uri uri = MakeUri(key);
    auto resultSet = helper->Query(uri, predicates, columns);
    ReleaseDataShareHelper(helper);
    if (resultSet == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "Query failed key=%{public}s",  key.c_str());
        return ERR_INVALID_OPERATION;
    }
    int32_t count;
    resultSet->GetRowCount(count);
    if (count == 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "no value, key=%{public}s, count=%{public}d", key.c_str(), count);
        resultSet->Close();
        return ERR_NAME_NOT_FOUND;
    }
    int32_t INDEX = 0;
    resultSet->GoToRow(INDEX);
    int32_t ret = resultSet->GetString(INDEX, value);
    if (ret != DataShare::E_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "get value failed, ret=%{public}d", ret);
        resultSet->Close();
        return ERR_INVALID_VALUE;
    }
    resultSet->Close();
    return ERR_OK;
}

Uri DataShareDelegate::MakeUri(const std::string& key)
{
    Uri uri(std::string(SETTING_URI_PROXY) + "&key=" + key);
    return uri;
}

int32_t DataShareDelegate::RegisterObserver(const std::string& key,
    std::shared_ptr<DataShare::DataShareObserver> observer)
{
    auto uri = MakeUri(key);
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        return ERR_NO_INIT;
    }
    helper->RegisterObserverExt(uri, observer, true);
    ReleaseDataShareHelper(helper);
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "register observer %{public}s", uri.ToString().c_str());
    return ERR_OK;
}

int32_t DataShareDelegate::UnregisterObserver(const std::string& key,
    std::shared_ptr<DataShare::DataShareObserver> observer)
{
    auto uri = MakeUri(key);
    auto helper = CreateDataShareHelper();
    if (helper == nullptr) {
        return ERR_NO_INIT;
    }
    helper->UnregisterObserverExt(uri, observer);
    ReleaseDataShareHelper(helper);
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "unregister observer %{public}s", uri.ToString().c_str());
    return ERR_OK;
}
} // namespace OHOS::MiscServices