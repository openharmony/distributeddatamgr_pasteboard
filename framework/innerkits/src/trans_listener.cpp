/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "trans_listener.h"

#include <dirent.h>
#include <filesystem>
#include <random>

#include "ipc_skeleton.h"
#include "pasteboard_hilog.h"
#include "sandbox_helper.h"
#include "uri.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::AppFileService;
using namespace AppFileService::ModuleFileUri;
const std::string NETWORK_PARA = "?networkid=";
const std::string FILE_MANAGER_AUTHORITY = "docs";
const std::string MEDIA_AUTHORITY = "media";
const std::string DISTRIBUTED_PATH = "/data/storage/el2/distributedfiles/";
std::atomic<uint32_t> TransListener::getSequenceId_ = 0;
static ProgressListener progressListener_;
constexpr int ERRNO_NOERR = 0;
constexpr int PERCENTAGE = 100;

void TransListener::RmDir(const std::string &path)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "RmDir path : %{private}s", path.c_str());
    std::filesystem::path pathName(path);
    std::error_code errCode;
    if (std::filesystem::exists(pathName, errCode)) {
        std::filesystem::remove_all(pathName, errCode);
        if (errCode.value() != 0) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT,
                "Failed to remove directory, error code: %{public}d", errCode.value());
        }
    } else {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "pathName is not exists, error code: %{public}d", errCode.value());
    }
}

std::string TransListener::CreateDfsCopyPath()
{
    std::random_device rd;
    std::string random = std::to_string(rd());
    while (std::filesystem::exists(DISTRIBUTED_PATH + random)) {
        random = std::to_string(rd());
    }
    return random;
}

int32_t TransListener::HandleCopyFailure(CopyEvent &copyEvent, const Storage::DistributedFile::HmdfsInfo &info,
    const std::string &disSandboxPath, const std::string &currentId)
{
    if (info.authority != FILE_MANAGER_AUTHORITY && info.authority != MEDIA_AUTHORITY) {
        RmDir(disSandboxPath);
    }
    return 0;
}

int MiscServices::TransListener::WaitForCopyResult(TransListener* transListener)
{
    if (transListener == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "transListener is nullptr");
        return FAILED;
    }
    std::unique_lock<std::mutex> lock(transListener->cvMutex_);
    transListener->cv_.wait(lock, [&transListener]() {
            return transListener->copyEvent_.copyResult == SUCCESS ||
                transListener->copyEvent_.copyResult == FAILED;
    });
    return transListener->copyEvent_.copyResult;
}

int32_t MiscServices::TransListener::CopyFileFromSoftBus(const std::string &srcUri, const std::string &destUri,
    std::shared_ptr<CopyInfo> copyInfo, std::shared_ptr<CopyCallback> callback,
    std::shared_ptr<GetDataParams> dataParam)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "CopyFileFromSoftBus begin.");
    progressListener_ = dataParam->listener;
    std::string currentId = "CopyFile_" + std::to_string(getpid()) + "_" + std::to_string(getSequenceId_);
    ++getSequenceId_;

    sptr<TransListener> transListener = new (std::nothrow) TransListener();
    if (transListener == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "new trans listener failed");
        return ENOMEM;
    }
    transListener->callback_ = callback;

    Storage::DistributedFile::HmdfsInfo info{};
    Uri uri(destUri);
    info.authority = uri.GetAuthority();
    info.sandboxPath = SandboxHelper::Decode(uri.GetPath());
    std::string disSandboxPath;
    auto ret = PrepareCopySession(srcUri, destUri, transListener, info, disSandboxPath);
    if (ret != ERRNO_NOERR) {
        return EIO;
    }
    auto copyResult = WaitForCopyResult(transListener);
    if (copyResult == FAILED) {
        return HandleCopyFailure(transListener->copyEvent_, info, disSandboxPath, currentId);
    }
    if (info.authority == FILE_MANAGER_AUTHORITY || info.authority == MEDIA_AUTHORITY) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "Public or media path not copy");
        return ERRNO_NOERR;
    }

    ret = CopyToSandBox(srcUri, disSandboxPath, info.sandboxPath, currentId);
    RmDir(disSandboxPath);
    if (ret != ERRNO_NOERR) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "CopyToSandBox failed, ret = %{public}d.", ret);
        return EIO;
    }
    return ERRNO_NOERR;
}

