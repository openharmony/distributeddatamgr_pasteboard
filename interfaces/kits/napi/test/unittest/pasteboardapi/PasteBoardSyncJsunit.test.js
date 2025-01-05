/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
// @ts-nocheck
import { describe, beforeAll, afterAll, it, expect } from 'deccjsunit/index';
import pasteboard from '@ohos.pasteboard';

describe('PasteBoardJSTest', function () {
  beforeAll(async function () {
    console.info('beforeAll');
  });

  afterAll(async function () {
    console.info('afterAll');
  });

  /**
   * @tc.name      pasteboard_Sync_getChangeCounttest1
   * @tc.desc      changeCount should increase to 2 when setData and reset to 0 after clear pasteboard
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_Sync_getChangeCounttest1', 0, async function (done) {
    const systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clearData();
    let changeCount = systemPasteboard.getChangeCount();
    const textData = 'Hello World!';
    const pasteData = pasteboard.createPlainTextData(textData);
    await systemPasteboard.setPasteData(pasteData);
    let newCount = systemPasteboard.getChangeCount();
    expect(newCount).assertEqual(changeCount+1);
    const htmlText = '<html><head></head><body>Hello World!</body></html>';
    const pasteData1 = pasteboard.createHtmlData(htmlText);
    await systemPasteboard.setPasteData(pasteData1);
    newCount = systemPasteboard.getChangeCount();
    expect(newCount).assertEqual(changeCount+2);
    await systemPasteboard.clearData();
    newCount = systemPasteboard.getChangeCount();
    expect(newCount).assertEqual(changeCount+2);
  });

});