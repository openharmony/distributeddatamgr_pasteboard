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

#ifndef PASTEBOARD_TRANS_LISTENER_H
#define PASTEBOARD_TRANS_LISTENER_H

#include <condition_variable>

#include "distributed_file_daemon_manager.h"
#include "file_trans_listener_stub.h"
#include "file_uri.h"
#include "hmdfs_info.h"
#include "pasteboard_copy.h"
#include "pasteboard_client.h"

constexpr int NONE = 0;
constexpr int SUCCESS = 1;
constexpr int FAILED = 2;
namespace OHOS {
namespace MiscServices {
struct CopyEvent {
    int copyResult = NONE;
    int32_t errorCode = 0;
};

class TransListener : public Storage::DistributedFile::FileTransListenerStub {
public:
    int32_t OnFileReceive(uint64_t totalBytes, uint64_t processedBytes);
    int32_t OnFinished(const std::string &sessionName);
    int32_t OnFailed(const std::string &sessionName, int32_t errorCode);
    static int32_t CopyFileFromSoftBus(const std::string &srcUri, const std::string &destUri,
        std::shared_ptr<CopyInfo> copyInfo, std::shared_ptr<CopyCallback> callback,
        std::shared_ptr<GetDataParams> dataParam);
    static std::string GetNetworkIdFromUri(const std::string &uri);
    static void RmDir(const std::string &path);
    static std::string CreateDfsCopyPath();
    static std::string GetFileName(const std::string &path);
    static int32_t CopyToSandBox(const std::string &srcUri, const std::string &disSandboxPath,
        const std::string &sandboxPath, const std::string &currentId);
    static int32_t PrepareCopySession(const std::string &srcUri, const std::string &destUri,
        TransListener* transListener, Storage::DistributedFile::HmdfsInfo &info, std::string &disSandboxPath);
    static int32_t HandleCopyFailure(CopyEvent &copyEvent, const Storage::DistributedFile::HmdfsInfo &info,
        const std::string &disSandboxPath, const std::string &currentId);
    static int WaitForCopyResult(TransListener* transListener);
    static std::atomic<uint32_t> getSequenceId_;
    std::mutex cvMutex_;
    std::condition_variable cv_;
    CopyEvent copyEvent_;
    std::mutex callbackMutex_;
    std::shared_ptr<CopyCallback> callback_;
};
} // namespace MiscServices
} // namespace OHOS

#endif // PASTEBOARD_TRANS_LISTENER_H