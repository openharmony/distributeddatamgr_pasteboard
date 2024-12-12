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
#include "pasteboard_observer.h"
#include "unified_data.h"
#include "uv.h"
#include "want.h"

namespace OHOS {
namespace MiscServices {
using namespace std;
using namespace OHOS::AppExecFwk;
const uint64_t MAX_VALUE = 0x7FFFFFFFFFFFFFFF;

struct ReceiveInfo {
    std::string path;                         // dir name
    std::map<std::string, uint64_t> fileList; // filename, proceededSize
};

#if !defined(WIN_PLATFORM) && !defined(IOS_PLATFORM)
class FileIoToken : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.fileio.open");

    FileIoToken() = default;
    virtual ~FileIoToken() noexcept = default;
};
#endif

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
    int32_t notifyFd = -1;
    int32_t eventFd = -1;
    int exceptionCode = 0;    // notify copy thread or listener thread has exceptions.
    std::set<std::string> filePaths;
    std::chrono::steady_clock::time_point notifyTime;
    bool isFile = false;
    bool run = true;
    bool hasListener = false;
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

void fs_req_cleanup(uv_fs_t* req);
bool IsDirectory(const std::string &path);
bool IsFile(const std::string &path);
bool IsMediaUri(const std::string &uriPath);
int32_t CheckOrCreatePath(const std::string &destPath);
int32_t MakeDir(const string &path);
int32_t RecurCopyDir(const string &srcPath, const string &destPath, std::shared_ptr<CopyInfo> copyInfo);
int32_t CopySubDir(const string &srcPath, const string &destPath, std::shared_ptr<CopyInfo> copyInfo);
int32_t CopyDirFunc(const string &src, const string &dest, std::shared_ptr<CopyInfo> copyInfo);
int32_t CopyFile(const string &src, const string &dest, std::shared_ptr<CopyInfo> copyInfo);
int32_t ExecCopy(std::shared_ptr<CopyInfo> copyInfo);
int32_t ExecLocal(std::shared_ptr<CopyInfo> copyInfo, std::shared_ptr<CopyCallback> callback);
int32_t CopyPasteData(PasteData &pasteData, std::shared_ptr<GetDataParams> dataParams);
uint64_t GetDirSize(std::shared_ptr<CopyInfo> infos, std::string path);
tuple<int, uint64_t> GetFileSize(const std::string &path);
int32_t InitLocalListener(std::shared_ptr<CopyInfo> infos, std::shared_ptr<CopyCallback> callback);
std::shared_ptr<CopyCallback> GetRegisteredListener(std::shared_ptr<CopyInfo> infos);
std::shared_ptr<ReceiveInfo> GetReceivedInfo(int wd, std::shared_ptr<CopyCallback> callback);
bool CheckFileValid(const std::string &filePath, std::shared_ptr<CopyInfo> infos);
int UpdateProgressSize(const std::string &filePath, std::shared_ptr<ReceiveInfo> receivedInfo,
    std::shared_ptr<CopyCallback> callback);
tuple<bool, int, bool> HandleProgress(
    inotify_event *event, std::shared_ptr<CopyInfo> infos, std::shared_ptr<CopyCallback> callback);
void ReadNotifyEvent(std::shared_ptr<CopyInfo> infos);
void GetNotifyEvent(std::shared_ptr<CopyInfo> infos);
void SetNotify(std::shared_ptr<CopyInfo> infos, std::shared_ptr<CopyCallback> callback);
std::string GetRealPath(const std::string& path);
std::shared_ptr<CopyCallback> RegisterListener(const std::shared_ptr<CopyInfo> &infos);
void UnregisterListener(std::shared_ptr<CopyInfo> infos);
void CloseNotifyFd(std::shared_ptr<CopyInfo> infos, std::shared_ptr<CopyCallback> callback);
void WaitNotifyFinished(std::shared_ptr<CopyCallback> callback);
void CopyComplete(std::shared_ptr<CopyInfo> infos, std::shared_ptr<CopyCallback> callback);
bool IsRemoteUri(const std::string &uri);

} // namespace MiscServices
} // namespace OHOS

#endif // PASTE_BOARD_COPY_H