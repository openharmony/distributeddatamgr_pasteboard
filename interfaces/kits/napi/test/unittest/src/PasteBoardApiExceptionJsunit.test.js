/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
   * @tc.name      pasteboard_function_test1
   * @tc.desc      自定义数据测试
   * @tc.type      Function
   * @tc.require   AR000HEECB
   */
  it('pasteboard_function_test1', 0, async function (done) {
    var systemPasteBoard = pasteboard.getSystemPasteboard();
    await systemPasteBoard.clear().then(async () => {
      var pasteData = undefined;
      console.info('systemPasteBoard clear data success');
      var dataUri = new ArrayBuffer(256);
      pasteData = pasteboard.createData('xxx', dataUri);
      var addUri = new ArrayBuffer(128);
      pasteData.addRecord('xxx', addUri);
      var recordUri = new ArrayBuffer(96);
      var pasteDataRecord = pasteboard.createRecord('xxx', recordUri);
      pasteData.addRecord(pasteDataRecord);
      await systemPasteBoard.setPasteData(pasteData).then(async () => {
        console.info('Set pastedata success');
        await systemPasteBoard.hasPasteData().then(async (data) => {
          console.info('Check pastedata has data success, result: ' + data);
          expect(data).assertTrue();
          await systemPasteBoard.getPasteData().then(async (data) => {
            console.info('Get paste data success');
            expect(data.getRecordCount()).assertEqual(3);
            expect(data.getRecordAt(0).data['xxx'].byteLength).assertEqual(96);
            expect(data.getRecordAt(1).data['xxx'].byteLength).assertEqual(128);
            expect(data.getRecordAt(2).data['xxx'].byteLength).assertEqual(256);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test2
   * @tc.desc      自定义数据测试
   * @tc.type      Function
   * @tc.require   AR000HEECB
   */
  it('pasteboard_function_test2', 0, async function (done) {
    var systemPasteBoard = pasteboard.getSystemPasteboard();
    await systemPasteBoard.clear().then(async () => {
      console.info('systemPasteBoard clear data success');
      var pasteData = undefined;
      var pasteRecord = undefined;

      var dataHtml = new ArrayBuffer(256);
      pasteData = pasteboard.createData('xy', dataHtml);
      expect(pasteData != undefined).assertTrue();

      pasteData.addRecord('x'.repeat(1024), dataHtml);
      expect(pasteData.getRecordCount()).assertEqual(2);

      pasteRecord = pasteboard.createRecord('xy2', dataHtml);
      expect(pasteRecord != undefined).assertTrue();
      pasteData.addRecord(pasteRecord);

      await systemPasteBoard.setPasteData(pasteData).then(async () => {
        console.info('set pastedata success');
        await systemPasteBoard.hasPasteData().then(async (data) => {
          console.info('Check pastedata has data success, result: ' + data);
          expect(data).assertTrue();
          await systemPasteBoard.getPasteData().then(async (data) => {
            console.info('get paste data success');
            expect(data.getRecordCount()).assertEqual(3);
            expect(data.getRecordAt(0).mimeType).assertEqual('xy2');
            expect(data.getRecordAt(1).mimeType).assertEqual('x'.repeat(1024));
            expect(data.getRecordAt(2).mimeType).assertEqual('xy');
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test3
   * @tc.desc      自定义数据异常测试
   * @tc.type      Function
   * @tc.require   AR000HEECB
   */
  it('pasteboard_function_test3', 0, async function (done) {
    console.info('pasteboard_function_test1 start');
    var systemPasteBoard = pasteboard.getSystemPasteboard();
    await systemPasteBoard.clear().then(async () => {
      console.info('systemPasteBoard clear data success');
      var pasteData = undefined;
      var pasteRecord = undefined;

      var dataHtml = new ArrayBuffer(256);
      try {
        pasteData = pasteboard.createData('x'.repeat(1025), dataHtml);
        expect(true === false).assertTrue();
      } catch (error) {
        console.info(error.code);
        console.info(error.message);
      }

      expect(pasteData).assertEqual(undefined);
      pasteData = pasteboard.createData('x'.repeat(1024), dataHtml);
      expect(pasteData != undefined).assertTrue();

      var addHtml = new ArrayBuffer(128);

      try {
        pasteData.addRecord('x'.repeat(1025), addHtml);
        expect(true === false).assertTrue();
      } catch (error) {
        console.info(error.code);
        console.info(error.message);
      }
      expect(pasteData.getRecordCount()).assertEqual(1);
      pasteData.addRecord('x'.repeat(1024), addHtml);
      expect(pasteData.getRecordCount()).assertEqual(2);

      var recordHtml = new ArrayBuffer(64);
      try {
        pasteRecord = pasteboard.createRecord('x'.repeat(1025), recordHtml);
        expect(true === false).assertTrue();
      } catch (error) {
        console.info(error.code);
        console.info(error.message);
      }
      expect(pasteRecord).assertEqual(undefined);
      pasteRecord = pasteboard.createRecord('x'.repeat(1024), recordHtml);
      expect(pasteRecord != undefined).assertTrue();
      pasteData.addRecord(pasteRecord);
      expect(pasteData.getRecordCount()).assertEqual(3);
      await systemPasteBoard.setPasteData(pasteData).then(async () => {
        console.info('set pastedata success');
        await systemPasteBoard.hasPasteData().then(async (data) => {
          console.info('Check pastedata has data success, result: ' + data);
          expect(data).assertTrue();
          await systemPasteBoard.getPasteData().then(async (data) => {
            console.info('get paste data success');
            expect(data.getRecordCount()).assertEqual(3);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test4
   * @tc.desc      自定义数据异常测试
   * @tc.type      Function
   * @tc.require   AR000HEECB
   */
  it('pasteboard_function_test4', 0, async function (done) {
    console.info('pasteboard_function_test2 start');
    var systemPasteBoard = pasteboard.getSystemPasteboard();
    await systemPasteBoard.clear().then(async () => {
      console.info('systemPasteBoard clear data success');
      var pasteData = undefined;
      var pasteRecord = undefined;

      var dataHtml = new ArrayBuffer(256);
      try {
        pasteData = pasteboard.createData('x'.repeat(1025), dataHtml);
        expect(true === false).assertTrue();
      } catch (error) {
        console.info(error.code);
        console.info(error.message);
      }
      expect(pasteData).assertEqual(undefined);
      pasteData = pasteboard.createData('x'.repeat(1024), dataHtml);
      expect(pasteData != undefined).assertTrue();

      try {
        pasteData.addRecord('x'.repeat(1025), dataHtml);
        expect(true === false).assertTrue();
      } catch (error) {
        console.info(error.code);
        console.info(error.message);
      }
      expect(pasteData.getRecordCount()).assertEqual(1);
      pasteData.addRecord('x'.repeat(1024), dataHtml);
      expect(pasteData.getRecordCount()).assertEqual(2);

      try {
        pasteRecord = pasteboard.createRecord('x'.repeat(1025), dataHtml);
        expect(true === false).assertTrue();
      } catch (error) {
        console.info(error.code);
        console.info(error.message);
      }
      expect(pasteRecord).assertEqual(undefined);
      pasteRecord = pasteboard.createRecord('x'.repeat(1024), dataHtml);
      expect(pasteRecord != undefined).assertTrue();

      pasteData.addRecord(pasteRecord);
      expect(pasteData.getRecordCount()).assertEqual(3);
      await systemPasteBoard.setPasteData(pasteData).then(async () => {
        console.info('set pastedata success');
        await systemPasteBoard.hasPasteData().then(async (data) => {
          console.info('Check pastedata has data success, result: ' + data);
          expect(data).assertTrue();
          await systemPasteBoard.getPasteData().then(async (data) => {
            console.info('get paste data success');
            expect(data.getRecordCount()).assertEqual(3);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test5
   * @tc.desc      一个record中多个数据类型：get primary html、pixelMap、want、text、uri
   * @tc.type      Function
   * @tc.require   AR000HEECB
   */
  it('pasteboard_function_test5', 0, async function (done) {
    var systemPasteBoard = pasteboard.getSystemPasteboard();
    systemPasteBoard.clear().then(() => {
      var dataHtml = new ArrayBuffer(256);
      var htmlText = '<html><head></head><body>Hello!</body></html>';
      var uriText = 'https://www.baidu.com/';
      var wantText = {
        bundleName: 'com.example.myapplication3',
        abilityName: 'com.example.myapplication3.MainAbility',
      };
      var plainText = '';
      var pasteData = pasteboard.createData('x'.repeat(1024), dataHtml);
      var record = pasteData.getRecordAt(0);
      record.htmlText = htmlText;
      record.plainText = plainText;
      record.uri = uriText;
      record.want = wantText;
      var buffer = new ArrayBuffer(128);
      var opt = {
        size: { height: 5, width: 5 },
        pixelFormat: 3,
        editable: true,
        alphaType: 1,
        scaleMode: 1,
      };
      image.createPixelMap(buffer, opt).then((pixelMap) => {
        record.pixelMap = pixelMap;
        pasteData.replaceRecordAt(0, record);
        systemPasteBoard.setPasteData(pasteData).then(() => {
          systemPasteBoard.hasPasteData().then((hasData) => {
            expect(hasData).assertTrue();
            systemPasteBoard.getPasteData().then((data) => {
              expect(data.getRecordCount()).assertEqual(1);
              expect(data.getRecordAt(0).mimeType).assertEqual('x'.repeat(1024));
              expect(data.getPrimaryWant().bundleName).assertEqual(wantText.bundleName);
              expect(data.getPrimaryWant().abilityName).assertEqual(wantText.abilityName);
              var newPixelMap = data.getPrimaryPixelMap();
              newPixelMap.getImageInfo().then((imageInfo) => {
                expect(imageInfo.size.height).assertEqual(opt.size.height);
                expect(imageInfo.size.width).assertEqual(opt.size.width);
              });
              expect(data.getPrimaryUri()).assertEqual(uriText);
              expect(data.getPrimaryText()).assertEqual(plainText);
              expect(data.getPrimaryHtml()).assertEqual(htmlText);
              done();
            });
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test6
   * @tc.desc      Test CreateRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test6', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      var uriText = 'https://www.baidu.com/';
      var pasteData = pasteboard.createUriData(uriText);
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            var pasteData1 = data;
            expect(pasteData1.getRecordCount()).assertEqual(1);
            var uriText1 = 'https://www.baidu.com/1';
            var pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_URI, uriText1);
            var replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
            expect(replace).assertEqual(true);
            var primaryUri = pasteData1.getPrimaryUri();
            expect(primaryUri).assertEqual(uriText1);
            expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
            var primaryUri = pasteData1.getPrimaryUri();
            expect(primaryUri).assertEqual(uriText1);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test7
   * @tc.desc      Test CreateRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test7', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(async () => {
      var uriText = 'https://www.baidu.com/';
      var textData = 'Hello World!';
      var htmlText = '<html><head></head><body>Hello World!</body></html>';
      var wantText = {
        bundleName: 'com.example.myapplication3',
        abilityName: 'com.example.myapplication3.MainAbility',
      };
      var dataHtml = new ArrayBuffer(256);
      var buffer = new ArrayBuffer(128);
      var opt = {
        size: { height: 5, width: 5 },
        pixelFormat: 3,
        editable: true,
        alphaType: 1,
        scaleMode: 1,
      };
      var pixelMap = await image.createPixelMap(buffer, opt);
      var pasteData = pasteboard.createUriData(uriText);

      try {
        var pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_URI, uriText);
        pasteData.addRecord(pasteDataRecord);
        pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_PLAIN, textData);
        pasteData.addRecord(pasteDataRecord);
        pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        pasteData.addRecord(pasteDataRecord);
        pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_WANT, wantText);
        pasteData.addRecord(pasteDataRecord);
        pasteDataRecord = pasteboard.createRecord('x'.repeat(1022), dataHtml);
        pasteData.addRecord(pasteDataRecord);
        pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        pasteData.addRecord(pasteDataRecord);
      } catch (error) {
        expect(error.code === undefined).assertTrue();
        expect(error.message === undefined).assertTrue();
        expect(True === false).assertTrue();
      }
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            expect(data.getRecordCount()).assertEqual(7);
            var dataRecord = data.getRecordAt(3);
            expect(dataRecord.htmlText).assertEqual(htmlText);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test8
   * @tc.desc      Test CreateRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test8', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(async () => {
      var uriText = 'https://www.baidu.com/';
      var htmlText = '<html><head></head><body>Hello World!</body></html>';
      var pasteData = pasteboard.createUriData(uriText);

      try {
        var pasteDataRecord = pasteboard.createRecord('xxddxx', htmlText);
        pasteData.addRecord(pasteDataRecord);
        expect(true === false).assertTrue();
      } catch (error) {
        expect(error.code).assertEqual('401');
        expect(error.message).assertEqual('Parameter error. The value does not match mimeType correctly.');
      }
      done();
    });
  });

  /**
   * @tc.name      pasteboard_function_test9
   * @tc.desc      Test Create Uri Data
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test9', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    var uriText = 'https://www.baidu.com/';
    var pasteData = undefined;
    try {
      pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    } catch (e) {
      expect(true === false).assertTrue();
    }
    systemPasteboard.setPasteData(pasteData).then(() => {
      systemPasteboard.hasPasteData().then((data) => {
        expect(data).assertEqual(true);
        systemPasteboard.getPasteData().then((data) => {
          expect(data.getRecordCount()).assertEqual(1);
          var dataRecord = data.getRecordAt(0);
          expect(dataRecord.uri).assertEqual(uriText);
          done();
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test10
   * @tc.desc      Test Create htmlText Data
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test10', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    var htmlText = '<html><head></head><body>Hello World!</body></html>';
    var pasteData = undefined;
    try {
      pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
    } catch (e) {
      expect(true === false).assertTrue();
    }
    systemPasteboard.setPasteData(pasteData).then(() => {
      systemPasteboard.hasPasteData().then((data) => {
        expect(data).assertEqual(true);
        systemPasteboard.getPasteData().then((data) => {
          expect(data.getRecordCount()).assertEqual(1);
          var dataRecord = data.getRecordAt(0);
          expect(dataRecord.htmlText).assertEqual(htmlText);
          done();
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test11
   * @tc.desc      Test Create wantText Data
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test11', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    var wantText = {
      bundleName: 'com.example.myapplication3',
      abilityName: 'com.example.myapplication3.MainAbility',
    };
    var pasteData = undefined;
    try {
      pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_WANT, wantText);
    } catch (e) {
      expect(true === false).assertTrue();
    }
    systemPasteboard.setPasteData(pasteData).then(() => {
      systemPasteboard.hasPasteData().then((data) => {
        expect(data).assertEqual(true);
        systemPasteboard.getPasteData().then((data) => {
          expect(data.getRecordCount()).assertEqual(1);
          var primaryWant = data.getPrimaryWant();
          expect(primaryWant.bundleName).assertEqual(wantText.bundleName);
          expect(primaryWant.abilityName).assertEqual(wantText.abilityName);
          done();
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test12
   * @tc.desc      Test Create pixelMap Data
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test12', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    var buffer = new ArrayBuffer(128);
    var opt = {
      size: { height: 5, width: 5 },
      pixelFormat: 3,
      editable: true,
      alphaType: 1,
      scaleMode: 1,
    };
    var pasteData = undefined;
    var pixelMap = await image.createPixelMap(buffer, opt);
    try {
      pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
    } catch (e) {
      expect(true === false).assertTrue();
    }
    systemPasteboard.setPasteData(pasteData).then(() => {
      systemPasteboard.hasPasteData().then((data) => {
        expect(data).assertEqual(true);
        systemPasteboard.getPasteData().then((data) => {
          expect(data.getRecordCount()).assertEqual(1);
          var primaryPixelMap = data.getPrimaryPixelMap();
          var PixelMapBytesNumber = primaryPixelMap.getPixelBytesNumber();
          expect(PixelMapBytesNumber).assertEqual(100);
          primaryPixelMap.getImageInfo().then((imageInfo) => {
            expect(imageInfo.size.height === 5 && imageInfo.size.width === 5).assertEqual(true);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test13
   * @tc.desc      Test CreateData throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test13', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    var dataHtml = new ArrayBuffer(256);
    var pasteData = undefined;
    try {
      pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, dataHtml);
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code).assertEqual('401');
      expect(e.message).assertEqual('Parameter error. The value does not match mimeType correctly.');
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test14
   * @tc.desc      Test Create KV Data
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test14', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    await systemPasteboard.clear();
    var dataHtml = new ArrayBuffer(256);
    var pasteData = undefined;
    try {
      pasteData = pasteboard.createData('x'.repeat(1034), dataHtml);
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code === '401').assertTrue();
      expect(e.message === 'Parameter error. The length of mimeType cannot be greater than 1024 bytes.').assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test15
   * @tc.desc      Test addRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test15', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(async () => {
      var uriText = 'https://www.baidu.com/';
      var textData = 'Hello World!';
      var htmlText = '<html><head></head><body>Hello World!</body></html>';
      var wantText = {
        bundleName: 'com.example.myapplication3',
        abilityName: 'com.example.myapplication3.MainAbility',
      };
      var dataHtml = new ArrayBuffer(256);
      var buffer = new ArrayBuffer(128);
      var opt = {
        size: { height: 5, width: 5 },
        pixelFormat: 3,
        editable: true,
        alphaType: 1,
        scaleMode: 1,
      };
      var pixelMap = await image.createPixelMap(buffer, opt);
      var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);

      try {
        pasteData.addRecord(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        pasteData.addRecord(pasteboard.MIMETYPE_TEXT_URI, uriText);
        pasteData.addRecord(pasteboard.MIMETYPE_TEXT_PLAIN, textData);
        pasteData.addRecord(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        pasteData.addRecord(pasteboard.MIMETYPE_TEXT_WANT, wantText);
        pasteData.addRecord('x'.repeat(100), dataHtml);
      } catch (error) {
        expect(true === false).assertTrue();
      }
      systemPasteboard.setPasteData(pasteData).then(() => {
        systemPasteboard.hasPasteData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getPasteData().then((data) => {
            expect(data.getRecordCount()).assertEqual(7);
            var dataRecord = data.getRecordAt(6);
            expect(dataRecord.uri).assertEqual(uriText);
            var primaryPixelMap = data.getPrimaryPixelMap();
            var PixelMapBytesNumber = primaryPixelMap.getPixelBytesNumber();
            expect(PixelMapBytesNumber).assertEqual(100);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test16
   * @tc.desc      Test addRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test16', 0, async function (done) {
    var uriText = 'https://www.baidu.com/';
    var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    try {
      pasteData.addRecord('xxxx', uriText);
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code === '401').assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test17
   * @tc.desc      Test addRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test17', 0, async function (done) {
    var uriText = 'https://www.baidu.com/';
    var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    try {
      for (var i = 0; i < 600; i++) {
        pasteData.addRecord(pasteboard.MIMETYPE_TEXT_URI, uriText);
      }
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code === '12900002').assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test18
   * @tc.desc      Test getRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test18', 0, async function (done) {
    var uriText = 'https://www.baidu.com/';
    var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    try {
      var dataRecord = pasteData.getRecord(0);
      expect(dataRecord.uri).assertEqual(uriText);
    } catch (e) {
      expect(true === false).assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test19
   * @tc.desc      Test getRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test19', 0, async function (done) {
    var uriText = 'https://www.baidu.com/';
    var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    try {
      var dataRecord = pasteData.getRecord(5);
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code === '12900001').assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test20
   * @tc.desc      Test replaceRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test20', 0, async function (done) {
    var uriText = 'https://www.baidu.com/';
    var uriText1 = 'https://www.baidu1.com/';
    var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    var dataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_URI, uriText1);
    try {
      pasteData.replaceRecord(0, dataRecord);
      var record = pasteData.getRecord(0);
      expect(record.uri).assertEqual(uriText1);
    } catch (e) {
      expect(true === false).assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test21
   * @tc.desc      Test replaceRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test21', 0, async function (done) {
    var uriText = 'https://www.baidu.com/';
    var uriText1 = 'https://www.baidu1.com/';
    var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    var dataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_URI, uriText1);
    try {
      pasteData.replaceRecord(0, dataRecord);
      var record = pasteData.getRecord(0);
      expect(record.uri).assertEqual(uriText1);
    } catch (e) {
      expect(true === false).assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test22
   * @tc.desc      Test replaceRecord throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test22', 0, async function (done) {
    var uriText = 'https://www.baidu.com/';
    var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    try {
      pasteData.replaceRecord(0, 'xxxxxx');
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code === '401').assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test23
   * @tc.desc      Test setData
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test23', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clearData().then(() => {
      var uriText = 'Hello//';
      var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
      systemPasteboard.setData(pasteData).then(() => {
        systemPasteboard.hasData().then((data) => {
          expect(data).assertEqual(true);
          systemPasteboard.getData().then((pasteData1) => {
            expect(pasteData1.getRecordCount()).assertEqual(1);
            expect(pasteData1.hasType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
            expect(pasteData1.getPrimaryUri()).assertEqual(uriText);
            done();
          });
        });
      });
    });
  });

  /**
   * @tc.name      pasteboard_function_test24
   * @tc.desc      Test setData throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test24', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    try {
      systemPasteboard.setData('xxxxx');
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code === '401').assertTrue();
      expect(e.message === 'Parameter error. The Type of data must be pasteData.').assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test25
   * @tc.desc      Test setproperty throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test25', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(async () => {
      var textData = 'Hello World!';
      var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_PLAIN, textData);
      var pasteDataProperty = pasteData.getProperty();
      expect(pasteDataProperty.shareOption).assertEqual(pasteboard.ShareOption.CrossDevice);
      pasteDataProperty.shareOption = pasteboard.ShareOption.InApp;
      pasteData.setProperty(pasteDataProperty);
      expect(pasteData.getProperty().shareOption).assertEqual(pasteboard.ShareOption.InApp);
      done();
    });
  });

  /**
   * @tc.name      pasteboard_function_test26
   * @tc.desc      Test setproperty throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test26', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(async () => {
      var textData = 'Hello World!';
      var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_PLAIN, textData);
      try {
        var obj = { shareOption: 1 };
        pasteData.setProperty(obj);
        expect(true === false).assertTrue();
      } catch (e) {
        expect(e.code === '401').assertTrue();
        expect(e.message === 'Parameter error. The type of property must be PasteDataProperty.').assertTrue();
      }
      done();
    });
  });

  /**
   * @tc.name      pasteboard_function_test27
   * @tc.desc      Test createData throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test27', 0, async function (done) {
    var textData = 'Hello World!';
    var dataXml = new ArrayBuffer(512);
    try {
      var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_PLAIN, dataXml);
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code === '401').assertTrue();
      expect(e.message === 'Parameter error. The value does not match mimeType correctly.').assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test28
   * @tc.desc      Test createData throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test28', 0, async function (done) {
    var textData = 'Hello World!';
    var dataXml = new ArrayBuffer(512);
    try {
      var pasteData = pasteboard.createData('xxxxx', textData);
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code === '401').assertTrue();
      expect(e.message === 'Parameter error. The value does not match mimeType correctly.').assertTrue();
    }
    done();
  });

  /**
   * @tc.name      pasteboard_function_test29
   * @tc.desc      Test createData throw error
   * @tc.type      Function
   * @tc.require   I5TYVJ
   */
  it('pasteboard_function_test29', 0, async function (done) {
    try {
      var pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, {});
      expect(true === false).assertTrue();
    } catch (e) {
      expect(e.code === '401').assertTrue();
      expect(e.message === 'Parameter error. The value does not match mimeType correctly.').assertTrue();
    }
    done();
  });
});
