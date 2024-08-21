/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "pasteboard_pattern.h"

#include <sstream>

namespace OHOS::MiscServices {
PatternCheckerFactory &PatternCheckerFactory::GetInstance()
{
    static PatternCheckerFactory instance;
    return instance;
}

void PatternCheckerFactory::InitPatternCheckers()
{
    if (!inited_) {
        RegisterPatternChecker(Pattern::URL, std::make_shared<URLPatternChecker>());
        RegisterPatternChecker(Pattern::Number, std::make_shared<NumberPatternChecker>());
        RegisterPatternChecker(Pattern::EmailAddress, std::make_shared<EmailAddressPatternChecker>());
        inited_ = true;
    }
}

std::shared_ptr<PatternChecker> PatternCheckerFactory::GetPatternChecker(
    const Pattern &pattern)
{
    auto it = patternCheckers_.find(pattern);
    if (it == patternCheckers_.end()) {
        return nullptr;
    }
    return it->second;
}

void PatternCheckerFactory::RegisterPatternChecker(
    const Pattern &pattern,
    const std::shared_ptr<PatternChecker> checker)
{
    patternCheckers_.insert_or_assign(pattern, checker);
}

bool URLPatternChecker::IsExist(const std::string &content)
{
    return std::regex_search(content, urlRegex_);
}

bool NumberPatternChecker::IsExist(const std::string &content)
{
    return std::regex_search(content, numberRegex_);
}

bool EmailAddressPatternChecker::IsExist(
    const std::string &content)
{
    return std::regex_search(content, emailAddressRegex_);
}

std::regex URLPatternChecker::urlRegex_("(?:(https?|file)://|www\\.)"
    "[-a-z0-9+&@#/%?=~_|!:,.;]*[-a-z0-9+&@#/%=~_]");

std::regex NumberPatternChecker::numberRegex_("[-+]?[0-9]*\\.?[0-9]+");

std::regex EmailAddressPatternChecker::emailAddressRegex_("(([a-zA-Z0-9_\\-\\.]+)@"
    "((?:\\[([0-9]{1,3}\\.){3}[0-9]{1,3}\\])|"
    "([a-zA-Z0-9\\-]+(?:\\.[a-zA-Z0-9\\-]+)*))"
    "([a-zA-Z]{2,}|[0-9]{1,3}))");

const Patterns ExistedPatterns(const Patterns &patternsToCheck,
    const std::shared_ptr<PasteData> pasteDataSP,
    const bool hasHTML, const bool hasPlain, const bool hasURI)
{
    bool needCheckURI = (patternsToCheck.find(Pattern::URL) != patternsToCheck.end());
    PatternCheckerFactory::GetInstance().InitPatternCheckers();
    std::unordered_set<Pattern> existedPatterns;

    for (auto& record : pasteDataSP->AllRecords()) {
        if (patternsToCheck == existedPatterns) {
            break;
        }
        if (hasPlain && record->GetPlainText() != nullptr) {
            std::string recordText = *(record->GetPlainText());
            CheckPlainText(existedPatterns, patternsToCheck, recordText);
        }
        if (hasHTML && record->GetHtmlText() != nullptr) {
            std::string recordText = *(record->GetHtmlText());
            CheckHTMLText(existedPatterns, patternsToCheck, recordText);
        }
        if (needCheckURI && hasURI && record->GetUri() != nullptr &&
            existedPatterns.find(Pattern::URL) == patternsToCheck.end()) {
            std::string recordText = record->GetUri()->ToString();
            CheckURI(existedPatterns, recordText);
        }
    }
    return existedPatterns;
}

void CheckPlainText(Patterns &patternsOut, const Patterns &patternsIn, const std::string &plainText)
{
    for (Pattern pattern : patternsIn) {
        if (patternsOut.find(pattern) != patternsOut.end()) {
            continue;
        }
        std::shared_ptr<PatternChecker> checkerSP = PatternCheckerFactory
        ::GetInstance().GetPatternChecker(pattern);
        if (checkerSP==nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetPatternChecker nullptr error!");
            break;
        }
        if (checkerSP->IsExist(plainText)) {
            patternsOut.insert(pattern);
        }
    }
}

void CheckHTMLText(Patterns &patternsOut, const Patterns &patternsIn, const std::string &htmlText)
{
    // borrow from #include "pasteboard_web_controller.h"
    const std::string IMG_TAG_PATTERN = "<img.*?data-ohos=.*?>";
    const std::string IMG_TAG_SRC_PATTERN = "src=(['\"])(.*?)\\1";
    const std::string imgTagPattern = IMG_TAG_PATTERN + "|" + IMG_TAG_SRC_PATTERN;
    std::regex imgRegex(imgTagPattern);
    // use -1 to remove imgTag and split htmlText and check Patterns
    std::sregex_token_iterator iter(htmlText.begin(), htmlText.end(), imgRegex, -1);
    std::sregex_token_iterator end;
    while (iter != end) {
        CheckPlainText(patternsOut, patternsIn, *iter++);
    }
}
void CheckURI(Patterns &patternsOut, const std::string &uriText)
{
    URLPatternChecker urlChecker;
    if (urlChecker.IsExist(uriText)) {
        patternsOut.insert(Pattern::URL);
    }
}
} // namespace OHOS::MiscServices