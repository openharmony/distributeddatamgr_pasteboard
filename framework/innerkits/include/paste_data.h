/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef PASTE_BOARD_DATA_H
#define PASTE_BOARD_DATA_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "parcel.h"
#include "paste_data_record.h"
#include "serializable/tlv_object.h"
#include "uri.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace MiscServices {
struct PasteDataProperty {
    AAFwk::WantParams additions;
    std::vector<std::string> mimeTypes;
    std::string tag;
    std::int64_t timestamp;
    bool localOnly;
};

class PasteData : public Parcelable, public TLVObject {
public:
    PasteData() = default;
    explicit PasteData(std::vector<std::shared_ptr<PasteDataRecord>> records);

    void AddHtmlRecord(const std::string &html);
    void AddPixelMapRecord(std::shared_ptr<OHOS::Media::PixelMap> pixelMap);
    void AddTextRecord(const std::string &text);
    void AddUriRecord(const OHOS::Uri &uri);
    void AddWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want);
    void AddRecord(std::shared_ptr<PasteDataRecord> record);
    void AddRecord(PasteDataRecord& record);
    std::vector<std::string> GetMimeTypes();
    std::shared_ptr<std::string> GetPrimaryHtml();
    std::shared_ptr<OHOS::Media::PixelMap> GetPrimaryPixelMap();
    std::shared_ptr<std::string> GetPrimaryText();
    std::shared_ptr<OHOS::Uri> GetPrimaryUri();
    std::shared_ptr<std::string> GetPrimaryMimeType();
    std::shared_ptr<OHOS::AAFwk::Want> GetPrimaryWant();
    std::shared_ptr<PasteDataRecord> GetRecordAt(std::size_t index);
    std::size_t GetRecordCount();
    std::string GetTag();
    bool RemoveRecordAt(std::size_t number);
    bool ReplaceRecordAt(std::size_t number, std::shared_ptr<PasteDataRecord> record);
    bool HasMimeType(const std::string &mimeType);
    PasteDataProperty GetProperty();
    std::vector<std::shared_ptr<PasteDataRecord>> AllRecords() const;

    virtual bool Marshalling(Parcel &parcel) const override;
    static PasteData *Unmarshalling(Parcel &parcel);

private:
    void RefreshMimeProp();
    PasteDataProperty props_;
    std::vector<std::shared_ptr<PasteDataRecord>> records_;
};
} // MiscServices
} // OHOS
#endif // PASTE_BOARD_DATA_H
