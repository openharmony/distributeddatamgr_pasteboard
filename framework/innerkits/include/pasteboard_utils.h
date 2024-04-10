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

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "paste_data_record.h"
#include "pasteboard_hilog.h"
#include "tlv_object.h"
#include "unified_record.h"
#include "uri.h"
#include "want.h"
#include "want_params.h"
#include "paste_data.h"
#include "unified_data.h"

namespace OHOS {
namespace MiscServices {
class PasteboardUtils{
public:
   using UnifiedRecord = UDMF::UnifiedRecord;
   using UnifiedData = UDMF::UnifiedData;
   using UnifiedDataProperties = UDMF::UnifiedDataProperties;
   using UDType = UDMF::UDType;

   static std::shared_ptr<PasteData> ConvertData(UnifiedData &unifiedData);
   static std::shared_ptr<UnifiedData> ConvertData(PasteData &pasteData);

   static std::vector<std::shared_ptr<PasteDataRecord>> ConvertRecords(std::vector<std::shared_ptr<UnifiedRecord>> &records);
   static std::vector<std::shared_ptr<UnifiedRecord>> ConvertRecords(std::shared_ptr<PasteDataRecord> record);

   static std::shared_ptr<PasteDataRecord> ConvertRecord(std::shared_ptr<UnifiedRecord> record);
   static std::shared_ptr<UnifiedRecord> ConvertRecord(std::shared_ptr<PasteDataRecord> record);

   static PasteDataProperty ConvertProperties(UnifiedDataProperties &properties);
   static std::shared_ptr<UnifiedDataProperties> ConvertProperties(PasteDataProperty &properties);

   static std::vector<std::string> ConvertTypes(std::vector<UDType> &types);
   static std::string ConvertType(UDType uDType);
};
}
}
#endif // PASTE_BOARD_UTILS_H