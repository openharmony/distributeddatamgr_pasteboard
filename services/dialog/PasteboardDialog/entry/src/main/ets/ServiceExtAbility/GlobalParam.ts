/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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