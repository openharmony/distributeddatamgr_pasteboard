/*
 * Copyright (c)2024 Huawei Device Co., Ltd.
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

#include <unordered_map>
#include <libxml/HTMLparser.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

namespace OHOS::MiscServices {
static const std::unordered_map<uint32_t, std::string> patternToRegexMap = {
    { static_cast<uint32_t>(Pattern::URL), std::string("[a-zA-Z0-9+.-]+://[-a-zA-Z0-9+&@#/%?"
                                                "=~_|!:,.;]*[-a-zA-Z0-9+&@#/%=~_]")},
    { static_cast<uint32_t>(Pattern::Number), std::string("[-+]?[0-9]*\\.?[0-9]+")},
    { static_cast<uint32_t>(Pattern::EmailAddress), std::string("(([a-zA-Z0-9_\\-\\.]+)@"
                                                "((?:\\[([0-9]{1,3}\\.){3}[0-9]{1,3}\\])|"
                                                "([a-zA-Z0-9\\-]+(?:\\.[a-zA-Z0-9\\-]+)*))"
                                                "([a-zA-Z]{2,}|[0-9]{1,3}))")},
};

void CheckPlainText(Patterns &patternsOut, const Patterns &patternsIn, const std::string &plainText)
{
    for (Pattern pattern : patternsIn) {
        if (patternsOut.find(pattern) != patternsOut.end()) {
            continue;
        }
        uint32_t patternUint32 = static_cast<uint32_t>(pattern);
        if (patternToRegexMap.find(patternUint32) == patternToRegexMap.end()) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_SERVICE, "pasteboard pattern, unexpected Pattern value!");
            continue;
        }
        std::regex curRegex(patternToRegexMap.at(patternUint32));
        if (std::regex_search(plainText, curRegex)) {
            patternsOut.insert(pattern);
        }
    }
}

std::string stringAppend(const std::string& dest, const char* src)
{
    return dest + std::string(src) + "\n";
}
std::string extractLeafNodes(xmlNode* a_node)
{
    std::string result;
    if (a_node == nullptr) return result;
    if (a_node->type == XML_TEXT_NODE) {
        xmlChar* trimmed = xmlNodeGetContent(a_node);
        if (trimmed) {
            xmlChar* trimmed_final = xmlStrstrim(trimmed, " \t\n");
            if (trimmed_final) {
                result = stringAppend(result, (const char*)trimmed_final);
                xmlFree(trimmed_final);
            }
            xmlFree(trimmed);
        }
    } else if (a_node->type == XML_ELEMENT_NODE) {
        for (xmlNode* child = a_node->children; child != nullptr; child = child->next) {
            result = stringAppend(result, extract_leaf_nodes(child));
        }
    }
    return result;
}
const std::string extractHtmlContent(const std::string& html_str)
{
    xmlDocPtr doc = htmlReadMemory(html_str.c_str(), html_str.size(), nullptr, nullptr, 0);
    if (doc == nullptr) {
        std::cerr << "Error parsing HTML" << std::endl;
        return "";
    }
    std::string result = extractLeafNodes(xmlDocGetRootElement(doc));
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return result;
}
const Patterns DetectPatterns(const Patterns &patternsToCheck,
    const PasteData &pasteData,
    const bool hasHTML, const bool hasPlain)
{
    std::unordered_set<Pattern> existedPatterns;
    for (auto& record : pasteData.AllRecords()) {
        if (patternsToCheck == existedPatterns) {
            break;
        }
        if (hasPlain && record->GetPlainText() != nullptr) {
            std::string recordText = *(record->GetPlainText());
            CheckPlainText(existedPatterns, patternsToCheck, recordText);
        }
        if (hasHTML && record->GetHtmlText() != nullptr) {
            std::string htmlText = *(record->GetHtmlText());
            std::string recordText = extractHtmlContent(htmlText);
            CheckPlainText(existedPatterns, patternsToCheck, recordText);
        }
    }
    return existedPatterns;
}
} // namespace OHOS::MiscServices