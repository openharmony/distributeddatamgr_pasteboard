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

#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include "api/visibility.h"
#include "ashmem.h"
#include "message_parcel_warp.h"
#include "parcel.h"
#include "pasteboard_hilog.h"
#include "securec.h"

namespace OHOS {
namespace MiscServices {

MessageParcelWarp::MessageParcelWarp()
{
    writeRawDataFd_ = -1;
    readRawDataFd_ = -1;
    kernelMappedWrite_ = nullptr;
    kernelMappedRead_ = nullptr;
    rawData_ = nullptr;
    rawDataSize_ = 0;
}

MessageParcelWarp::~MessageParcelWarp()
{
    if (kernelMappedWrite_ != nullptr) {
        ::munmap(kernelMappedWrite_, rawDataSize_);
        kernelMappedWrite_ = nullptr;
    }
    if (kernelMappedRead_ != nullptr) {
        ::munmap(kernelMappedRead_, rawDataSize_);
        kernelMappedRead_ = nullptr;
    }

    if (readRawDataFd_ > 0) {
        ::close(readRawDataFd_);
        readRawDataFd_ = -1;
    }
    if (writeRawDataFd_ > 0) {
        ::close(writeRawDataFd_);
        writeRawDataFd_ = -1;
    }
    rawData_ = nullptr;
    rawDataSize_ = 0;
}

bool MessageParcelWarp::MemcpyData(void *ptr, const void *data, size_t size)
{
    size_t chunkSize = 256 * 1024 * 1024;
    if (size > chunkSize) {
        char* ptrDest = static_cast<char*>(ptr);
        const char* ptrSrc = static_cast<const char*>(data);
        size_t remaining = size;
        size_t offset = 0;
        while (remaining > 0) {
            size_t currentChunkSize = (remaining > chunkSize) ? chunkSize : remaining;
            size_t destSize = size - offset;
            size_t count = currentChunkSize;
            size_t copyCount = (count <= destSize) ? count : destSize;
            if (memcpy_s(ptrDest + offset, copyCount, ptrSrc + offset, currentChunkSize) != EOK) {
                ::munmap(ptr, size);
                PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "memcpy_s failed, size:%{public}zu", size);
                return false;
            }
            offset += currentChunkSize;
            remaining -= currentChunkSize;
        }
    } else {
        if (memcpy_s(ptr, size, data, size) != EOK) {
            ::munmap(ptr, size);
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "memcpy_s failed, size:%{public}zu", size);
            return false;
        }
    }
    return true;
}

bool MessageParcelWarp::WriteRawData(MessageParcel &parcelPata, const void *data, size_t size)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(data != nullptr, false,
        PASTEBOARD_MODULE_COMMON, "data is null");
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(0 < size && size <= maxAshmemDataSize, false,
        PASTEBOARD_MODULE_COMMON, "size invalid, size:%{public}zu", size);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(kernelMappedWrite_ == nullptr, false,
        PASTEBOARD_MODULE_COMMON, "kernelMappedWrite_ not null end.");
    if (!parcelPata.WriteInt64(size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "data WriteInt64 failed end.");
        return false;
    }
    if (size <= minAshmemDataSize) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "use WriteUnpadBuffer end.");
        rawDataSize_ = size;
        return parcelPata.WriteUnpadBuffer(data, size);
    }
    int fd = AshmemCreate("Pasteboard Ashmem", size);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(fd >= 0, false, PASTEBOARD_MODULE_COMMON, "ashmem create failed");
    
    writeRawDataFd_ = fd;
    int result = AshmemSetProt(fd, PROT_READ | PROT_WRITE);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(result >= 0, false, PASTEBOARD_MODULE_COMMON, "ashmem set port failed");
    
    void *ptr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ptr != MAP_FAILED, false,
        PASTEBOARD_MODULE_COMMON, "mmap failed, fd:%{public}d size:%{public}zu", fd, size);

    if (!parcelPata.WriteFileDescriptor(fd)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "write file descriptor failed, size:%{public}zu", size);
        ::munmap(ptr, size);
        return false;
    }
    if (!MemcpyData(ptr, data, size)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "memcpy_s failed, fd:%{public}d size:%{public}zu", fd, size);
        return false;
    }
    kernelMappedWrite_ = ptr;
    rawDataSize_ = size;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_COMMON, "write ashmem end. fd:%{public}d size:%{public}zu", fd, size);
    return true;
}

const void *MessageParcelWarp::ReadRawData(MessageParcel &parcelPata, size_t size)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(size != 0, nullptr,
        PASTEBOARD_MODULE_COMMON, "size invalid, size:%{public}zu", size);
    size_t bufferSize = static_cast<size_t>(parcelPata.ReadInt64());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(bufferSize == size, nullptr,
        PASTEBOARD_MODULE_COMMON,
        "buffer size not equal size, bufferSize:%{public}zu size:%{public}zu", bufferSize, size);
    if (bufferSize <= minAshmemDataSize) {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_COMMON, "use ReadUnpadBuffer end.");
        return parcelPata.ReadUnpadBuffer(size);
    }

    // from /foundation/communication/ipc/ipc/native/src/core/framework/source/message_parcel.cpp
    if (rawData_ != nullptr && writeRawDataFd_ == 0) {
        /* should read fd for move readCursor of parcel */
        if (parcelPata.ReadFileDescriptor()) {
            // do nothing
        }
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(rawDataSize_ == size, nullptr,
            PASTEBOARD_MODULE_COMMON,
            "rawData is received from remote, the rawDataSize:%{public}zu not equal size:%{public}zu",
            rawDataSize_, size);
        return rawData_.get();
    }
    int fd = parcelPata.ReadFileDescriptor();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(fd >= 0, nullptr,
        PASTEBOARD_MODULE_COMMON, "read file descriptor failed fd:%{public}d", fd);
    readRawDataFd_ = fd;

    void *ptr = ::mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ptr != MAP_FAILED, nullptr,
        PASTEBOARD_MODULE_COMMON, "mmap failed, fd:%{public}d size:%{public}zu", fd, size);
    
    kernelMappedRead_ = ptr;
    rawDataSize_ = size;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_COMMON, "read ashmem end. fd:%{public}d size:%{public}zu", fd, size);
    return ptr;
}
}
}