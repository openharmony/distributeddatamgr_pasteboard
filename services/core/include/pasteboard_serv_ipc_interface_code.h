/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PASTE_BOARD_PASTEBOARD_SERV_IPC_INTERFACE_CODE_H
#define PASTE_BOARD_PASTEBOARD_SERV_IPC_INTERFACE_CODE_H

/* SAID: 3701 */
namespace OHOS {
namespace Security {
namespace PasteboardServ {
enum PasteboardServiceInterfaceCode {
    GET_PASTE_DATA = 0,
    HAS_PASTE_DATA = 1,
    SET_PASTE_DATA = 2,
    CLEAR_ALL = 3,
    ADD_CHANGED_OBSERVER = 4,
    DELETE_CHANGED_OBSERVER = 5,
    DELETE_ALL_CHANGED_OBSERVER = 6,
    ADD_EVENT_OBSERVER = 7,
    DELETE_EVENT_OBSERVER = 8,
    DELETE_ALL_EVENT_OBSERVER = 9,
    IS_REMOTE_DATA = 10,
    GET_DATA_SOURCE = 11,
    HAS_DATA_TYPE = 12,
    SET_GLOBAL_SHARE_OPTION = 13,
    REMOVE_GLOBAL_SHARE_OPTION = 14,
    GET_GLOBAL_SHARE_OPTION = 15,
    SET_APP_SHARE_OPTIONS = 16,
    REMOVE_APP_SHARE_OPTIONS = 17,
};

enum PasteboardObserverInterfaceCode {
    ON_PASTE_BOARD_CHANGE = 0,
    ON_PASTE_BOARD_EVENT = 1,
};
} // namespace PasteboardServ
} // namespace Security
} // namespace OHOS
#endif // PASTE_BOARD_PASTEBOARD_SERV_IPC_INTERFACE_CODE_H
