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

#ifndef PASTE_BOARD_PATTERN_H
#define PASTE_BOARD_PATTERN_H

#include <regex>
#include <unordered_set>

#include "paste_data.h"

namespace OHOS::MiscServices {
enum class Pattern : uint32_t { URL = 0, Number, EmailAddress, PatternCount };
using Patterns = std::unordered_set<Pattern>;
  
void CheckPlainText(Patterns &patternsOut, const Patterns &PatternsIn, const std::string &plainText);
// const std::string stripHtmlTags(const std::string &html);
const Patterns DetectPatterns(const Patterns &patternsToCheck,
    const PasteData &pasteData,
    const bool hasHTML, const bool hasPlain, const bool hasURI);
} // namespace OHOS::MiscServices

#endif // PASTE_BOARD_SERVICE_H