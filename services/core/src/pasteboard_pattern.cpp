#include "pasteboard_service.h"

#include <bitset>
#include <unistd.h>

namespace OHOS::MiscServices {
PasteboardService::PatternCheckerFactory &PasteboardService::PatternCheckerFactory::
GetInstance()
{
    static PatternCheckerFactory instance;
    return instance;
}
void PasteboardService::PatternCheckerFactory::InitPatternCheckers()
{
    if (!inited_) {
        RegisterPatternChecker(Pattern::URL, std::make_shared<URLPatternChecker>());
        RegisterPatternChecker(Pattern::Number, std::make_shared<NumberPatternChecker>());
        RegisterPatternChecker(Pattern::EmailAddress, std::make_shared<EmailAddressPatternChecker>());
        inited_ = true;
    }
}
std::shared_ptr<PasteboardService::PatternChecker> PasteboardService::
PatternCheckerFactory::GetPatternChecker(const Pattern &pattern)
{
    auto it = patternCheckers_.find(pattern);
    if (it == patternCheckers_.end()) {
        return nullptr;
    }
    return it->second;
}
void PasteboardService::PatternCheckerFactory::RegisterPatternChecker(
    const Pattern &pattern, 
    const std::shared_ptr<PasteboardService::PatternChecker> checker)
{
    patternCheckers_.insert_or_assign(pattern, checker);
}
bool PasteboardService::URLPatternChecker::IsExist(const std::string &content)
{
    return std::regex_search(content, urlRegex_);
}
bool PasteboardService::NumberPatternChecker::IsExist(const std::string &content)
{
    return std::regex_search(content, numberRegex_);
}
bool PasteboardService::EmailAddressPatternChecker::IsExist(
    const std::string &content)
{
    return std::regex_search(content, emailAddressRegex_);
}
std::regex PasteboardService::URLPatternChecker::
urlRegex_("(?:(https?|ftp|file)://|www\\.)"
    "[-a-z0-9+&@#/%?=~_|!:,.;]*[-a-z0-9+&@#/%=~_]");
std::regex PasteboardService::NumberPatternChecker::
numberRegex_("[-+]?[0-9]*\\.?[0-9]+");
std::regex PasteboardService::EmailAddressPatternChecker::
emailAddressRegex_("(([a-zA-Z0-9_\\-\\.]+)@"
    "((?:\\[([0-9]{1,3}\\.){3}[0-9]{1,3}\\])|"
    "([a-zA-Z0-9\\-]+(?:\\.[a-zA-Z0-9\\-]+)*))"
    "([a-zA-Z]{2,}|[0-9]{1,3}))");

std::unordered_set<Pattern> PasteboardService::ExistedPatterns(const std::unordered_set<Pattern> &patternsToCheck)
{
    // 1
    bool hasHTML = HasDataType(MIMETYPE_TEXT_HTML);
    bool hasPlain = HasDataType(MIMETYPE_TEXT_PLAIN);
    if (!hasHTML && !hasPlain) {
        return {};   
    }
    // 2
    int32_t userId = GetCurrentAccountId();
    std::shared_ptr<PasteData> pasteDataSP = clips_.Find(userId).second;
    // 3
    PatternCheckerFactory::GetInstance().InitPatternCheckers();
    std::unordered_set<Pattern> existedPatterns;
    // 4
    int recordsSize = pasteDataSP->GetRecordCount();
    for (int i = 0; i != recordsSize; i++) {
        std::shared_ptr<PasteDataRecord> record = pasteDataSP->GetRecordAt(i);
        // 4.1
        if (patternsToCheck == existedPatterns) {
            break;
        }
        // 4.2
        if (!hasPlain) {
            break;
        }
        if (record->GetPlainText() != nullptr) {
            std::string recordText = *(record->GetPlainText());
            for (Pattern pattern : patternsToCheck) {
                if (existedPatterns.find(pattern) != existedPatterns.end()) {
                    continue;
                }
                std::shared_ptr<PatternChecker> checkerSP = PatternCheckerFactory
                ::GetInstance().GetPatternChecker(pattern);
                if (checkerSP==nullptr){
                    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "GetPatternChecker nullptr error!");
                    break;
                }
                if (checkerSP->IsExist(recordText)) {
                    existedPatterns.insert(pattern);
                }
            }
        }
    }
    // 5
    return existedPatterns;
}
}// namespace OHOS::MiscServices