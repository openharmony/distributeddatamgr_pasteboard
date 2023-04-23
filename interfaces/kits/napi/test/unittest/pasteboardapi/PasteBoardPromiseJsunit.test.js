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
  it('pasteboard_promise_test1', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData = 'Hello World!';
    let pasteData = pasteboard.createPlainTextData(textData);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    let types = pasteData.getMimeTypes();
    expect('text/plain').assertEqual(types[0]);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(1);
      let primaryText = pasteData1.getPrimaryText();
      expect(primaryText).assertEqual(textData);
      expect(pasteboard.MAX_RECORD_NUM).assertEqual(512);
      expect(pasteData1.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test2
   * @tc.desc      Adds PlainTextData = ''
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test2', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData = '';
    let pasteData = pasteboard.createPlainTextData(textData);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let recordCount = data.getRecordCount();
      expect(recordCount).assertEqual(1);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test3
   * @tc.desc      Adds PlainTextData = 'Hello 中国!@#$%^&*()_+{}\?.'
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test3', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData = 'Hello 中国!@#$%^&*()_+{}?.';
    let pasteData = pasteboard.createPlainTextData(textData);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(1);
      let primaryText = pasteData1.getPrimaryText();
      expect(primaryText).assertEqual(textData);
      expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_PLAIN)).assertEqual(true);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test4
   * @tc.desc      Adds 300K PlainTextData
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test4', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData = '';
    for (let i = 0; i < 300; i++) {
      textData = textData + 'A';
    }
    const pasteData = pasteboard.createPlainTextData(textData);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      const pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(1);
      const primaryText = pasteData1.getPrimaryText();
      expect(primaryText).assertEqual(textData);
      expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_PLAIN)).assertEqual(true);
      expect(pasteData1.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test5
   * @tc.desc      Adds htmlText
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test5', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let htmlText = '<html><head></head><body>Hello World!</body></html>';
    let pasteData = pasteboard.createHtmlData(htmlText);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(1);
      let primaryHtml = pasteData1.getPrimaryHtml();
      expect(primaryHtml).assertEqual(htmlText);
      expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
      expect(pasteData1.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test6
   * @tc.desc      Adds htmlText = ''
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test6', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let htmlText = '';
    let pasteData = pasteboard.createHtmlData(htmlText);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      expect(data.getRecordCount()).assertEqual(1);
      let primaryHtml = data.getPrimaryHtml();
      expect(primaryHtml).assertEqual(htmlText);
      expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
      expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test7
   * @tc.desc      Adds htmlText = 'Hello 中国!@#$%^&*()_+{}\?.'
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test7', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let htmlText = 'Hello 中国!@#$%^&*()_+{}?.';
    let pasteData = pasteboard.createHtmlData(htmlText);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      expect(data.getRecordCount()).assertEqual(1);
      expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
      expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
      expect(data.getPrimaryHtml()).assertEqual(htmlText);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test8
   * @tc.desc      Adds uriText
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test8', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let uriText = 'https://www.baidu.com/';
      let pasteData = pasteboard.createUriData(uriText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let primaryUri = pasteData1.getPrimaryUri();
            expect(primaryUri).assertEqual(uriText);
            expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test9
   * @tc.desc      Adds uriText = ''
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test9', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let uriText = '';
    let pasteData = pasteboard.createUriData(uriText);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(1);
      expect(pasteData1.getPrimaryUri()).assertEqual(uriText);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test10
   * @tc.desc      Set uriText = 'Hello //'
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test10', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let uriText = 'Hello//';
    let pasteData = pasteboard.createUriData(uriText);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(1);
      expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
      expect(pasteData1.getPrimaryUri()).assertEqual(uriText);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test11
   * @tc.desc      Adds want
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test11', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let want = {
      bundleName: 'com.example.myapplication8',
      abilityName: 'com.example.myapplication8.MainAbility',
    };
    let pasteData = pasteboard.createWantData(want);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(1);
      let primaryWant = pasteData1.getPrimaryWant();
      expect(want.bundleName).assertEqual(primaryWant.bundleName);
      expect(want.abilityName).assertEqual(primaryWant.abilityName);
      expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_WANT)).assertEqual(true);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test12
   * @tc.desc      Adds one record(s)
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test12', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData = 'Hello World!';
    let pasteData = pasteboard.createPlainTextData(textData);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(1);
      let recordText = pasteData1.getRecordAt(0).plainText;
      expect(recordText).assertEqual(textData);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test13
   * @tc.desc      Adds 2 record(s)
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test13', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData0 = 'Hello World!';
    let pasteData = pasteboard.createPlainTextData(textData0);
    let textData1 = 'Hello World1';
    pasteData.addTextRecord(textData1);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      expect(data.getRecordCount()).assertEqual(2);
      let dataRecord0 = data.getRecordAt(0);
      let dataRecord1 = data.getRecordAt(1);
      expect(dataRecord0.plainText).assertEqual(textData1);
      expect(dataRecord1.plainText).assertEqual(textData0);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test14
   * @tc.desc      Adds 15 record(s)
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test14', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData0 = 'Hello World!';
    let pasteData = pasteboard.createPlainTextData(textData0);
    let textData = '';
    for (let i = 1; i < 15; i++) {
      textData = 'Hello World';
      textData = textData + i;
      pasteData.addTextRecord(textData);
    }
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(15);
      let dataRecord = pasteData1.getRecordAt(14);
      expect(dataRecord.plainText).assertEqual(textData0);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test15
   * @tc.desc      Adds 30 record(s)
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test15', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData0 = 'Hello World!';
    let pasteData = pasteboard.createPlainTextData(textData0);
    let textData = '';
    for (let i = 1; i < 30; i++) {
      textData = 'Hello World';
      textData = textData + i;
      pasteData.addTextRecord(textData);
    }
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(30);
      let dataRecord = pasteData1.getRecordAt(0);
      expect(dataRecord.plainText).assertEqual('Hello World29');
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test16
   * @tc.desc      Adds 31 record(s)
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test16', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData0 = 'Hello World!';
    let pasteData = pasteboard.createPlainTextData(textData0);
    let textData = '';
    for (let i = 1; i < 31; i++) {
      textData = 'Hello World';
      textData = textData + i;
      pasteData.addTextRecord(textData);
    }
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      let recordCount = pasteData1.getRecordCount();
      expect(recordCount).assertEqual(31);
      let dataRecord = pasteData1.getRecordAt(0);
      expect(dataRecord.plainText).assertEqual('Hello World30');
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test17
   * @tc.desc      Adds PlainText,HtmlText,UriText
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test17', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData = 'Hello World!';
    let pasteData = pasteboard.createPlainTextData(textData);
    let htmlText = '<html><head></head><body>Hello World!</body></html>';
    pasteData.addHtmlRecord(htmlText);
    let uriText = 'https://www.baidu.com/';
    pasteData.addUriRecord(uriText);
    let want = {
      bundleName: 'com.example.myapplication8',
      abilityName: 'com.example.myapplication8.MainAbility',
    };
    pasteData.addWantRecord(want);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(4);
      let wantRecord = pasteData1.getPrimaryWant();
      expect(wantRecord.bundleName).assertEqual(want.bundleName);
      expect(wantRecord.abilityName).assertEqual(want.abilityName);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_promise_test18
   * @tc.desc      Delete one PlainTextData
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test18', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    let textData = 'Hello World!';
    let pasteData = pasteboard.createPlainTextData(textData);
    await systemPasteboard.setPasteData(pasteData);
    const data = await systemPasteboard.hasPasteData();
    expect(data).assertEqual(true);
    systemPasteboard.getPasteData().then((data) => {
      let pasteData1 = data;
      expect(pasteData1.getRecordCount()).assertEqual(1);
      expect(pasteData1.removeRecordAt(0)).assertEqual(true);
      systemPasteboard.setPasteData(pasteData1).then(() => {
        systemPasteboard.getPasteData().then((data) => {
          let pasteData2 = data;
          let recordCount = pasteData2.getRecordCount();
          expect(recordCount).assertEqual(0);
          done();
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test19
   * @tc.desc      Delete one htmlText
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test19', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let htmlText = '<html><head></head><body>Hello World!</body></html>';
      let pasteData = pasteboard.createHtmlData(htmlText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            systemPasteboard.setPasteData(pasteData1).then(() => {
              systemPasteboard.getPasteData().then((data) => {
                let pasteData2 = data;
                let recordCount = pasteData2.getRecordCount();
                expect(recordCount).assertEqual(0);
                done();
              });
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test20
   * @tc.desc      Delete one uriText
   * @tc.type      Function
   * @tc.require   AR000H5HVI
   */
  it('pasteboard_promise_test20', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let uriText = 'https://www.baidu.com/';
      let pasteData = pasteboard.createUriData(uriText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            systemPasteboard.setPasteData(pasteData1).then(() => {
              systemPasteboard.getPasteData().then((data) => {
                let pasteData2 = data;
                let recordCount = pasteData2.getRecordCount();
                expect(recordCount).assertEqual(0);
                done();
              });
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test21
   * @tc.desc      Delete one want
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test21', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let want = {
        bundleName: 'com.example.myapplication8',
        abilityName: 'com.example.myapplication8.MainAbility',
      };
      let pasteData = pasteboard.createWantData(want);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            systemPasteboard.setPasteData(pasteData1).then(() => {
              systemPasteboard.getPasteData().then((data) => {
                let pasteData2 = data;
                let recordCount = pasteData2.getRecordCount();
                expect(recordCount).assertEqual(0);
                done();
              });
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test22
   * @tc.desc      Deletes 300K PlainTextData
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test22', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = '';
      for (let i = 0; i < 300; i++) {
        textData = textData + 'A';
      }
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            systemPasteboard.setPasteData(pasteData1).then(() => {
              systemPasteboard.getPasteData().then((data) => {
                let pasteData2 = data;
                let recordCount = pasteData2.getRecordCount();
                expect(recordCount).assertEqual(0);
                done();
              });
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test23
   * @tc.desc      Deletes 30 record(s)
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test23', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData0 = 'Hello World';
      let pasteData = pasteboard.createPlainTextData(textData0);
      let textData = '';
      for (let i = 1; i < 30; i++) {
        textData = 'Hello World';
        textData = textData + i;
        pasteData.addTextRecord(textData);
      }
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(30);
            for (let i = 0; i < 30; i++) {
              expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            }
            expect(pasteData1.getRecordCount()).assertEqual(0);
            systemPasteboard.setPasteData(pasteData1).then(() => {
              systemPasteboard.getPasteData().then((data) => {
                let pasteData2 = data;
                let recordCount = pasteData2.getRecordCount();
                expect(recordCount).assertEqual(0);
                done();
              });
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test24
   * @tc.desc      Deletes replaced record
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test24', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let textData1 = 'Hello World1';
            let pasteDataRecord = pasteboard.createPlainTextRecord(textData1);
            let replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
            expect(replace).assertEqual(true);
            let primaryText = pasteData1.getPrimaryText();
            expect(primaryText).assertEqual(textData1);
            expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_PLAIN)).assertEqual(true);
            let dataRecord = pasteData1.getRecordAt(0);
            expect(dataRecord.plainText).assertEqual(textData1);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            systemPasteboard.setPasteData(pasteData1).then(() => {
              systemPasteboard.getPasteData().then((data) => {
                let pasteData2 = data;
                let recordCount = pasteData2.getRecordCount();
                expect(recordCount).assertEqual(0);
                done();
              });
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test25
   * @tc.desc      Deletes 文本、uri、html
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test25', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      let htmlText = '<html><head></head><body>Hello World!</body></html>';
      pasteData.addHtmlRecord(htmlText);
      let uriText = 'https://www.baidu.com/';
      pasteData.addUriRecord(uriText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(3);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            systemPasteboard.setPasteData(pasteData1).then(() => {
              systemPasteboard.getPasteData().then((data) => {
                let pasteData2 = data;
                let recordCount = pasteData2.getRecordCount();
                expect(recordCount).assertEqual(0);
                done();
              });
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test26
   * @tc.desc      Replaces 文本 record
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test26', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let textData1 = 'Hello World1';
            let pasteDataRecord = pasteboard.createPlainTextRecord(textData1);
            let replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
            expect(replace).assertEqual(true);
            let primaryText = pasteData1.getPrimaryText();
            expect(primaryText).assertEqual(textData1);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test27
   * @tc.desc      Replaces htmlText record
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test27', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let htmlText = '<html><head></head><body>Hello World!</body></html>';
      let pasteData = pasteboard.createHtmlData(htmlText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let htmlText1 = '<html><head></head><body>Hello World 1</body></html>';
            let pasteDataRecord = pasteboard.createHtmlTextRecord(htmlText1);
            let replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
            expect(replace).assertEqual(true);
            let primaryHtml1 = pasteData1.getPrimaryHtml();
            expect(primaryHtml1).assertEqual(htmlText1);
            expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
            let primaryHtml2 = pasteData1.getPrimaryHtml();
            expect(primaryHtml2).assertEqual(htmlText1);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test28
   * @tc.desc      Replaces uri record
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test28', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let uriText = 'https://www.baidu.com/';
      let pasteData = pasteboard.createUriData(uriText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let uriText1 = 'https://www.baidu.com/1';
            let pasteDataRecord = pasteboard.createUriRecord(uriText1);
            let replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
            expect(replace).assertEqual(true);
            let primaryUri1 = pasteData1.getPrimaryUri();
            expect(primaryUri1).assertEqual(uriText1);
            expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
            let primaryUri2 = pasteData1.getPrimaryUri();
            expect(primaryUri2).assertEqual(uriText1);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test29
   * @tc.desc      Replaces want record
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test29', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let wantText0 = {
        bundleName: 'com.example.myapplication3',
        abilityName: 'com.example.myapplication3.MainAbility',
      };
      let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_WANT, wantText0);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let wantText1 = {
              bundleName: 'com.example.myapplication30',
              abilityName: 'com.example.myapplication30.MainAbility',
            };
            let pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_WANT, wantText1);
            let replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
            expect(replace).assertEqual(true);
            let primaryWant = pasteData1.getPrimaryWant();
            expect(primaryWant.bundleName).assertEqual(wantText1.bundleName);
            expect(primaryWant.abilityName).assertEqual(wantText1.abilityName);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test30
   * @tc.desc      Replaces 300k文本 record
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test30', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = '';
      for (let i = 0; i < 300; i++) {
        textData = textData + 'A';
      }
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let textData1 = 'Hello World1';
            let pasteDataRecord = pasteboard.createPlainTextRecord(textData1);
            let replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
            expect(replace).assertEqual(true);
            let primaryText = pasteData1.getPrimaryText();
            expect(primaryText).assertEqual(textData1);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test31
   * @tc.desc      Clears pasteBoard, gets record count
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test31', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            await systemPasteboard.clear();
            systemPasteboard.getPasteData().then((data) => {
              let pasteData2 = data;
              let recordCount = pasteData2.getRecordCount();
              expect(recordCount).assertEqual(0);
              done();
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test32
   * @tc.desc      Adds Property
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test32', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let pasteDataProperty = pasteData1.getProperty();
            expect(pasteDataProperty.shareOption).assertEqual(pasteboard.ShareOption.CrossDevice);
            pasteDataProperty.shareOption = pasteboard.ShareOption.InApp;
            pasteData1.setProperty(pasteDataProperty);
            expect(pasteData1.getProperty().shareOption).assertEqual(pasteboard.ShareOption.InApp);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test33
   * @tc.desc      Clears pasteBoard and check property
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test33', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            expect(data.getRecordCount()).assertEqual(1);
            await systemPasteboard.clear();
            let newPasteData = await systemPasteboard.getPasteData();
            expect(newPasteData.getProperty().shareOption).assertEqual(pasteboard.ShareOption.CrossDevice);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test34
   * @tc.desc      打开内容变化通知功能
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test34', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      systemPasteboard.on('update', contentChanges);
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            expect(pasteData1.getRecordCount()).assertEqual(0);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test35
   * @tc.desc      清除剪切板内的文本数据项
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test35', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            await systemPasteboard.clear();
            systemPasteboard.getPasteData().then((data) => {
              let pasteData2 = data;
              let recordCount = pasteData2.getRecordCount();
              expect(recordCount).assertEqual(0);
              done();
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test36
   * @tc.desc      清除剪切板内的uri数据项
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test36', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let uriText = 'https://www.baidu.com/';
      let pasteData = pasteboard.createUriData(uriText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            await systemPasteboard.clear();
            systemPasteboard.getPasteData().then((data) => {
              let pasteData2 = data;
              let recordCount = pasteData2.getRecordCount();
              expect(recordCount).assertEqual(0);
              done();
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test37
   * @tc.desc      清除剪切板内的html数据项
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test37', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let htmlText = '<html><head></head><body>Hello World!</body></html>';
      let pasteData = pasteboard.createHtmlData(htmlText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            await systemPasteboard.clear();
            systemPasteboard.getPasteData().then((data) => {
              let pasteData2 = data;
              let recordCount = pasteData2.getRecordCount();
              expect(recordCount).assertEqual(0);
              done();
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test38
   * @tc.desc      清除剪切板内的want数据项
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test38', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let myWant = {
        bundleName: 'com.example.myapplication55',
        abilityName: 'com.example.myapplication55.MainAbility',
      };
      let pasteData = pasteboard.createWantData(myWant);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            await systemPasteboard.clear();
            systemPasteboard.getPasteData().then((data) => {
              let pasteData2 = data;
              let recordCount = pasteData2.getRecordCount();
              expect(recordCount).assertEqual(0);
              done();
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test39
   * @tc.desc      向剪切板内增加30条数据项，然后清除
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test39', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData0 = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData0);
      let textData = '';
      for (let i = 1; i < 30; i++) {
        textData = 'Hello World';
        textData = textData + i;
        pasteData.addTextRecord(textData);
      }
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(30);
            await systemPasteboard.clear();
            systemPasteboard.getPasteData().then((data) => {
              let pasteData2 = data;
              let recordCount = pasteData2.getRecordCount();
              expect(recordCount).assertEqual(0);
              done();
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test40
   * @tc.desc      向剪贴板数据各增加5条文本、uri、html数据，然后清除
   * @tc.type      Function
   * @tc.require   AR000H5I1D
   */
  it('pasteboard_promise_test40', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData0 = 'Hello World0';
      let pasteData = pasteboard.createPlainTextData(textData0);
      let textData = '';
      for (let i = 1; i < 5; i++) {
        textData = 'Hello World';
        textData = textData + i;
        pasteData.addTextRecord(textData);
      }
      let htmlText = '';
      for (let i = 0; i < 5; i++) {
        htmlText = '<html><head></head><body>Hello World!</body></html>';
        htmlText = htmlText + i;
        pasteData.addHtmlRecord(htmlText);
      }
      let uriText = '';
      for (let i = 0; i < 5; i++) {
        uriText = 'https://www.baidu.com/';
        uriText = uriText + i;
        pasteData.addUriRecord(uriText);
      }
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(15);
            await systemPasteboard.clear();
            systemPasteboard.getPasteData().then((data) => {
              let pasteData2 = data;
              let recordCount = pasteData2.getRecordCount();
              expect(recordCount).assertEqual(0);
              done();
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test41
   * @tc.desc      更新剪贴板数据，查询剪贴板存在剪贴板数据
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test41', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let textData1 = 'Hello World1';
            let pasteDataRecord = pasteboard.createPlainTextRecord(textData1);
            let replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
            expect(replace).assertEqual(true);
            await systemPasteboard.setPasteData(pasteData1);
            systemPasteboard.hasPasteData().then(async (data) => {
              expect(data).assertEqual(true);
              let newData = await systemPasteboard.getPasteData();
              expect(newData.getPrimaryText()).assertEqual(textData1);
              let newPasteDataRecord = newData.getRecordAt(0);
              expect(newPasteDataRecord.plainText).assertEqual(textData1);
              done();
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test42
   * @tc.desc      删除所有的剪贴板数据，查询剪贴板不存在剪贴板数据
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test42', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            let recordCount = pasteData1.getRecordCount();
            expect(recordCount).assertEqual(1);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            expect(pasteData1.getRecordCount()).assertEqual(0);
            let newData = await systemPasteboard.getPasteData();
            expect(newData.getRecordCount()).assertEqual(1);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test43
   * @tc.desc      将文本数据强制转换为文本
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test43', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello World!';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then(async (data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let pasteDataRecord = pasteData1.getRecordAt(0);
            let text = await pasteDataRecord.convertToText();
            expect(text).assertEqual(textData);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test44
   * @tc.desc      将一条含有特殊字符、中英混杂的文本数据强制转换为文本
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test44', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = 'Hello 中国!@#$%^&*()_+{}?.';
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let pasteDataRecord = pasteData1.getRecordAt(0);
            pasteDataRecord.convertToText((err, text) => {
              if (err) {
                console.info('f_test44 pasteDataRecord.convertToText error: ' + error);
              } else {
                expect(textData).assertEqual(text);
                done();
              }
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test45
   * @tc.desc      将一条超长文本数据 (大小为301K)强制转换为文本
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test45', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData = '';
      for (let i = 0; i < 301; i++) {
        textData = textData + 'A';
      }
      let pasteData = pasteboard.createPlainTextData(textData);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let pasteDataRecord = pasteData1.getRecordAt(0);
            pasteDataRecord.convertToText((err, text) => {
              if (err) {
                console.info('f_test45 pasteDataRecord.convertToText error: ' + error);
              } else {
                expect(textData).assertEqual(text);
                done();
              }
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test46
   * @tc.desc      将uri数据强制转换为文本
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test46', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let uriText = 'https://www.baidu.com/';
      let pasteData = pasteboard.createUriData(uriText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            let pasteDataRecord = pasteData1.getRecordAt(0);
            pasteDataRecord
              .convertToText()
              .then((text) => {
                expect(uriText).assertEqual(text);
                done();
              })
              .catch((error) => {
                console.info('f_test46 pasteDataRecord.convertToText error: ' + error);
              });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test47
   * @tc.desc      复制文本、uri格式
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test47', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let textData0 = 'Hello World0';
      let pasteData = pasteboard.createPlainTextData(textData0);
      let uriText = pasteboard.createUriRecord('https://www.baidu.com/');
      pasteData.addRecord(uriText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((pasteData1) => {
            let recordCount = pasteData1.getRecordCount();
            expect(recordCount).assertEqual(2);
            let pasteDataRecord1 = pasteData1.getRecordAt(0);
            let pasteDataRecord2 = pasteData1.getRecordAt(1);
            expect(pasteDataRecord1.uri).assertEqual(uriText.uri);
            expect(pasteDataRecord2.plainText).assertEqual(textData0);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test48
   * @tc.desc      关闭内容变化通知功能：向剪贴板数据增加、删除等html数据项
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test48', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      systemPasteboard.off('update', contentChanges);
      let htmlText = '<html><head></head><body>Hello World!</body></html>';
      let pasteData = pasteboard.createHtmlData(htmlText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            let pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
            expect(pasteData1.getRecordCount()).assertEqual(0);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test49
   * @tc.desc      创建pixelMap
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test49', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let buffer = new ArrayBuffer(128);
      let opt = {
        size: { height: 5, width: 5 },
        pixelFormat: 3,
        editable: true,
        alphaType: 1,
        scaleMode: 1,
      };
      image.createPixelMap(buffer, opt).then(async (pixelMap) => {
        expect(pixelMap.getPixelBytesNumber()).assertEqual(100);
        let pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        systemPasteboard.setPasteData(pasteData).then(() => {
          systemPasteboard.hasPasteData().then((data) => {
            expect(data).assertEqual(true);
            systemPasteboard.getPasteData().then((newPasteData) => {
              let recordCount = newPasteData.getRecordCount();
              expect(recordCount).assertEqual(1);
              let newPixelMap = newPasteData.getPrimaryPixelMap();
              let PixelMapBytesNumber = newPixelMap.getPixelBytesNumber();
              expect(PixelMapBytesNumber).assertEqual(100);
              newPixelMap.getImageInfo().then((imageInfo) => {
                expect(imageInfo.size.height === 5 && imageInfo.size.width === 5).assertEqual(true);
                done();
              });
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test50
   * @tc.desc      创建kv Record
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test50', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let dataXml = new ArrayBuffer(512);
      let int32view = new Int32Array(dataXml);
      for (let i = 0; i < int32view.length; i++) {
        int32view[i] = 65535 + i;
      }
      let pasteDataRecord = pasteboard.createRecord('app/xml', dataXml);
      let dataJpg = new ArrayBuffer(256);
      pasteDataRecord.data['image/ipg'] = dataJpg;
      let pasteData = pasteboard.createHtmlData('application/xml');
      let replace = pasteData.replaceRecordAt(0, pasteDataRecord);
      expect(replace).assertEqual(true);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((newPasteData) => {
            let recordCount = newPasteData.getRecordCount();
            expect(recordCount).assertEqual(1);
            let newPasteDataRecord = newPasteData.getRecordAt(0);
            let newAppXml = newPasteDataRecord.data['app/xml'];
            let newImageIpg = newPasteDataRecord.data['image/ipg'];
            expect(newAppXml.byteLength === 512 && newImageIpg.byteLength === 256).assertEqual(true);
            let newAppXmlView = new Int32Array(newAppXml);
            let newImageIpgView = new Int32Array(newImageIpg);
            for (let i = 0; i < newAppXmlView.length; i++) {
              console.info('newAppXml[' + i + '] = ' + newAppXmlView[i]);
            }
            for (let i = 0; i < newImageIpgView.length; i++) {
              console.info('newImageIpg[' + i + '] = ' + newImageIpg[i]);
            }
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test51
   * @tc.desc      测试addPixelMapRecord
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_promise_test51', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      let buffer = new ArrayBuffer(128);
      let opt = {
        size: { height: 5, width: 5 },
        pixelFormat: 3,
        editable: true,
        alphaType: 1,
        scaleMode: 1,
      };
      let pasteData = pasteboard.createHtmlData('application/xml');
      image.createPixelMap(buffer, opt).then((pixelMap) => {
        expect(pixelMap.getPixelBytesNumber() === 100).assertEqual(true);
        pasteData.addRecord(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        systemPasteboard.setPasteData(pasteData).then(() => {
          systemPasteboard.hasPasteData().then((data) => {
            expect(data).assertEqual(true);
            systemPasteboard.getPasteData().then((newPasteData) => {
              let recordCount = newPasteData.getRecordCount();
              expect(recordCount).assertEqual(2);
              let newPixelMap = newPasteData.getPrimaryPixelMap();
              let PixelMapBytesNumber = newPixelMap.getPixelBytesNumber();
              expect(PixelMapBytesNumber).assertEqual(100);
              let newHtmlData = newPasteData.getRecordAt(1);
              expect(newHtmlData.htmlText).assertEqual('application/xml');
              newPixelMap.getImageInfo().then((imageInfo) => {
                expect(imageInfo.size.height === 5 && imageInfo.size.width === 5).assertEqual(true);
                done();
              });
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_promise_test52
   * @tc.desc      支持512个record
   * @tc.type      Function
   * @tc.require   AR000HEECB
   */
  it('pasteboard_promise_test52', 0, async function (done) {
    let systemPasteBoard = pasteboard.getSystemPasteboard();
    systemPasteBoard.clear().then(() => {
      let dataHtml = new ArrayBuffer(256);
      let htmlText = '<html><head></head><body>Hello!</body></html>';
      let uriText = 'https://www.baidu.com/';
      let wantText = {
        bundleName: 'com.example.myapplication3',
        abilityName: 'com.example.myapplication3.MainAbility',
      };
      let plainText = '';
      let pasteData = pasteboard.createData('x'.repeat(1024), dataHtml);
      let record = pasteData.getRecordAt(0);
      record.htmlText = htmlText;
      record.plainText = plainText;
      record.uri = uriText;
      record.want = wantText;
      let buffer = new ArrayBuffer(128);
      let opt = {
        size: { height: 5, width: 5 },
        pixelFormat: 3,
        editable: true,
        alphaType: 1,
        scaleMode: 1,
      };
      image.createPixelMap(buffer, opt).then((pixelMap) => {
        record.pixelMap = pixelMap;
        pasteData.replaceRecordAt(0, record);
        for (let i = 0; i < 511; i++) {
          pasteData.addRecord(record);
        }
        systemPasteBoard.setPasteData(pasteData).then(() => {
          systemPasteBoard.hasPasteData().then((hasData) => {
            expect(hasData).assertTrue();
            systemPasteBoard.getPasteData().then((data) => {
              expect(data.getRecordCount()).assertEqual(512);
              expect(data.getRecordAt(0).mimeType).assertEqual('x'.repeat(1024));
              expect(data.getPrimaryWant().bundleName).assertEqual(wantText.bundleName);
              expect(data.getRecordAt(253).htmlText).assertEqual(htmlText);
              expect(data.getRecordAt(511).plainText).assertEqual(plainText);
              done();
            });
          });
        });
      });
    });
  });

  /**
   *  The callback function is used for pasteboard content changes
   */
  function contentChanges() {
    console.info('#EVENT: The content is changed in the pasteboard');
  }
});
