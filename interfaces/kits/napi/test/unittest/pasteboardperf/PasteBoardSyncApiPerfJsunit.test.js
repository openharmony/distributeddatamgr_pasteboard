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

describe('PasteBoardPerfJSTest', function () {
  beforeAll(async function () {
    console.info('beforeAll');
  });

  afterAll(async function () {
    console.info('afterAll');
  });

  const BASE_CONUT = 200;
  const pixelMapBuffer = new ArrayBuffer(10000);
  const opt = {
    size: { height: 50, width: 50 },
    pixelFormat: 3,
    editable: true,
    alphaType: 1,
    scaleMode: 1,
  };
  const htmlText = '<html><head></head><body>Hello!</body></html>';
  const uriText = 'https://www.baidu.com/';
  const plainText = 'Hello World!';
  const wantText = {
    bundleName: 'com.example.myapplication8',
    abilityName: 'com.example.myapplication8.MainAbility',
  };

  /**
   * @tc.name      getSystemPasteboard_performance_test_001
   * @tc.desc      getSystemPasteboard interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getSystemPasteboard_performance_test_001', 0, async function (done) {
    let startTime = new Date().getTime();
    funcWithNoparam(pasteboard.getSystemPasteboard, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getSystemPasteboard_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createData_performance_test_001
   * @tc.desc      createData interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createData_performance_test_001', 0, async function (done) {
    let pixelMap = await image.createPixelMap(pixelMapBuffer, opt);
    let startTime = new Date().getTime();
    funcWithTwoparam(pasteboard.createData, pasteboard.MIMETYPE_PIXELMAP, pixelMap, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createData_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createRecord_performance_test_001
   * @tc.desc      createRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createRecord_performance_test_001', 0, async function (done) {
    let pixelMap = await image.createPixelMap(pixelMapBuffer, opt);
    let startTime = new Date().getTime();
    funcWithTwoparam(pasteboard.createRecord, pasteboard.MIMETYPE_PIXELMAP, pixelMap, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createHtmlData_performance_test_001
   * @tc.desc      createHtmlData interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createHtmlData_performance_test_001', 0, async function (done) {
    let startTime = new Date().getTime();
    funcWithOneparam(pasteboard.createHtmlData, htmlText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createHtmlData_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createWantData_performance_test_001
   * @tc.desc      createWantData interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createWantData_performance_test_001', 0, async function (done) {
    let startTime = new Date().getTime();
    funcWithOneparam(pasteboard.createWantData, wantText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createWantData_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createPlainTextData_performance_test_001
   * @tc.desc      createPlainTextData interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createPlainTextData_performance_test_001', 0, async function (done) {
    let startTime = new Date().getTime();
    funcWithOneparam(pasteboard.createPlainTextData, plainText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createPlainTextData_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createUriData_performance_test_001
   * @tc.desc      createUriData interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createUriData_performance_test_001', 0, async function (done) {
    let startTime = new Date().getTime();
    funcWithOneparam(pasteboard.createUriData, uriText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createUriData_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createHtmlTextRecord_performance_test_001
   * @tc.desc      createHtmlTextRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createHtmlTextRecord_performance_test_001', 0, async function (done) {
    let startTime = new Date().getTime();
    funcWithOneparam(pasteboard.createHtmlTextRecord, htmlText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createHtmlTextRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createWantRecord_performance_test_001
   * @tc.desc      createWantRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createWantRecord_performance_test_001', 0, async function (done) {
    let startTime = new Date().getTime();
    funcWithOneparam(pasteboard.createWantRecord, wantText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createWantRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createPlainTextRecord_performance_test_001
   * @tc.desc      createPlainTextRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createPlainTextRecord_performance_test_001', 0, async function (done) {
    let startTime = new Date().getTime();
    funcWithOneparam(pasteboard.createPlainTextRecord, plainText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createPlainTextRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      createUriRecord_performance_test_001
   * @tc.desc      createUriRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('createUriRecord_performance_test_001', 0, async function (done) {
    let startTime = new Date().getTime();
    funcWithOneparam(pasteboard.createUriRecord, uriText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'createUriRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      addRecord_performance_test_001
   * @tc.desc      addRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('addRecord_performance_test_001', 0, async function (done) {
    let pixelMap = await image.createPixelMap(pixelMapBuffer, opt);
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithTwoparam(pasteData.addRecord, pasteboard.MIMETYPE_PIXELMAP, pixelMap, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'addRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getRecord_performance_test_001
   * @tc.desc      getRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getRecord_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.getRecord, 0, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      hasType_performance_test_001
   * @tc.desc      hasType interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('hasType_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.hasType, pasteboard.MIMETYPE_PIXELMAP, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'hasType_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      removeRecord_performance_test_001
   * @tc.desc      removeRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('removeRecord_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    funcWithOneparam(pasteData.addHtmlRecord, htmlText, BASE_CONUT);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.removeRecord, 0, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'removeRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      replaceRecord_performance_test_001
   * @tc.desc      replaceRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('replaceRecord_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let dataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithTwoparam(pasteData.replaceRecord, 0, dataRecord, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'replaceRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      addHtmlRecord_performance_test_001
   * @tc.desc      addHtmlRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('addHtmlRecord_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.addHtmlRecord, htmlText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'addHtmlRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      addWantRecord_performance_test_001
   * @tc.desc      addWantRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('addWantRecord_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.addWantRecord, wantText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'addWantRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      addRecord_performance_test_002
   * @tc.desc      addRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('addRecord_performance_test_002', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let dataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.addRecord, dataRecord, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'addRecord_performance_test_002 averageTime:');
    done();
  });

  /**
   * @tc.name      addTextRecord_performance_test_001
   * @tc.desc      addTextRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('addTextRecord_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.addTextRecord, plainText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'addTextRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      addUriRecord_performance_test_001
   * @tc.desc      addUriRecord interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('addUriRecord_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.addUriRecord, uriText, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'addUriRecord_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getMimeTypes_performance_test_001
   * @tc.desc      getMimeTypes interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getMimeTypes_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithNoparam(pasteData.getMimeTypes, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getMimeTypes_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getPrimaryHtml_performance_test_001
   * @tc.desc      getPrimaryHtml interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getPrimaryHtml_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
    let startTime = new Date().getTime();
    funcWithNoparam(pasteData.getPrimaryHtml, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getPrimaryHtml_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getPrimaryWant_performance_test_001
   * @tc.desc      getPrimaryWant interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getPrimaryWant_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_WANT, wantText);
    let startTime = new Date().getTime();
    funcWithNoparam(pasteData.getPrimaryWant, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getPrimaryWant_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getPrimaryMimeType_performance_test_001
   * @tc.desc      getPrimaryMimeType interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getPrimaryMimeType_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_WANT, wantText);
    let startTime = new Date().getTime();
    funcWithNoparam(pasteData.getPrimaryMimeType, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getPrimaryMimeType_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getPrimaryUri_performance_test_001
   * @tc.desc      getPrimaryUri interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getPrimaryUri_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithNoparam(pasteData.getPrimaryUri, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getPrimaryUri_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getPrimaryPixelMap_performance_test_001
   * @tc.desc      getPrimaryPixelMap interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getPrimaryPixelMap_performance_test_001', 0, async function (done) {
    let pixelMap = await image.createPixelMap(pixelMapBuffer, opt);
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
    let startTime = new Date().getTime();
    funcWithNoparam(pasteData.getPrimaryPixelMap, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getPrimaryPixelMap_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getProperty_performance_test_001
   * @tc.desc      getProperty interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getProperty_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithNoparam(pasteData.getProperty, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getProperty_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      setProperty_performance_test_001
   * @tc.desc      setProperty interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('setProperty_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let property = pasteData.getProperty();
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.setProperty, property, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'setProperty_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getRecordAt_performance_test_001
   * @tc.desc      getRecordAt interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getRecordAt_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.getRecordAt, 0, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getRecordAt_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      getRecordCount_performance_test_001
   * @tc.desc      getRecordCount interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('getRecordCount_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithNoparam(pasteData.getRecordCount, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'getRecordCount_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      hasMimeType_performance_test_001
   * @tc.desc      hasMimeType interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('hasMimeType_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.hasMimeType, pasteboard.MIMETYPE_TEXT_URI, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'hasMimeType_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      removeRecordAt_performance_test_001
   * @tc.desc      removeRecordAt interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('removeRecordAt_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let startTime = new Date().getTime();
    funcWithOneparam(pasteData.removeRecordAt, 0, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'removeRecordAt_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      replaceRecordAt_performance_test_001
   * @tc.desc      replaceRecordAt interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('replaceRecordAt_performance_test_001', 0, async function (done) {
    let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_URI, uriText);
    let dataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
    let startTime = new Date().getTime();
    funcWithTwoparam(pasteData.replaceRecordAt, 0, dataRecord, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'replaceRecordAt_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      on_performance_test_001
   * @tc.desc      on interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('on_performance_test_001', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    let startTime = new Date().getTime();
    funcWithTwoparam(systemPasteboard.on, 'update', contentChanges, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'on_performance_test_001 averageTime:');
    done();
  });

  /**
   * @tc.name      off_performance_test_001
   * @tc.desc      off interface performance test
   * @tc.type      PERF
   * @tc.require   I5YP4X
   */
  it('off_performance_test_001', 0, async function (done) {
    let systemPasteboard = pasteboard.getSystemPasteboard();
    let startTime = new Date().getTime();
    funcWithTwoparam(systemPasteboard.off, 'update', contentChanges, BASE_CONUT);
    computeAverageTime(startTime, BASE_CONUT, 'off_performance_test_001 averageTime:');
    done();
  });

  function funcWithNoparam(func, count) {
    for (let index = 0; index < count; index++) {
      func();
    }
  }

  function funcWithOneparam(func, param, count) {
    for (let index = 0; index < count; index++) {
      func(param);
    }
  }

  function funcWithTwoparam(func, paramOne, paramTwo, count) {
    for (let index = 0; index < count; index++) {
      func(paramOne, paramTwo);
    }
  }

  function computeAverageTime(startTime, baseCount, message) {
    let endTime = new Date().getTime();
    let averageTime = ((endTime - startTime) * 1000) / baseCount;
    console.info(message + averageTime);
  }

  function contentChanges() {
    console.info('#EVENT: The content is changed in the pasteboard');
  }
});
