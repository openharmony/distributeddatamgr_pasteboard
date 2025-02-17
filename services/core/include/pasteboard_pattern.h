/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef PASTE_BOARD_PATTERN_H
#define PASTE_BOARD_PATTERN_H

#include <regex>

#include "paste_data.h"

namespace OHOS::MiscServices {
enum class Pattern : uint32_t { URL = 0, NUMBER, EMAIL_ADDRESS, COUNT };
class PatternDetection {
public:
    static const std::set<Pattern> Detect(
        const std::set<Pattern> &patternsToCheck, const PasteData &pasteData, bool hasHTML, bool hasPlain);
    static bool IsValid(const std::set<Pattern> &patterns);

private:
    static std::string ExtractHtmlContent(const std::string &html_str);
    static void DetectPlainText(
        std::set<Pattern> &patternsOut, const std::set<Pattern> &PatternsIn, const std::string &plainText);

    static std::map<uint32_t, std::string> patterns_;
};
} // namespace OHOS::MiscServices
#endif // PASTE_BOARD_PATTERN_H