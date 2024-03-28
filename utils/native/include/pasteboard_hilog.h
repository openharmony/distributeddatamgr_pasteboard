/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef PASTEBOARD_HILOG_H
#define PASTEBOARD_HILOG_H

#include "hilog/log.h"

namespace OHOS {
namespace MiscServices {
// param of log interface, such as PASTEBOARD_HILOGF.
enum PasteboardSubModule {
    PASTEBOARD_MODULE_INNERKIT = 0,
    PASTEBOARD_MODULE_CLIENT,
    PASTEBOARD_MODULE_SERVICE,
    PASTEBOARD_MODULE_JAVAKIT, // java kit, defined to avoid repeated use of domain.
    PASTEBOARD_MODULE_JNI,
    PASTEBOARD_MODULE_COMMON,
    PASTEBOARD_MODULE_JS_NAPI,
    PASTEBOARD_MODULE_BUTT,
};

// 0xD001C00: subsystem:PASTEBOARD module:PasteboardManager, 8 bits reserved.
static constexpr unsigned int BASE_PASTEBOARD_DOMAIN_ID = 0xD001C00;

enum PasteboardDomainId {
    PASTEBOARD_INNERKIT_DOMAIN = BASE_PASTEBOARD_DOMAIN_ID + PASTEBOARD_MODULE_INNERKIT,
    PASTEBOARD_CLIENT_DOMAIN,
    PASTEBOARD_SERVICE_DOMAIN,
    PASTEBOARD_JAVAKIT_DOMAIN,
    PASTEBOARD_JNI_DOMAIN,
    PASTEBOARD_COMMON_DOMAIN,
    PASTEBOARD_JS_NAPI,
    PASTEBOARD_BUTT,
};

static constexpr OHOS::HiviewDFX::HiLogLabel PASTEBOARD[PASTEBOARD_MODULE_BUTT] = {
    { LOG_CORE, PASTEBOARD_INNERKIT_DOMAIN, "PasteboardInnerKit" },
    { LOG_CORE, PASTEBOARD_CLIENT_DOMAIN, "PasteboardClient" },
    { LOG_CORE, PASTEBOARD_SERVICE_DOMAIN, "PasteboardService" },
    { LOG_CORE, PASTEBOARD_JAVAKIT_DOMAIN, "PasteboardJavaKit" },
    { LOG_CORE, PASTEBOARD_JNI_DOMAIN, "PasteboardJni" },
    { LOG_CORE, PASTEBOARD_COMMON_DOMAIN, "PasteboardCommon" },
    { LOG_CORE, PASTEBOARD_JS_NAPI, "PasteboardJSNAPI" },
};

#define FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

// In order to improve performance, do not check the module range.
// Besides, make sure module is less than PASTEBOARD_MODULE_BUTT.
#define PASTEBOARD_HILOGF(module, ...)                                                                    \
    do {                                                                                                  \
        if (HiLogIsLoggable(PASTEBOARD[module].domain, PASTEBOARD[module].tag, LOG_FATAL)) {              \
            ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, PASTEBOARD[module].domain, PASTEBOARD[module].tag,     \
                              "[%{public}s] %{public}s# " fmt, FILENAME__, __FUNCTION__, ##__VA_ARGS__)); \
        }                                                                                                 \
    } while (0)
#define PASTEBOARD_HILOGE(module, fmt, ...)                                                               \
    do {                                                                                                  \
        if (HiLogIsLoggable(PASTEBOARD[module].domain, PASTEBOARD[module].tag, LOG_ERROR)) {              \
            ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, PASTEBOARD[module].domain, PASTEBOARD[module].tag,     \
                              "[%{public}s] %{public}s# " fmt, FILENAME__, __FUNCTION__, ##__VA_ARGS__)); \
        }                                                                                                 \
    } while (0)
#define PASTEBOARD_HILOGW(module, fmt, ...)                                                               \
    do {                                                                                                  \
        if (HiLogIsLoggable(PASTEBOARD[module].domain, PASTEBOARD[module].tag, LOG_WARN)) {               \
            ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, PASTEBOARD[module].domain, PASTEBOARD[module].tag,      \
                              "[%{public}s] %{public}s# " fmt, FILENAME__, __FUNCTION__, ##__VA_ARGS__)); \
        }                                                                                                 \
    } while (0)
#define PASTEBOARD_HILOGI(module, fmt, ...)                                                               \
    do {                                                                                                  \
        if (HiLogIsLoggable(PASTEBOARD[module].domain, PASTEBOARD[module].tag, LOG_INFO)) {               \
            ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, PASTEBOARD[module].domain, PASTEBOARD[module].tag,      \
                              "[%{public}s] %{public}s# " fmt, FILENAME__, __FUNCTION__, ##__VA_ARGS__)); \
        }                                                                                                 \
    } while (0)
#define PASTEBOARD_HILOGD(module, fmt, ...)                                                               \
    do {                                                                                                  \
        if (HiLogIsLoggable(PASTEBOARD[module].domain, PASTEBOARD[module].tag, LOG_DEBUG)) {              \
            ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, PASTEBOARD[module].domain, PASTEBOARD[module].tag,     \
                              "[%{public}s] %{public}s# " fmt, FILENAME__, __FUNCTION__, ##__VA_ARGS__)); \
        }                                                                                                 \
    } while (0)
} // namespace MiscServices
} // namespace OHOS

#endif // PASTEBOARD_HILOG_H
