/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "pasteboardservice_fuzzer.h"

#include <thread>

#include "loader.h"
#include "message_parcel.h"
#include "pasteboard_service.h"
#include "pasteboard_serv_ipc_interface_code.h"

namespace {
using namespace OHOS;
using namespace OHOS::MiscServices;
using namespace OHOS::Security::PasteboardServ;

const std::u16string PASTEBOARDSERVICE_INTERFACE_TOKEN = u"ohos.miscservices.pasteboard.IPasteboardService";
const std::vector<PasteboardServiceInterfaceCode> CODE_LIST = {
    GET_PASTE_DATA,
    HAS_PASTE_DATA,
    SET_PASTE_DATA,
    CLEAR_ALL,
    SUBSCRIBE_OBSERVER,
    UNSUBSCRIBE_OBSERVER,
    UNSUBSCRIBE_ALL_OBSERVER,
    IS_REMOTE_DATA,
    GET_DATA_SOURCE,
    HAS_DATA_TYPE,
    SET_GLOBAL_SHARE_OPTION,
    REMOVE_GLOBAL_SHARE_OPTION,
    GET_GLOBAL_SHARE_OPTION,
    SET_APP_SHARE_OPTIONS,
    REMOVE_APP_SHARE_OPTIONS,
    PASTE_START,
    PASTE_COMPLETE,
    REGISTER_CLIENT_DEATH_OBSERVER,
    DETECT_PATTERNS,
    GET_RECORD_VALUE,
    GET_MIME_TYPES,
    GET_REMOTE_DEVICE_NAME,
    SHOW_PROGRESS,
    GET_CHANGE_COUNT,
    SUBSCRIBE_ENTITY_OBSERVER,
    UNSUBSCRIBE_ENTITY_OBSERVER,
};

class PasteboardServiceMock : public PasteboardServiceStub {
public:
    int32_t SetPasteData(int fd, int64_t memSize, const std::vector<uint8_t> &buffer,
        const sptr<IPasteboardDelayGetter> &delayGetter, const sptr<IPasteboardEntryGetter> &entryGetter) override
    {
        (void)fd;
        (void)memSize;
        (void)buffer;
        (void)delayGetter;
        (void)entryGetter;
        return 0;
    }

    int32_t SetPasteDataDelayData(int fd, int64_t memSize, const std::vector<uint8_t> &buffer,
        const sptr<IPasteboardDelayGetter> &delayGetter) override
    {
        (void)fd;
        (void)memSize;
        (void)buffer;
        (void)delayGetter;
        return 0;
    }

    int32_t SetPasteDataEntryData(int fd, int64_t memSize, const std::vector<uint8_t> &buffer,
        const sptr<IPasteboardEntryGetter> &entryGetter) override
    {
        (void)fd;
        (void)memSize;
        (void)buffer;
        (void)entryGetter;
        return 0;
    }

    int32_t SetPasteDataOnly(int fd, int64_t memSize, const std::vector<uint8_t> &buffer) override
    {
        (void)fd;
        (void)memSize;
        (void)buffer;
        return 0;
    }

    int32_t GetPasteData(int &fd, int64_t &memSize, std::vector<uint8_t> &buffer,
        const std::string &pasteId, int32_t &syncTime) override
    {
        (void)fd;
        (void)memSize;
        (void)buffer;
        (void)pasteId;
        (void)syncTime;
        return 0;
    }

    int32_t GetRecordValueByType(uint32_t dataId, uint32_t recordId, int64_t &rawDataSize,
        std::vector<uint8_t> &buffer, int &fd) override
    {
        (void)dataId;
        (void)recordId;
        (void)rawDataSize;
        (void)buffer;
        (void)fd;
        return 0;
    }

    int32_t GetDataSource(std::string &bundleName) override
    {
        (void)bundleName;
        return 0;
    }

    int32_t GetGlobalShareOption(const std::vector<uint32_t> &tokenIds,
        std::unordered_map<uint32_t, int32_t>& funcResult) override
    {
        (void)tokenIds;
        (void)funcResult;
        return 0;
    }

    int32_t SetGlobalShareOption(const std::unordered_map<uint32_t, int32_t> &globalShareOptions) override
    {
        (void)globalShareOptions;
        return 0;
    }

    int32_t RemoveGlobalShareOption(const std::vector<uint32_t> &tokenIds) override
    {
        (void)tokenIds;
        return 0;
    }

