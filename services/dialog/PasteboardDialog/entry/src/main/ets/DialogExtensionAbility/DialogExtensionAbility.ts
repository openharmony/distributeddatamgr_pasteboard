/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import rpc from '@ohos.rpc';
import hilog from '@ohos.hilog';
import window from '@ohos.window';
import display from '@ohos.display';
import ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility';
import type Want from '@ohos.application.Want';

interface IRect {
  left: number;
  top: number;
  width: number;
  height: number;
}

const RECT_NUMBER = 72;
const TAG = 'DialogExtensionAbility';

class DialogStub extends rpc.RemoteObject {
  constructor(des: string) {
    super(des);
  }
  onConnect(code, data, reply, option): void {}
}

export default class DialogExtensionAbility extends ServiceExtensionAbility {
  onCreate(want: Want): void {
    hilog.info(0, TAG, 'onCreate');
    globalThis.context = this.context;
  }

  onConnect(want: Want): DialogStub {
    hilog.info(0, TAG, 'onConnect');
    globalThis.dialogInfo = {
      appName: want.parameters.appName,
      deviceType: want.parameters.deviceType,
    };
    display
      .getDefaultDisplay()
      .then((display: display.Display) => {
        const dialogRect = {
          left: 0,
          top: 72,
          width: display.width,
          height: display.height - RECT_NUMBER,
        };
        this.createFloatWindow('PasteboardDialog' + new Date().getTime(), dialogRect);
      })
      .catch((err) => {
        hilog.info(0, TAG, 'getDefaultDisplay err: ' + JSON.stringify(err));
      });
    return new DialogStub('PasteboardDialog');
  }

  onReconnect(want: Want): void {
    hilog.info(0, TAG, 'onReconnect');
    this.onConnect(want);
  }

  onDestroy(): void {
    hilog.info(0, TAG, 'onDestroy');
    globalThis.extensionWin.destroyWindow();
    globalThis.context.terminateSelf();
  }

  private async createFloatWindow(name: string, rect: IRect): Promise<void> {
    hilog.info(0, TAG, 'create window begin');

    if (globalThis.windowNum > 0) {
      globalThis.windowNum = 0;
      this.onDestroy();
    }
    let windowClass = null;
    let config = {
      name,
      windowType: window.WindowType.TYPE_FLOAT,
      ctx: this.context,
    };
    try {
      window.createWindow(config, (err, data) => {
        if (err.code) {
          hilog.error(0, TAG, 'Failed to create the window. Cause: ' + JSON.stringify(err));
          return;
        }
        windowClass = data;
        globalThis.extensionWin = data;
        hilog.info(0, TAG, 'Succeeded in creating the window. Data: ' + JSON.stringify(data));
        try {
          windowClass.setUIContent('pages/index', (err) => {
            if (err.code) {
              hilog.error(0, TAG, 'Failed to load the content. Cause:' + JSON.stringify(err));
              return;
            }
            windowClass.moveWindowTo(rect.left, rect.top);
            windowClass.resize(rect.width, rect.height);
            windowClass.loadContent('pages/index');
            windowClass.setBackgroundColor('#00000000');
            windowClass.showWindow();
            globalThis.windowNum++;
            hilog.info(0, TAG, 'Create window successfully');
          });
        } catch (exception) {
          hilog.error(0, TAG, 'Failed to load the content. Cause:' + JSON.stringify(exception));
        }
      });
    } catch (exception) {
      hilog.error(0, TAG, 'Failed to create the window. Cause: ' + JSON.stringify(exception));
    }
  }
}
