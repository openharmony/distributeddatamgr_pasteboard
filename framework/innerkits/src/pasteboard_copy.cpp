/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "pasteboard_copy.h"

#include <filesystem>
#include "copy/file_copy_manager.h"
#include "copy/file_size_utils.h"
#include "file_uri.h"
#include "paste_data.h"
#include "paste_data_record.h"
#include "pasteboard_error.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
using namespace AppFileService::ModuleFileUri;
const std::string NETWORK_PARA = "?networkid=";
constexpr int PERCENTAGE = 100;
constexpr int ERRNO_NOERR = 0;
constexpr int E_EXIST = 17;
constexpr float FILE_PERCENTAGE = 0.8;
constexpr int BEGIN_PERCENTAGE = 20;
constexpr int FILE_TO_KILO_BYTE = 1024;
constexpr int DFS_CANCEL_SUCCESS = 204;

static int32_t g_recordSize = 0;
ProgressListener PasteBoardCopyFile::progressListener_;

PasteBoardCopyFile &PasteBoardCopyFile::GetInstance()
{
    static PasteBoardCopyFile instance;
    return instance;
}

bool PasteBoardCopyFile::IsDirectory(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Stat failed. errno=%{public}d", errno);
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFDIR;
}

bool PasteBoardCopyFile::IsFile(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Stat failed. errno = %{public}d", errno);
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFREG;
}

void PasteBoardCopyFile::OnProgressNotify(std::shared_ptr<ProgressInfo> proInfo)
{
    if (proInfo->percentage > PERCENTAGE) {
        proInfo->percentage = PERCENTAGE;
    }
    proInfo->percentage = static_cast<int32_t>(proInfo->percentage * FILE_PERCENTAGE + BEGIN_PERCENTAGE);
    proInfo->percentage = std::abs(proInfo->percentage);
    proInfo->percentage = std::max(proInfo->percentage, 0);
    if (progressListener_.ProgressNotify != nullptr) {
        progressListener_.ProgressNotify(proInfo);
    } else {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "ProgressNotify is nullptr.");
    }
}

std::string PasteBoardCopyFile::GetRealPath(const std::string& path)
{
    std::filesystem::path tempPath(path);
    std::filesystem::path realPath{};
    for (const auto& component : tempPath) {
        if (component == ".") {
            continue;
        } else if (component == "..") {
            realPath = realPath.parent_path();
        } else {
            realPath /= component;
        }
    }
    return realPath.string();
}

bool PasteBoardCopyFile::IsRemoteUri(const std::string &uri)
{
    return uri.find(NETWORK_PARA) != uri.npos;
}

int32_t PasteBoardCopyFile::CheckCopyParam(PasteData &pasteData, std::shared_ptr<GetDataParams> dataParams)
{
    if (dataParams == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Invalid dataParams");
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    g_recordSize = (int32_t)pasteData.GetRecordCount();
    if (g_recordSize <= 0) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Invalid records size");
        return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

int32_t PasteBoardCopyFile::InitCopyInfo(const std::string srcUri, std::shared_ptr<GetDataParams> dataParams,
    std::shared_ptr<CopyInfo> copyInfo, int32_t index)
{
    copyInfo->srcUri = srcUri;
    copyInfo->destUri = dataParams->destUri;
    FileUri srcFileUri(copyInfo->srcUri);
    copyInfo->srcPath = srcFileUri.GetRealPath();
    FileUri destFileUri(copyInfo->destUri);
    copyInfo->destPath = destFileUri.GetPath();
    copyInfo->srcPath = GetRealPath(copyInfo->srcPath);
    copyInfo->destPath = GetRealPath(copyInfo->destPath);
    std::string realSrc = copyInfo->srcPath;
    if (IsRemoteUri(copyInfo->srcUri)) {
        uint32_t index = copyInfo->srcPath.rfind("?", 0);
        realSrc = copyInfo->srcPath.substr(0, index);
    }
    if (IsDirectory(copyInfo->destPath)) {
        std::filesystem::path filePath(realSrc);
        auto fileName = filePath.filename();
        if (copyInfo->destUri.back() != '/') {
            copyInfo->destUri += '/';
        }
        copyInfo->destUri += fileName.string();
        FileUri realDest(copyInfo->destUri);
        copyInfo->destPath = realDest.GetPath();
        copyInfo->destPath = GetRealPath(copyInfo->destPath);
    }
    std::error_code errCode;
    if (std::filesystem::exists(copyInfo->destPath, errCode) && errCode.value() == ERRNO_NOERR &&
        dataParams->fileConflictOption == FILE_SKIP) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "File has existed.");
        copyInfo->isExist = true;
        return E_EXIST;
    }
    FileUri realFileUri(realSrc);
    std::string realPath = realFileUri.GetRealPath();
    realPath = GetRealPath(realPath);
    if (!IsFile(realPath)) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Softbus not support dir to remote.");
        return ENOMEM;
    }
    return ERRNO_NOERR;
}

