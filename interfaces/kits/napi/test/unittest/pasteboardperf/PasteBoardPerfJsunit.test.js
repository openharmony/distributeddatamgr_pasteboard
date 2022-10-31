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
import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'
import pasteboard from '@ohos.pasteboard'
import image from '@ohos.multimedia.image'

describe('PasteBoardPerfJSTest', function () {
    beforeAll(async function () {
        console.info('beforeAll');
    })

    afterAll(async function () {
        console.info('afterAll');
    })

    var BASE_CONUT = 2000;
    var ADDRECORD_COUNT = 500;
    var BASELINE = 5000;
    var REMOVERECORD_COUNT = 1;

    var buffer = new ArrayBuffer(10000);
    var opt = {
        size: {height: 50, width: 50},
        pixelFormat: 3,
        editable: true,
        alphaType: 1,
        scaleMode: 1
    };

    /**
     * @tc.name      getSystemPasteboard_performance_test_001
     * @tc.desc      getSystemPasteboard interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('getSystemPasteboard_performance_test_001', 0, async function (done) {
        var startTime = new Date().getTime();
        for (var index = 0; index < BASE_CONUT; index++) {
            var systemPasteboard = pasteboard.getSystemPasteboard();
        }
        computeAverageTime(startTime, BASE_CONUT, BASELINE, "getSystemPasteboard_performance_test_001 averageTime:");
        done();
    })

    /**
     * @tc.name      clearData_Promise_performance_test_001
     * @tc.desc      clearData interface promise performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('clearData_Promise_performance_test_001', 0, async function (done) {
        var startTime = new Date().getTime();
        var systemPasteboard = pasteboard.getSystemPasteboard();
        clearDataPromisePerfTest(0);

        function clearDataPromisePerfTest(index) {
            var promise = systemPasteboard.clearData();
            promise.then(() => {
                if (index < BASE_CONUT) {
                    clearDataPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, BASELINE, "clearData_Promise_performance_test_001 averageTime:");
                    done();
                }
            }).catch(err => {
                console.info("clearData_Promise_performance_test_001 failed, err:" + err);
            })
        }
    })

    /**
     * @tc.name      createData_performance_test_001
     * @tc.desc      createData interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('createData_performance_test_001', 0, async function (done) {
        var pixelMap = await image.createPixelMap(buffer, opt);
        var startTime = new Date().getTime();
        for (var index = 0; index < BASE_CONUT; index++) {
            var pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        }
        computeAverageTime(startTime, BASE_CONUT, BASELINE, "createData_performance_test_001 averageTime:");
        done();
    })

    /**
     * @tc.name      createRecord_performance_test_001
     * @tc.desc      createRecord interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('createRecord_performance_test_001', 0, async function (done) {
        var pixelMap = await image.createPixelMap(buffer, opt);
        var startTime = new Date().getTime();
        for (var index = 0; index < BASE_CONUT; index++) {
            var dataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        }
        computeAverageTime(startTime, BASE_CONUT, BASELINE, "createRecord_performance_test_001 averageTime:");
        done();
    })

    /**
     * @tc.name      addRecord_performance_test_001
     * @tc.desc      addRecord interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('addRecord_performance_test_001', 0, async function (done) {
        var pixelMap = await image.createPixelMap(buffer, opt);
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        var startTime = new Date().getTime();
        for (var index = 0; index < ADDRECORD_COUNT; index++) {
            pasteData.addRecord(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        }
        computeAverageTime(startTime, ADDRECORD_COUNT, BASELINE, "addRecord_performance_test_001 averageTime:");
        done();
    })

    /**
     * @tc.name      getRecord_performance_test_001
     * @tc.desc      getRecord interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('getRecord_performance_test_001', 0, async function (done) {
        var pixelMap = await image.createPixelMap(buffer, opt);
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        var startTime = new Date().getTime();
        for (var index = 0; index < BASE_CONUT; index++) {
            var dataRecord = pasteData.getRecord(0);
        }
        computeAverageTime(startTime, ADDRECORD_COUNT, BASELINE, "getRecord_performance_test_001 averageTime:");
        done();
    })

    /**
     * @tc.name      hasType_performance_test_001
     * @tc.desc      hasType interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('hasType_performance_test_001', 0, async function (done) {
        var pixelMap = await image.createPixelMap(buffer, opt);
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        var startTime = new Date().getTime();
        for (var index = 0; index < BASE_CONUT; index++) {
            var type = pasteData.hasType(pasteboard.MIMETYPE_PIXELMAP);
        }
        computeAverageTime(startTime, BASE_CONUT, BASELINE, "hasType_performance_test_001 averageTime:");
        done();
    })

    /**
     * @tc.name      removeRecord_performance_test_001
     * @tc.desc      removeRecord interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('removeRecord_performance_test_001', 0, async function (done) {
        var pixelMap = await image.createPixelMap(buffer, opt);
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        var startTime = new Date().getTime();
        pasteData.removeRecord(0);
        computeAverageTime(startTime, REMOVERECORD_COUNT, BASELINE, "removeRecord_performance_test_001 averageTime:");
        done();
    })

    /**
     * @tc.name      replaceRecord_performance_test_001
     * @tc.desc      replaceRecord interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('replaceRecord_performance_test_001', 0, async function (done) {
        var pixelMap = await image.createPixelMap(buffer, opt);
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        var uriText = 'https://www.baidu.com/';
        var dataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_URI, uriText);
        var startTime = new Date().getTime();
        for (var index = 0; index < BASE_CONUT; index++) {
            pasteData.replaceRecord(0, dataRecord);
        }
        computeAverageTime(startTime, BASE_CONUT, BASELINE, "replaceRecord_performance_test_001 averageTime:");
        done();
    })

    /**
     * @tc.name      addHtmlRecord_performance_test_001
     * @tc.desc      addHtmlRecord interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('addHtmlRecord_performance_test_001', 0, async function (done) {
        var pixelMap = await image.createPixelMap(buffer, opt);
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        var htmlText = '<html><head></head><body>Hello!</body></html>';
        var startTime = new Date().getTime();
        for (var index = 0; index < BASE_CONUT; index++) {
            pasteData.addHtmlRecord(htmlText);
        }
        computeAverageTime(startTime, BASE_CONUT, BASELINE, "addHtmlRecord_performance_test_001 averageTime:");
        done();
    })

    /**
     * @tc.name      addHtmlRecord_performance_test_001
     * @tc.desc      addHtmlRecord interface performance test
     * @tc.type      PERF
     * @tc.require   AR000H5HVI
     */
    it('addWRecord_performance_test_001', 0, async function (done) {
        var pixelMap = await image.createPixelMap(buffer, opt);
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
        var htmlText = '<html><head></head><body>Hello!</body></html>';
        var startTime = new Date().getTime();
        for (var index = 0; index < BASE_CONUT; index++) {
            pasteData.addHtmlRecord(htmlText);
        }
        computeAverageTime(startTime, BASE_CONUT, BASELINE, "addHtmlRecord_performance_test_001 averageTime:");
        done();
    })

    function computeAverageTime(startTime, baseCount, baseTime, message) {
        var endTime = new Date().getTime();
        var averageTime = ((endTime - startTime) * 1000) / baseCount;
        console.info(message + averageTime);
        expect(averageTime < baseTime).assertTrue();
    }

});
