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
  let changeCount = Number(0);
  beforeAll(async function () {
    const systemPasteboard = pasteboard.getSystemPasteboard();
    changeCount = systemPasteboard.getChangeCount();
    console.info('beforeAll');
  });

  afterAll(async function () {
    console.info('afterAll');
  });

  /**
   * @tc.name      pasteboard_Sync_getChangeCount_test1
   * @tc.desc      changeCount should add 1 when process setPasteData
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_Sync_getChangeCount_test1', 0, async function (done) {
    const systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clearData();
    const textData = 'Hello World!';
    const pasteData = pasteboard.createPlainTextData(textData);
    await systemPasteboard.setPasteData(pasteData);
    let newCount = systemPasteboard.getChangeCount();
    let expectCount = changeCount + 1; 
    expect(newCount).assertEqual(expectCount);
    done();
  });

  /**
   * @tc.name      pasteboard_Sync_getChangeCount_test2
   * @tc.desc      changeCount should add 2 when process another setPasteData
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_Sync_getChangeCount_test2', 0, async function (done) {
    const systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clearData();
    const htmlText = '<html><head></head><body>Hello World!</body></html>';
    const pasteData = pasteboard.createHtmlData(htmlText);
    await systemPasteboard.setPasteData(pasteData);
    let newCount = systemPasteboard.getChangeCount();
    let expectCount = changeCount + 2;
    expect(newCount).assertEqual(expectCount);
    done();
  });

});