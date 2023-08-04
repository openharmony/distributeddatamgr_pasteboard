/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef PASTE_DATA_TEST_CLIP_FACTORY_H
#define PASTE_DATA_TEST_CLIP_FACTORY_H
#include "clip/clip_plugin.h"
namespace OHOS::MiscServices {
class ClipFactory : public ClipPlugin::Factory {
public:
    ClipFactory();
    ~ClipFactory();
    ClipPlugin *Create() override;
    bool Destroy(ClipPlugin *plugin) override;

private:
    ClipPlugin *clip_ = nullptr;
    static ClipFactory factory_;
};

ClipPlugin *ClipFactory::Create()
{
    return clip_;
}

bool ClipFactory::Destroy(ClipPlugin *plugin)
{
    if (plugin == clip_) {
        delete clip_;
        clip_ = nullptr;
        return true;
    }
    return false;
}
}
#endif // PASTE_DATA_TEST_CLIP_FACTORY_H