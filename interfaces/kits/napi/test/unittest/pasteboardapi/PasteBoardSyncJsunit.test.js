/*
 * Copyright (C) 2022-2023 Huawei Device Co., Ltd.
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
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index';
import pasteboard from '@ohos.pasteboard';
import image from '@ohos.multimedia.image';
import Want from '@ohos.app.ability.Want';
import List from '@ohos.util.List';

const myType = 'my-mime-type';
const allTypes = [pasteboard.MIMETYPE_TEXT_PLAIN, pasteboard.MIMETYPE_TEXT_HTML, pasteboard.MIMETYPE_TEXT_URI,
  pasteboard.MIMETYPE_TEXT_WANT, pasteboard.MIMETYPE_PIXELMAP, myType
];

describe('PasteBoardJSTest', function () {
  beforeAll(async function () {
    console.info('beforeAll');
  });

  afterAll(async function () {
    console.info('afterAll');
  });

  /**
   * @tc.name      pasteboard_promise_test1
   * @tc.desc      Adds PlainTextData
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_Sync_getChangeCounttest1', 0, async function (done) {
    const systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clearData();
    let changeCount = systemPasteboard.getChangeCount();
    expect(changeCount).assertEqual(0);
    const textData1 = 'Hello World!';
    const pasteData = pasteboard.createPlainTextData(textData1);
    await systemPasteboard.setPasteData(pasteData);
    changeCount = systemPasteboard.getChangeCount();
    expect(changeCount).assertEqual(1);
    await systemPasteboard.clearData();
    changeCount = systemPasteboard.getChangeCount();
    expect(changeCount).assertEqual(0);
  });

});