/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
import Want from '@ohos.application.Want';
import hilog from '@ohos.hilog';
import window from '@ohos.window'
import display from '@ohos.display';
import rpc from '@ohos.rpc'
import ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility'

const TAG = 'DialogExtensionAbility'

class DialogStub extends rpc.RemoteObject {
    constructor(des) {
        super(des);
    }

    onConnect(code, data, reply, option) {
    }
}

export default class DialogExtensionAbility extends ServiceExtensionAbility {
    onCreate(want: Want) {
        hilog.info(0, TAG, "onCreate")
        globalThis.context = this.context
    }

    onConnect(want: Want) {
        hilog.info(0, TAG, "onConnect")
        globalThis.dialogInfo = {
            appName: want.parameters['appName'],
            deviceType: want.parameters['deviceType']
        }
        display.getDefaultDisplay().then((display: display.Display) => {
            let dialogRect = {
                left: 0,
                top: 72,
                width: display.width,
                height: display.height - 72,
            }
            this.createWindow("PasteboardDialog" + new Date().getTime(), window.WindowType.TYPE_DIALOG, dialogRect)
        }).catch((err) => {
            hilog.info(0, TAG, "getDefaultDisplay err: " + JSON.stringify(err));
        })
        return new DialogStub('PasteboardDialog')
    }

    onReconnect(want: Want) {
        hilog.info(0, TAG, "onReconnect")
        this.onConnect(want)
    }

    onDestroy() {
        hilog.info(0, TAG, "onDestroy")
        globalThis.extensionWin.destroy()
        globalThis.context.terminateSelf()
    }

    private async createWindow(name: string, windowType: number, rect: any) {
        hilog.info(0, TAG, "create window begin")
        try {
            if (globalThis.windowNum > 0) {
                globalThis.windowNum = 0
                globalThis.extensionWin.destroy()
                this.onDestroy()
            }
            const win = await window.create(this.context, name, windowType)
            globalThis.extensionWin = win
            await win.moveTo(rect.left, rect.top)
            await win.resetSize(rect.width, rect.height)
            await win.loadContent('pages/index')
            await win.setBackgroundColor('#00000000')
            await win.show()
            globalThis.windowNum++
            hilog.info(0, TAG, "create window successfully")
        } catch {
            hilog.info(0, TAG, "create window failed")
        }
    }
}
