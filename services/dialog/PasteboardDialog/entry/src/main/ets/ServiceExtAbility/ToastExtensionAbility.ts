/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
import GlobalContext from './GlobalParam';
import { GlobalExtensionWindow } from './GlobalParam';

interface IRect {
  left: number;
  top: number;
  width: number;
  height: number;
}

const DISTANCE_NUMBER = 68;
const TAG = 'ToastExtensionAbility';

class ToastStub extends rpc.RemoteObject {
  constructor(des: string) {
    super(des);
  }
}

export class ToastInfo {
  public fromAppName: string = '';
  public toAppName: string = '';
  public displayHeight: number = 0;
  private static toastInfo: ToastInfo;

  public static getInstance(): ToastInfo {
    if (ToastInfo.toastInfo == null) {
      ToastInfo.toastInfo = new ToastInfo();
    }

    return ToastInfo.toastInfo;
  }
}

export default class ToastExtensionAbility extends ServiceExtensionAbility {
  onCreate(want: Want): void {
    hilog.info(0, TAG, 'onCreate');
    GlobalContext.getInstance().context = this.context;
  }

  onConnect(want: Want): ToastStub {
    hilog.info(0, TAG, 'onConnect');
    display
      .getDefaultDisplay()
      .then((display: display.Display) => {
        const toastRect = {
          left: 0,
          top: 0,
          width: display.width,
          height: display.height,
        };
        ToastInfo.getInstance().fromAppName = want.parameters.fromAppName;
        ToastInfo.getInstance().toAppName = want.parameters.toAppName;
        ToastInfo.getInstance().displayHeight = display.height / display.densityPixels - DISTANCE_NUMBER;
        this.createToastWindow('PasteboardToast' + new Date().getTime(), toastRect);
      })
      .catch((err) => {
        hilog.info(0, TAG, 'getDefaultDisplay err: ' + JSON.stringify(err));
      });
    return new ToastStub('PasteboardToast');
  }

  onRequest(want: Want, startId: number): void {
    hilog.info(0, TAG, 'onRequest');
    this.onConnect(want);
  }

  onDisconnet(): void {
    hilog.info(0, TAG, 'onDisconnet');
    this.onDestroy();
  }

  onDestroy(): void {
    hilog.info(0, TAG, 'onDestroy');
    GlobalExtensionWindow.getInstance().extensionWin.destroyWindow();
    GlobalContext.getInstance().context.terminateSelf();
  }

  private async createToastWindow(name: string, rect: IRect): Promise<void> {
    hilog.info(0, TAG, 'create toast begin');

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
        GlobalExtensionWindow.getInstance().extensionWin = data;
        hilog.info(0, TAG, 'Succeeded in creating the window. Data: ' + JSON.stringify(data));
        try {
          windowClass.setUIContent('pages/toastIndex', (err) => {
            if (err.code) {
              hilog.error(0, TAG, 'Failed to load the content. Cause:' + JSON.stringify(err));
              return;
            }
            windowClass.moveWindowTo(rect.left, rect.top);
            windowClass.resize(rect.width, rect.height);
            windowClass.setBackgroundColor('#00000000');
            windowClass.setWindowTouchable(false);
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
