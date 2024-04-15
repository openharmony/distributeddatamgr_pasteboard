/*
* Copyright (C) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PASTE_BOARD_UTILS_H
#define PASTE_BOARD_UTILS_H

#include "paste_data.h"
#include "paste_data_record.h"
#include "data/unified_data.h"
#include "data/unified_record.h"
namespace OHOS {
namespace MiscServices {
class PasteboardUtils {
public:
    using UnifiedRecord = UDMF::UnifiedRecord;
    using UnifiedData = UDMF::UnifiedData;
    using UnifiedDataProperties = UDMF::UnifiedDataProperties;
    using UDType = UDMF::UDType;
    static PasteboardUtils* GetInstance();
    PasteboardUtils();
    std::shared_ptr<PasteData> UnifiedData2PasteData(const UnifiedData& unifiedData);
    std::shared_ptr<UnifiedData> PasteData2UnifiedData(PasteData& pasteData);

private:
    std::vector<std::shared_ptr<PasteDataRecord>> UnifiedRecords2PasteRecords(
        std::vector<std::shared_ptr<UnifiedRecord>>& records);
    static std::vector<std::shared_ptr<UnifiedRecord>> Custom2UnifiedRecord(
        const std::shared_ptr<PasteDataRecord>& record);

    static PasteDataProperty UnifiedProp2PaseteProp(UnifiedDataProperties& properties);
    static std::shared_ptr<UnifiedDataProperties> PaseteProp2UnifiedProp(PasteDataProperty& properties);

    static std::vector<std::string> UtdTypes2PaseteTypes(std::vector<UDType>& uDTypes);
    static std::string UtdType2PaseteType(UDType uDType);
    static UDType PaseteType2UDType(int32_t uDType, const std::string& mimeType);

    using PasteRecord2UnifiedRecordFunc =
        std::function<std::shared_ptr<UnifiedRecord>(std::shared_ptr<PasteDataRecord>)>;
    using UnifiedRecord2PasteRecordFunc =
        std::function<std::shared_ptr<PasteDataRecord>(std::shared_ptr<UnifiedRecord>)>;
    void InitDecodeMap();

    static std::shared_ptr<UnifiedRecord> PasteRecord2Text(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2PlaintText(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Want(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Html(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Link(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2File(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Image(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Video(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Audio(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Folder(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2PixelMap(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2SystemDefined(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2AppItem(const std::shared_ptr<PasteDataRecord>& record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Form(const std::shared_ptr<PasteDataRecord>& record);

    static std::shared_ptr<PasteDataRecord> Text2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> PlainText2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> Want2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> Html2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> Link2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> File2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> Image2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> Video2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> Audio2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> Folder2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> PixelMap2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> AppItem2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> Form2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> SystemDefined2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);
    static std::shared_ptr<PasteDataRecord> AppDefined2PasteRecord(const std::shared_ptr<UnifiedRecord>& record);

    std::map<int32_t, PasteRecord2UnifiedRecordFunc> pasteRecords2UnifiedRecordMap_;
    std::map<int32_t, UnifiedRecord2PasteRecordFunc> unifiedRecord2PasteRecordsMap_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_UTILS_H