    int32_t SetAppShareOptions(int32_t shareOptions) override
    {
        (void)shareOptions;
        return 0;
    }

    int32_t RemoveAppShareOptions() override
    {
        return 0;
    }

    int32_t RegisterClientDeathObserver(const sptr<IRemoteObject> &observer) override
    {
        (void)observer;
        return 0;
    }

    int32_t HasPasteData(bool& funcResult) override
    {
        (void)funcResult;
        return 0;
    }

    int32_t HasDataType(const std::string &mimeType, bool& funcResult) override
    {
        (void)mimeType;
        (void)funcResult;
        return 0;
    }

    int32_t GetMimeTypes(std::vector<std::string>& funcResult) override
    {
        (void)funcResult;
        return 0;
    }

    int32_t GetChangeCount(uint32_t &changeCount) override
    {
        return 0;
    }

    int32_t SubscribeEntityObserver(
        EntityType entityType, uint32_t expectedDataLength, const sptr<IEntityRecognitionObserver> &observer) override
    {
        (void)entityType;
        (void)expectedDataLength;
        (void)observer;
        return 0;
    }

    int32_t UnsubscribeEntityObserver(
        EntityType entityType, uint32_t expectedDataLength,  const sptr<IEntityRecognitionObserver> &observer) override
    {
        (void)entityType;
        (void)expectedDataLength;
        (void)observer;
        return 0;
    }

    int32_t IsRemoteData(bool& funcResult) override
    {
        (void)funcResult;
        return 0;
    }

    int32_t DetectPatterns(const std::vector<Pattern> &patternsToCheck,
        std::vector<Pattern>& funcResult) override
    {
        (void)patternsToCheck;
        (void)funcResult;
        return 0;
    }

    int32_t SubscribeObserver(PasteboardObserverType type, const sptr<IPasteboardChangedObserver> &observer) override
    {
        (void)type;
        (void)observer;
        return 0;
    }

    int32_t UnsubscribeObserver(PasteboardObserverType type, const sptr<IPasteboardChangedObserver> &observer) override
    {
        (void)type;
        (void)observer;
        return 0;
    }

    int32_t UnsubscribeAllObserver(PasteboardObserverType type) override
    {
        (void)type;
        return 0;
    }

    int32_t PasteStart(const std::string &pasteId) override
    {
        (void)pasteId;
        return 0;
    }

    int32_t PasteComplete(const std::string &deviceId, const std::string &pasteId) override
    {
        (void)deviceId;
        (void)pasteId;
        return 0;
    }

    int32_t Clear() override
    {
        return 0;
    }

    int32_t GetRemoteDeviceName(std::string &deviceName, bool &isRemote) override
    {
        (void)deviceName;
        (void)isRemote;
        return 0;
    }

    int32_t ShowProgress(const std::string &progressKey, const sptr<IRemoteObject> &observer) override
    {
        (void)progressKey;
        (void)observer;
        return 0;
    }
};

static inline void DoSleep(void)
{
    constexpr uint32_t SLEEP_MS = 10;
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
}

class TestEnv {
public:
    TestEnv()
    {
        isInited_ = false;
        stub_ = new PasteboardServiceMock();
        if (stub_ == nullptr) {
            return;
        }
        isInited_ = true;
    }

    ~TestEnv()
    {
        isInited_ = false;
        stub_ = nullptr;
    }

    bool IsInited() const noexcept
    {
        return isInited_;
    }

    void DoRemoteRequest(PasteboardServiceInterfaceCode code, MessageParcel &data)
    {
        MessageParcel reply;
        MessageOption option;
        if (stub_ != nullptr) {
            stub_->OnRemoteRequest(static_cast<uint32_t>(code), data, reply, option);
        }
    }

private:
    volatile bool isInited_;
    sptr<PasteboardServiceMock> stub_;
};
} // anonymous namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    static TestEnv env;
    if (!env.IsInited()) {
        return 0;
    }

    if (data == nullptr || size == 0) {
        return 0;
    }
    PasteboardServiceInterfaceCode code = CODE_LIST[data[0] % CODE_LIST.size()];

    OHOS::MessageParcel parcel;
    parcel.WriteInterfaceToken(PASTEBOARDSERVICE_INTERFACE_TOKEN);
    parcel.WriteBuffer(data + 1, size - 1);

    env.DoRemoteRequest(code, parcel);
    DoSleep();
    return 0;
}