int32_t PasteBoardCopyFile::CopyFileData(PasteData &pasteData, std::shared_ptr<GetDataParams> dataParams)
{
    int32_t ret = static_cast<int32_t>(PasteboardError::E_OK);
    progressListener_ = dataParams->listener;
    std::shared_ptr<PasteDataRecord> record = std::make_shared<PasteDataRecord>();
    int32_t index = 0;
    int32_t recordCount = 0;
    int32_t recordProcessedIndex = 0;
    while (index < (int32_t)pasteData.GetRecordCount()) {
        recordProcessedIndex++;
        if (ProgressSignalClient::GetInstance().CheckCancelIfNeed()) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "canceled success!");
            pasteData.RemoveRecordAt(index);
            continue;
        }
        record = pasteData.GetRecordAt(index);
        if (record == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Record is nullptr");
            return static_cast<int32_t>(PasteboardError::INVALID_PARAM_ERROR);
        }
        std::shared_ptr<OHOS::Uri> uri = record->GetUri();
        if (uri == nullptr) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Record has no uri");
            index++;
            continue;
        }
        std::shared_ptr<CopyInfo> copyInfo = std::make_shared<CopyInfo>();
        std::string srcUri = uri->ToString();
        if (InitCopyInfo(srcUri, dataParams, copyInfo, recordCount) == E_EXIST) {
            recordCount++;
            pasteData.RemoveRecordAt(index);
            continue;
        }
        recordCount++;

        using ProcessCallBack = std::function<void(uint64_t processSize, uint64_t totalSize)>;
        ProcessCallBack listener = [&](uint64_t processSize, uint64_t totalSize) {
            HandleProgress(recordProcessedIndex, srcUri, copyInfo->destUri, processSize, totalSize);
        };
        ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy(srcUri, copyInfo->destUri, listener);
        if ((ret == static_cast<int32_t>(PasteboardError::E_OK) || ret == ERRNO_NOERR) && !copyInfo->isExist &&
            !ProgressSignalClient::GetInstance().CheckCancelIfNeed()) {
            auto sharedUri = std::make_shared<OHOS::Uri>(copyInfo->destUri);
            record->SetUri(sharedUri);
            index++;
        } else {
            pasteData.RemoveRecordAt(index);
        }
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "DFS copy ret: %{public}d", ret);
    }
    return (ret == ERRNO_NOERR || ret == DFS_CANCEL_SUCCESS) ? static_cast<int32_t>(PasteboardError::E_OK) : ret;
}

void PasteBoardCopyFile::HandleProgress(int32_t index, const std::string &srcUri, const std::string &destUri,
    uint64_t processSize, uint64_t totalSize)
{
    if (index < 1) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "invalid index: %{public}d", index);
        return;
    }

    if (ProgressSignalClient::GetInstance().CheckCancelIfNeed()) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "Cancel copy.");
        std::thread([&]() {
            auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Cancel(srcUri, destUri);
            if (ret != ERRNO_NOERR) {
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Cancel failed. errno=%{public}d", ret);
            }
        }).detach();
        return;
    }
    auto totalKBytes = totalSize / FILE_TO_KILO_BYTE;
    auto processedKBytes = processSize / FILE_TO_KILO_BYTE;
    int32_t percentage = (int32_t)((PERCENTAGE * processedKBytes) / totalKBytes);
    int32_t totalProgress = ((index - 1) * PERCENTAGE + percentage) / g_recordSize;
    std::shared_ptr<ProgressInfo> proInfo = std::make_shared<ProgressInfo>();
    proInfo->percentage = totalProgress;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "process record index:%{public}d/%{public}d, progress=%{public}d",
        index, g_recordSize, totalProgress);
    OnProgressNotify(proInfo);
}

int32_t PasteBoardCopyFile::CopyPasteData(PasteData &pasteData, std::shared_ptr<GetDataParams> dataParams)
{
    g_recordSize = 0;
    int32_t ret = CheckCopyParam(pasteData, dataParams);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Invalid copy params");
        g_recordSize = 0;
        return ret;
    }
    ret = CopyFileData(pasteData, dataParams);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "copy file failed, ret=%{public}d", ret);
        ret = static_cast<int32_t>(PasteboardError::COPY_FILE_ERROR);
    }
    std::shared_ptr<ProgressInfo> proInfo = std::make_shared<ProgressInfo>();
    proInfo->percentage = PERCENTAGE;
    OnProgressNotify(proInfo);
    g_recordSize = 0;
    return ret;
}
} // namespace MiscServices
} // namespace OHOS