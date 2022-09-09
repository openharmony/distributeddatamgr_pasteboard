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
#include "uri_info.h"

#include <fcntl.h>
#include <unistd.h>

#include "errors.h"
#include "os_account_manager.h"
#include "parcel_util.h"
#include "pasteboard_hilog_wreapper.h"
#include "remote_file_share.h"
#include "remote_uri.h"
namespace OHOS::MiscServices {
std::string UriInfo::ToShareUri() const
{
    if (fd_ < 0) {
        return "";
    }
    std::string sharePath;
    std::vector<int32_t> ids;
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    if (ret != ERR_OK || ids.empty()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "query active user failed errCode=%{public}d", ret);
        return sharePath;
    }
    ret = AppFileService::ModuleRemoteFileShare::RemoteFileShare::CreateSharePath(fd_, sharePath, ids[0]);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, " create share path failed, %{public}d ", ret);
        return sharePath;
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "share path: %{public}s", sharePath.c_str());
    return sharePath;
}
std::string UriInfo::ToClientUri() const
{
    if (fd_ < 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "fd not available");
        return "";
    }
    std::string result;
    int ret = DistributedFS::ModuleRemoteUri::RemoteUri::ConvertUri(fd_, result);
    if (ret != 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "convert uri from fd failed: %{public}d", ret);
        return result;
    }
    return result;
}
bool UriInfo::Marshalling(MessageParcel &parcel)
{
    if (!OpenPath()) {
        return false;
    }
    return parcel.WriteFileDescriptor(fd_);
}
bool UriInfo::OpenPath()
{
    if (fd_ >= 0) {
        return true;
    }
    fd_ = open(uri_.c_str(), O_RDONLY);
    if (fd_ < 0) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "open uri failed, maybe its not a legal file path %{public}s",
            uri_.c_str());
        return false;
    }
    return true;
}
bool UriInfo::Unmarshalling(MessageParcel &parcel)
{
    fd_ = parcel.ReadFileDescriptor();
    return fd_ >= 0;
}
size_t UriInfo::Count()
{
    if (IsCrossDevice()) {
        return TLVObject::Count(ToShareUri());
    }
    return 0;
}
bool UriInfo::Encode(std::vector<std::uint8_t> &buffer)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "is cross:%{public}d", IsCrossDevice());
    if (IsCrossDevice()) {
        return Write(buffer, TAG_URI, ToShareUri());
    }
    return true;
}
bool UriInfo::Decode(const std::vector<std::uint8_t> &buffer)
{
    for (; IsEnough();) {
        TLVHead head{};
        bool ret = ReadHead(buffer, head);
        switch (head.tag) {
            case TAG_URI: {
                if (IsCrossDevice()) {
                    ret = ret && ReadValue(buffer, uri_, head);
                }
                break;
            }
            default:
                ret = ret && Skip(head.len, buffer.size());
                break;
        }
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "read value,tag:%{public}u, len:%{public}u, ret:%{public}d",
            head.tag, head.len, ret);
        if (!ret) {
            return false;
        }
    }
    return true;
}
UriInfo::~UriInfo()
{
    if (fd_ >= 0) {
        close(fd_);
    }
}
} // namespace OHOS::MiscServices