/*
 * Copyright (C) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef WEB_CLIPBORD_CONTROLLER_H
#define WEB_CLIPBORD_CONTROLLER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "paste_data.h"
#include "paste_data_record.h"
#include "refbase.h"

namespace OHOS {
namespace MiscServices {

class API_EXPORT PasteboardWebController : public RefBase {
public:
    PasteboardWebController() {};
    ~PasteboardWebController() {};

    static PasteboardWebController& GetInstance();

    std::shared_ptr<PasteData> SplitHtml(std::shared_ptr<std::string> html) noexcept;
    std::shared_ptr<std::string> RebuildHtml(std::shared_ptr<PasteData> pasteData) noexcept;

private:
    std::vector<std::pair<std::string, uint32_t>> SplitHtmlWithImgLabel(
        const std::shared_ptr<std::string> html) noexcept;
    std::map<std::string, std::vector<uint8_t>> SplitHtmlWithImgSrcLabel(
        const std::vector<std::pair<std::string, uint32_t>>& matchVec) noexcept;
    std::shared_ptr<PasteData> BuildPasteData(
        std::shared_ptr<std::string> html, const std::map<std::string, std::vector<uint8_t>>& imgSrcMap) noexcept;
    void RemoveAllRecord(std::shared_ptr<PasteData> pasteData) noexcept;
    bool IsLocalURI(std::string& uri) noexcept;
};
} // namespace MiscServices
} // namespace OHOS

#endif // WEB_CLIPBORD_CONTROLLER_H
