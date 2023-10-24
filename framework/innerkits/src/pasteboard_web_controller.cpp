/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include <regex>
#include "file_uri.h"
#include "pasteboard_web_controller.h"

namespace {
const std::string IMG_TAG_PATTERN = "<img.*?data-ohos=.*?>";
const std::string IMG_TAG_SRC_PATTERN = "src=(['\"])(.*?)\\1";
const std::string IMG_TAG_SRC_HEAD = "src=\"";
const std::string IMG_LOCAL_URI = "file:///";
const std::string IMG_LOCAL_PATH = "://";
constexpr uint32_t FOUR_BYTES = 4;
constexpr uint32_t EIGHT_BIT = 8;

struct Cmp {
    bool operator()(const uint32_t& lhs, const uint32_t& rhs) const
    {
        return lhs > rhs;
    }
};
} // namespace

namespace OHOS {
namespace MiscServices {

// static
PasteboardWebController& PasteboardWebController::GetInstance()
{
    static PasteboardWebController instance;
    return instance;
}

std::shared_ptr<PasteData> PasteboardWebController::SplitHtml(std::shared_ptr<std::string> html) noexcept
{
    std::vector<std::pair<std::string, uint32_t>> matchVec = SplitHtmlWithImgLabel(html);
    std::map<std::string, std::vector<uint8_t>> imgSrcMap = SplitHtmlWithImgSrcLabel(matchVec);
    std::shared_ptr<PasteData> pasteData = BuildPasteData(html, imgSrcMap);
    return pasteData;
}

std::shared_ptr<std::string> PasteboardWebController::RebuildHtml(
    std::shared_ptr<PasteData> pasteData) noexcept
{
    std::vector<std::shared_ptr<PasteDataRecord>> pasteDataRecords = pasteData->AllRecords();
    std::shared_ptr<std::string> htmlData;
    std::map<uint32_t, std::pair<std::string, std::string>, Cmp> replaceUris;

    for (auto& item : pasteDataRecords) {
        std::shared_ptr<std::string> html = item->GetHtmlText();
        if (html) {
            htmlData = html;
        }
        std::shared_ptr<OHOS::Uri> uri = item->GetUri();
        std::shared_ptr<MiscServices::MineCustomData> customData = item->GetCustomData();
        if (!uri || !customData) {
            continue;
        }
        std::map<std::string, std::vector<uint8_t>> customItemData = customData->GetItemData();
        for (auto& itemData : customItemData) {
            for (uint32_t i = 0; i < itemData.second.size(); i += FOUR_BYTES) {
                uint32_t offset = static_cast<uint32_t>(itemData.second[i]) |
                                  static_cast<uint32_t>(itemData.second[i + 1] << 8) |
                                  static_cast<uint32_t>(itemData.second[i + 2] << 16) |
                                  static_cast<uint32_t>(itemData.second[i + 3] << 24);
                replaceUris[offset] = std::make_pair(uri->ToString(), itemData.first);
            }
        }
    }

    RemoveAllRecord(pasteData);
    for (auto& replaceUri : replaceUris) {
        htmlData->replace(replaceUri.first, replaceUri.second.second.size(), replaceUri.second.first);
    }
    pasteData->AddHtmlRecord(*htmlData);
    return htmlData;
}

std::vector<std::pair<std::string, uint32_t>> PasteboardWebController::SplitHtmlWithImgLabel(
    const std::shared_ptr<std::string> html) noexcept
{
    std::smatch results;
    std::string pattern(IMG_TAG_PATTERN);
    std::regex r(pattern);
    std::string::const_iterator iterStart = html->begin();
    std::string::const_iterator iterEnd = html->end();
    std::vector<std::pair<std::string, uint32_t>> matchVec;

    while (std::regex_search(iterStart, iterEnd, results, r)) {
        std::string tmp = results[0];
        iterStart = results[0].second;
        uint32_t offset = static_cast<uint32_t>(results[0].first - html->begin());

        matchVec.emplace_back(std::make_pair(tmp, offset));
    }

    return matchVec;
}

std::map<std::string, std::vector<uint8_t>> PasteboardWebController::SplitHtmlWithImgSrcLabel(
    const std::vector<std::pair<std::string, uint32_t>>& matchVec) noexcept
{
    std::map<std::string, std::vector<uint8_t>> res;
    std::smatch match;
    std::regex re(IMG_TAG_SRC_PATTERN);
    for (auto& node : matchVec) {
        std::string::const_iterator iterStart = node.first.begin();
        std::string::const_iterator iterEnd = node.first.end();

        while (std::regex_search(iterStart, iterEnd, match, re)) {
            std::string tmp = match[0];
            iterStart = match[0].second;
            uint32_t offset = static_cast<uint32_t>(match[0].first - node.first.begin());
            tmp = tmp.substr(IMG_TAG_SRC_HEAD.size());
            tmp.pop_back();
            if (!IsLocalURI(tmp)) {
                continue;
            }
            offset += IMG_TAG_SRC_HEAD.size() + node.second;
            for (uint32_t i = 0; i < FOUR_BYTES; i++) {
                res[tmp].emplace_back((offset >> (EIGHT_BIT * i)) & 0xff);
            }
        }
    }
    return res;
}

std::shared_ptr<PasteData> PasteboardWebController::BuildPasteData(
    std::shared_ptr<std::string> html, const std::map<std::string, std::vector<uint8_t>>& imgSrcMap) noexcept
{
    std::shared_ptr<PasteData> pasteData = std::make_shared<PasteData>();
    pasteData->AddHtmlRecord(*html);
    for (auto& item : imgSrcMap) {
        PasteDataRecord::Builder builder(MiscServices::MIMETYPE_TEXT_URI);
        auto uri = std::make_shared<OHOS::Uri>(item.first);
        builder.SetUri(uri);
        auto customData = std::make_shared<MiscServices::MineCustomData>();

        customData->AddItemData(item.first, item.second);
        builder.SetCustomData(customData);
        auto record = builder.Build();
        pasteData->AddRecord(record);
    }
    return pasteData;
}

void PasteboardWebController::RemoveAllRecord(std::shared_ptr<PasteData> pasteData) noexcept
{
    std::size_t recordCount = pasteData->GetRecordCount();
    for (uint32_t i = 0; i < recordCount; i++) {
        if (!pasteData->RemoveRecordAt(0)) {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "WebClipboardController RemoveRecord failed, i=%{public}u", i);
        }
    }
}

bool PasteboardWebController::IsLocalURI(std::string& uri) noexcept
{
    return uri.substr(0, IMG_LOCAL_URI.size()) == IMG_LOCAL_URI || uri.find(IMG_LOCAL_PATH) == std::string::npos;
}
} // namespace MiscServices
} // namespace OHOS
