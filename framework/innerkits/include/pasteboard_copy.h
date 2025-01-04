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

#ifndef PASTE_BOARD_COPY_H
#define PASTE_BOARD_COPY_H

#include <chrono>
#include <condition_variable>
#include <functional>
#include <set>
#include <singleton.h>
#include <sys/inotify.h>
#include <thread>

#include "bundle_mgr_client_impl.h"
#include "i_pasteboard_service.h"
#include "paste_data.h"
#include "paste_data_record.h"
#include "pasteboard_client.h"
#include "pasteboard_delay_getter.h"
#include "pasteboard_fd_guard.h"
#include "pasteboard_observer.h"
#include "unified_data.h"
#include "uv.h"
#include "want.h"

namespace OHOS {
namespace MiscServices {
using namespace OHOS::AppExecFwk;
const uint64_t MAX_VALUE = 0x7FFFFFFFFFFFFFFF;

struct ReceiveInfo {
    std::string path;                         // dir name
    std::map<std::string, uint64_t> fileList; // filename, proceededSize
};

class FileIoToken : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.fileio.open");

    FileIoToken() = default;
    virtual ~FileIoToken() noexcept = default;
};

struct CopyCallback {
    int32_t notifyFd = -1;
    int32_t eventFd = -1;
    std::vector<std::pair<int, std::shared_ptr<ReceiveInfo>>> wds;
    uint64_t totalSize = 0;
    uint64_t progressSize = 0;
    uint64_t maxProgressSize = 0;
    int32_t errorCode = 0;
    int32_t percentage = 0;
    std::thread notifyHandler;
    void CloseFd()
    {
        if (eventFd != -1) {
            close(eventFd);
            eventFd = -1;
        }
        if (notifyFd == -1) {
            return;
        }
        for (auto item : wds) {
            inotify_rm_watch(notifyFd, item.first);
        }
        close(notifyFd);
        notifyFd = -1;
    }

