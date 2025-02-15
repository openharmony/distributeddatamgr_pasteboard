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
};

class PasteboardServiceMock : public PasteboardServiceStub {
public:
    int32_t SetPasteData(PasteData &pasteData, const sptr<IPasteboardDelayGetter> delayGetter = nullptr,
        const sptr<IPasteboardEntryGetter> entryGetter = nullptr) override
    {
        (void)pasteData;
        (void)delayGetter;
        (void)entryGetter;
        return 0;
    }

    int32_t GetPasteData(PasteData &data, int32_t &syncTime) override
    {
        (void)data;
        (void)syncTime;
        return 0;
    }

    int32_t GetRecordValueByType(uint32_t dataId, uint32_t recordId, PasteDataEntry &value) override
    {
        (void)dataId;
        (void)recordId;
        (void)value;
        return 0;
    }

    int32_t GetDataSource(std::string &bundleName) override
    {
        (void)bundleName;
        return 0;
    }

    std::map<uint32_t, ShareOption> GetGlobalShareOption(const std::vector<uint32_t> &tokenIds) override
    {
        (void)tokenIds;
        return {};
    }

    int32_t SetGlobalShareOption(const std::map<uint32_t, ShareOption> &globalShareOptions) override
    {
        (void)globalShareOptions;
        return 0;
    }

    int32_t RemoveGlobalShareOption(const std::vector<uint32_t> &tokenIds) override
    {
        (void)tokenIds;
        return 0;
    }

    int32_t SetAppShareOptions(const ShareOption &shareOptions) override
    {
        (void)shareOptions;
        return 0;
    }

    int32_t RemoveAppShareOptions() override
    {
        return 0;
    }

    int32_t RegisterClientDeathObserver(sptr<IRemoteObject> observer) override
    {
        (void)observer;
        return 0;
    }

    bool HasPasteData() override
    {
        return true;
    }

    bool HasDataType(const std::string &mimeType) override
    {
        (void)mimeType;
        return true;
    }

    std::vector<std::string> GetMimeTypes() override
    {
        return {};
    }

    int32_t GetChangeCount(uint32_t &changeCount) override
    {
        return 0;
    }

    bool IsRemoteData() override
    {
        return true;
    }

    std::set<Pattern> DetectPatterns(const std::set<Pattern> &patternsToCheck) override
    {
        (void)patternsToCheck;
        return {};
    }

    void SubscribeObserver(PasteboardObserverType type, const sptr<IPasteboardChangedObserver> &observer) override
    {
        (void)type;
        (void)observer;
    }

    void UnsubscribeObserver(PasteboardObserverType type, const sptr<IPasteboardChangedObserver> &observer) override
    {
        (void)type;
        (void)observer;
    }

    void UnsubscribeAllObserver(PasteboardObserverType type) override
    {
        (void)type;
    }

    void PasteStart(const std::string &pasteId) override
    {
        (void)pasteId;
    }

    void PasteComplete(const std::string &deviceId, const std::string &pasteId) override
    {
        (void)deviceId;
        (void)pasteId;
    }

    void Clear() override
    {
    }

    int32_t GetRemoteDeviceName(std::string &deviceName, bool &isRemote) override
    {
        (void)deviceName;
        (void)isRemote;
        return 0;
    }

    void ShowProgress(const std::string &progressKey, const sptr<IRemoteObject> &observer) override
    {
        (void)progressKey;
        (void)observer;
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