int32_t MiscServices::TransListener::PrepareCopySession(const std::string &srcUri, const std::string &destUri,
    TransListener* transListener, Storage::DistributedFile::HmdfsInfo &info, std::string &disSandboxPath)
{
    std::string tmpDir;
    if (info.authority != MiscServices::FILE_MANAGER_AUTHORITY && info.authority != MiscServices::MEDIA_AUTHORITY) {
        tmpDir = MiscServices::TransListener::CreateDfsCopyPath();
        disSandboxPath = DISTRIBUTED_PATH + tmpDir;
        std::error_code errCode;
        if (!std::filesystem::create_directory(disSandboxPath, errCode)) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Create dir failed, error code=%{public}d", errCode.value());
            return errCode.value();
        }

        auto pos = info.sandboxPath.rfind('/');
        if (pos == std::string::npos) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "invalid file path");
            return EIO;
        }
        auto sandboxDir = info.sandboxPath.substr(0, pos);
        if (std::filesystem::exists(sandboxDir, errCode)) {
            info.dirExistFlag = true;
        }
    }

    info.copyPath = tmpDir;
    auto networkId = GetNetworkIdFromUri(srcUri);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "dfs PrepareSession begin.");
    auto ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().PrepareSession(srcUri, destUri,
        networkId, transListener, info);
    if (ret != ERRNO_NOERR) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "PrepareSession failed, ret=%{public}d.", ret);
        if (info.authority != FILE_MANAGER_AUTHORITY && info.authority != MEDIA_AUTHORITY) {
            RmDir(disSandboxPath);
        }
        return EIO;
    }
    return ERRNO_NOERR;
}

int32_t MiscServices::TransListener::CopyToSandBox(const std::string &srcUri, const std::string &disSandboxPath,
    const std::string &sandboxPath, const std::string &currentId)
{
    std::error_code errCode;
    if (std::filesystem::exists(sandboxPath) && std::filesystem::is_directory(sandboxPath)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Copy dir");
        std::filesystem::copy(disSandboxPath, sandboxPath,
            std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing, errCode);
        if (errCode.value() != 0) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Copy dir failed: errCode=%{public}d", errCode.value());
            return EIO;
        }
    } else {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Copy file.");
        Uri uri(srcUri);
        auto fileName = GetFileName(uri.GetPath());
        if (fileName.empty()) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Get filename failed");
            RmDir(disSandboxPath);
            return EIO;
        }
        std::filesystem::copy(disSandboxPath + fileName, sandboxPath, std::filesystem::copy_options::update_existing,
            errCode);
        if (errCode.value() != 0) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Copy file failed: errCode=%{public}d", errCode.value());
            return EIO;
        }
    }
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Copy file success.");
    return ERRNO_NOERR;
}

std::string MiscServices::TransListener::GetFileName(const std::string &path)
{
    auto pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "invalid path");
        return "";
    }
    return SandboxHelper::Decode(path.substr(pos));
}

std::string MiscServices::TransListener::GetNetworkIdFromUri(const std::string &uri)
{
    return uri.substr(uri.find(NETWORK_PARA) + NETWORK_PARA.size(), uri.size());
}

int32_t MiscServices::TransListener::OnFileReceive(uint64_t totalBytes, uint64_t processedBytes)
{
    std::lock_guard<std::mutex> lock(callbackMutex_);
    if (callback_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Failed to parse watcher callback");
        return ENOMEM;
    }
    callback_->totalSize = totalBytes;
    callback_->progressSize = processedBytes;
    if (totalBytes == 0) {
        return ENOMEM;
    }
    callback_->percentage = (int32_t)(PERCENTAGE * (processedBytes / totalBytes));
    std::shared_ptr<ProgressInfo> proInfo = std::make_shared<ProgressInfo>();
    proInfo->percentage = callback_->percentage;
    progressListener_.ProgressNotify(proInfo);

    return ERRNO_NOERR;
}

int32_t MiscServices::TransListener::OnFinished(const std::string &sessionName)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "OnFinished");
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_->percentage = PERCENTAGE;
        std::shared_ptr<ProgressInfo> proInfo = std::make_shared<ProgressInfo>();
        proInfo->percentage = callback_->percentage;
        progressListener_.ProgressNotify(proInfo);
        callback_ = nullptr;
    }
    copyEvent_.copyResult = SUCCESS;
    cv_.notify_all();
    return ERRNO_NOERR;
}

int32_t MiscServices::TransListener::OnFailed(const std::string &sessionName, int32_t errorCode)
{
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "OnFailed, errorCode=%{public}d", errorCode);
    {
        std::lock_guard<std::mutex> lock(callbackMutex_);
        callback_ = nullptr;
    }
    copyEvent_.copyResult = FAILED;
    copyEvent_.errorCode = errorCode;
    cv_.notify_all();
    return ERRNO_NOERR;
}
} // namespace MiscServices
} // namespace OHOS