    ~CopyCallback()
    {
        CloseFd();
    }
};

struct CopyInfo {
    std::string srcUri;
    std::string destUri;
    std::string srcPath;
    std::string destPath;
    int32_t index;
    int32_t uriNum;
    int32_t notifyFd = -1;
    int32_t eventFd = -1;
    int exceptionCode = 0;    // notify copy thread or listener thread has exceptions.
    FileConflictOption option = FILE_OVERWRITE;
    std::set<std::string> filePaths;
    std::chrono::steady_clock::time_point notifyTime;
    bool isFile = false;
    bool run = true;
    bool hasListener = false;
    bool isExist = false;
    bool operator==(const CopyInfo &infos) const
    {
        return (srcUri == infos.srcUri && destUri == infos.destUri);
    }
    bool operator<(const CopyInfo &infos) const
    {
        if (srcUri == infos.srcUri) {
            return destUri < infos.destUri;
        }
        return srcUri < infos.srcUri;
    }
};


class PasteBoardCopyFile {
public:
    static PasteBoardCopyFile &GetInstance();
    int32_t CopyPasteData(PasteData &pasteData, std::shared_ptr<GetDataParams> dataParams);
private:
    PasteBoardCopyFile() = default;
    ~PasteBoardCopyFile() = default;
    DISALLOW_COPY_AND_MOVE(PasteBoardCopyFile);
    static void FsReqClean(uv_fs_t *req);
    static bool IsDirectory(const std::string &path);
    static bool IsFile(const std::string &path);
    static bool IsMediaUri(const std::string &uriPath);
    static int32_t CheckOrCreatePath(const std::string &destPath);
    static int32_t MakeDir(const std::string &path);
    static int32_t RecurCopyDir(const std::string &srcPath, const std::string &destPath,
        std::shared_ptr<CopyInfo> copyInfo);
    static int32_t CopySubDir(const std::string &srcPath, const std::string &destPath,
        std::shared_ptr<CopyInfo> copyInfo);
    static int32_t CopyDirFunc(const std::string &src, const std::string &dest, std::shared_ptr<CopyInfo> copyInfo);
    static int32_t CopyFile(const std::string &src, const std::string &dest, std::shared_ptr<CopyInfo> copyInfo);
    static int32_t ExecCopy(std::shared_ptr<CopyInfo> copyInfo);
    static int32_t ExecLocal(std::shared_ptr<CopyInfo> copyInfo, std::shared_ptr<CopyCallback> callback);
    static uint64_t GetDirSize(std::string path);
    static std::tuple<int, uint64_t> GetFileSize(const std::string &path);
    static int32_t InitLocalListener(std::shared_ptr<CopyInfo> infos, std::shared_ptr<CopyCallback> callback);
    static std::shared_ptr<CopyCallback> GetRegisteredListener(std::shared_ptr<CopyInfo> infos);
    static std::shared_ptr<ReceiveInfo> GetReceivedInfo(int wd, std::shared_ptr<CopyCallback> callback);
    static bool CheckFileValid(const std::string &filePath, std::shared_ptr<CopyInfo> infos);
    static int UpdateProgressSize(const std::string &filePath, std::shared_ptr<ReceiveInfo> receivedInfo,
        std::shared_ptr<CopyCallback> callback);
    static std::tuple<bool, int, bool> HandleProgress(inotify_event *event, std::shared_ptr<CopyInfo> infos,
        std::shared_ptr<CopyCallback> callback);
    static void ReadNotifyEvent(std::shared_ptr<CopyInfo> infos);
    static void GetNotifyEvent(std::shared_ptr<CopyInfo> infos);
    static void SetNotify(std::shared_ptr<CopyInfo> infos, std::shared_ptr<CopyCallback> callback);
    static std::string GetRealPath(const std::string& path);
    static std::shared_ptr<CopyCallback> RegisterListener(const std::shared_ptr<CopyInfo> &infos);
    static void UnregisterListener(std::shared_ptr<CopyInfo> infos);
    static void CloseNotifyFd(std::shared_ptr<CopyInfo> infos, std::shared_ptr<CopyCallback> callback);
    static void WaitNotifyFinished(std::shared_ptr<CopyCallback> callback);
    static void CopyComplete(std::shared_ptr<CopyInfo> infos, std::shared_ptr<CopyCallback> callback);
    static bool IsRemoteUri(const std::string &uri);
    static void GetTotalSize(std::map<int32_t, std::string> uriMap, std::shared_ptr<GetDataParams> dataParams);
    static int32_t CheckCopyParam(PasteData &pasteData, std::shared_ptr<GetDataParams> dataParams);
    static std::string GetModeFromFlags(unsigned int flags);
    static int32_t OpenSrcFile(const std::string &srcPath, std::shared_ptr<CopyInfo> copyInfo, int32_t &srcFd);
    static int32_t SendFileCore(std::shared_ptr<MiscServices::FDGuard> srcFdg,
        std::shared_ptr<MiscServices::FDGuard> destFdg, std::shared_ptr<CopyInfo> copyInfo);
    static int32_t InitCopyInfo(const std::string srcUri, std::shared_ptr<GetDataParams> dataParams,
        std::shared_ptr<CopyInfo> copyInfo, int32_t index);
    static int FilterFunc(const struct dirent *filename);
    static void Deleter(struct NameList *arg);
    static void OnProgressNotify(std::shared_ptr<ProgressInfo> proInfo);
    static void ProgressInit(void);
    static int32_t CopyFileData(PasteData &pasteData, std::shared_ptr<GetDataParams> dataParams);

    static std::recursive_mutex mutex_;
    static std::map<CopyInfo, std::shared_ptr<CopyCallback>> cbMap_;
    static ProgressListener progressListener_;
    static PasteBoardCopyFile *instance_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // PASTE_BOARD_COPY_H