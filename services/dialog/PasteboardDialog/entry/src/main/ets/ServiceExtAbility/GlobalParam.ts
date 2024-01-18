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

import type window from '@ohos.window';
import type ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility';

type ServiceExtensionContext = ServiceExtensionAbility['context'];

export default class GlobalContext {
  public context: ServiceExtensionContext;
  private static globalContext: GlobalContext;

  public static getInstance(): GlobalContext {
    if (GlobalContext.globalContext == null) {
      GlobalContext.globalContext = new GlobalContext();
    }

    return GlobalContext.globalContext;
  }
}

export class GlobalExtensionWindow {
  public extensionWin: window.Window;
  private static globalExtensionWindow: GlobalExtensionWindow;

  public static getInstance(): GlobalExtensionWindow {
    if (GlobalExtensionWindow.globalExtensionWindow == null) {
      GlobalExtensionWindow.globalExtensionWindow = new GlobalExtensionWindow();
    }

    return GlobalExtensionWindow.globalExtensionWindow;
  }
}