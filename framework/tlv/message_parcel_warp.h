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

#ifndef MESSAGE_PARCEL_WARP_H
#define MESSAGE_PARCEL_WARP_H

#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
class MessageParcelWarp {
public:
    MessageParcelWarp();
    ~MessageParcelWarp();

    bool WriteRawData(MessageParcel &parcelPata, const void *data, size_t size);
    const void *ReadRawData(MessageParcel &parcelData, size_t size);

    static inline int64_t maxRawDataSize = 128 * 1024 * 1024; // 128M

private:
    bool MemcpyData(void *ptr, size_t size, const void *data, size_t count);

    std::shared_ptr<char> rawData_;
    int writeRawDataFd_;
    int readRawDataFd_;
    void *kernelMappedWrite_;
    void *kernelMappedRead_;
    size_t rawDataSize_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // MESSAGE_PARCEL_WARP_H