/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility';
import common from '@ohos.app.ability.common';
import window from '@ohos.window';
import Want from '@ohos.app.ability.Want';
import display from '@ohos.display';
import GlobalContext from './GlobalContext';
import { BusinessError } from '@ohos.base';

const TAG = 'PasteboardProgressAbility: ';
const BG_COLOR = '#00000000';
let createWindowStatus = 'created';

interface IRect {
  left: number;
  top: number;
  width: number;
  height: number;
}

interface StorageItem {
  want: Want,
  win: window.Window,
  serviceContext: common.ServiceExtensionContext,
  globalContext: GlobalContext
}

export default class PasteboardProgressAbility extends ServiceExtensionAbility {
  onCreate(want: Want): void {
    console.info(TAG + 'PasteboardProgressAbility onCreate, ability name is ' + want.abilityName);
  }

  async onRequest(want: Want, startId: number) {
    console.info(TAG + 'PasteboardProgressAbility onRequest, startId is' + startId);
    console.info(TAG + 'want: ' + JSON.stringify(want));
    try {
      let displayClass = display.getDefaultDisplaySync();
      let navigationBarRect: IRect = {
        left: 0,
        top: 0,
        width: displayClass.width,
        height: displayClass.height,
      };
      if (canIUse('SystemCapability.Window.SessionManager')) {
        await window.getVisibleWindowInfo().then((infos: window.WindowInfo[]) => {
          const targetWindowId = want.parameters?.windowId as number;
          const targetWindowInfo = infos.find(info => info.windowId === targetWindowId);
          navigationBarRect = targetWindowInfo?.rect as IRect;
          console.info(TAG + `targetWindowInfo rect is ${JSON.stringify(navigationBarRect)}`);
        }).catch((err: BusinessError) => {
          console.error(TAG + 'Failed to getWindowInfo. Cause: ' + JSON.stringify(err));
        });
      }
      this.createWindow(`PasteboardProgressDialog${startId}`, navigationBarRect, want);
    } catch (exception) {
      console.error(TAG + 'Failed to obtain the default display object. Code:' + JSON.stringify(exception))
    }
  }

  onDestroy(): void {
    console.info(TAG + 'PasteboardProgressAbility onDestroy.')
  }

  private async createWindow(name: string, rect: IRect, want: Want): Promise<void> {
    if (createWindowStatus === 'created') {
      createWindowStatus = 'creating';
      console.info(TAG + 'create progressWindow');
    } else {
      console.warn(TAG + 'window is creating');
      return;
    }
    const progressKey = want.parameters?.['progressKey'] as string;
    if (GlobalContext.getInstance().dialogSet.has(progressKey)) {
      console.info(TAG + 'window already exists');
      return;
    }
    try {
      const win = await window.createWindow({
        ctx: this.context,
        name,
        windowType: window.WindowType.TYPE_DIALOG
      });
      await win.bindDialogTarget((want.parameters?.['tokenKey'] as ESObject).value, () => {
        win.destroyWindow();
        console.info(TAG + 'destroyWindow');
        GlobalContext.getInstance().dialogSet.delete(progressKey);
        if (GlobalContext.getInstance().dialogSet.size === 0) {
          this.context.terminateSelf();
        }
      });
      let storageItem: StorageItem = {
        want,
        win,
        globalContext: GlobalContext.getInstance(),
        serviceContext: this.context
      }
      let storage: LocalStorage = new LocalStorage(storageItem);
      await win.moveWindowTo(rect.left, rect.top);
      await win.resize(rect.width, rect.height);
      await win.loadContent('pages/PasteboardProgress', storage);
      win.setWindowBackgroundColor(BG_COLOR);
      await win.showWindow();
      GlobalContext.getInstance().dialogSet.add(progressKey);
      console.info(TAG + `dialogSet add, size is ${GlobalContext.getInstance().dialogSet.size}`);
    } catch (err) {
      console.error(TAG + `window create failed! err: ${JSON.stringify(err)}`);
    } finally {
      createWindowStatus = 'created';
    }
  }
}