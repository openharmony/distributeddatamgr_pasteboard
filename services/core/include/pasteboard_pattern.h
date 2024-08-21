/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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
using Patterns = std::unordered_set<Pattern>;

class PatternChecker {
public:
    virtual bool IsExist(const std::string &content) = 0;
    virtual ~PatternChecker() {}
};

class PatternCheckerFactory {
public:
    static PatternCheckerFactory &GetInstance();
    void InitPatternCheckers();
    std::shared_ptr<PatternChecker> GetPatternChecker(const Pattern &pattern);
private:
    PatternCheckerFactory() {inited_ = false;}
    ~PatternCheckerFactory() {}
    PatternCheckerFactory(const PatternCheckerFactory &) = delete;
    PatternCheckerFactory &operator=(const PatternCheckerFactory &) = delete;
    void RegisterPatternChecker(const Pattern &pattern, std::shared_ptr<PatternChecker> registChecker);
    std::map<Pattern, std::shared_ptr<PatternChecker>> patternCheckers_;
    bool inited_;
};

class URLPatternChecker : public PatternChecker {
public:
    URLPatternChecker() {}
    bool IsExist(const std::string &content) override;
private:
    static std::regex urlRegex_;
};

class NumberPatternChecker : public PatternChecker {
public:
    NumberPatternChecker() {}
    bool IsExist(const std::string &content) override;
private:
    static std::regex numberRegex_;
};

class EmailAddressPatternChecker : public PatternChecker {
public:
    EmailAddressPatternChecker() {}
    bool IsExist(const std::string &content) override;
private:
    static std::regex emailAddressRegex_;
};

void CheckPlainText(Patterns &patternsOut, const Patterns &PatternsIn, const std::string &plainText);
void CheckHTMLText(Patterns &patternsOut, const Patterns &PatternsIn, const std::string &htmlText);
void CheckURI(Patterns &patternsOut, const std::string &uriText);
const Patterns ExistedPatterns(const Patterns &patternsToCheck,
    const std::shared_ptr<PasteData> pasteDataSP,
    const bool hasHTML, const bool hasPlain, const bool hasURI);
} // namespace OHOS::MiscServices

#endif // PASTE_BOARD_SERVICE_H