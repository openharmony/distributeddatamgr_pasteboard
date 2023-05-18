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

    const BASE_CONUT = 100;
    const htmlText = '<html><head></head><body>Hello!</body></html>';


    /**
     * @tc.name      clearData_Promise_performance_test_001
     * @tc.desc      clearData interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('clearData_Promise_performance_test_001', 0, async function (done) {
        let systemPasteboard = pasteboard.getSystemPasteboard();
        let startTime = new Date().getTime();
        clearDataPromisePerfTest(0);

        function clearDataPromisePerfTest(index) {
            systemPasteboard.clearData().then(() => {
                if (index < BASE_CONUT) {
                    clearDataPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "clearData_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      clear_Promise_performance_test_001
     * @tc.desc      clear interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('clear_Promise_performance_test_001', 0, async function (done) {
        let systemPasteboard = pasteboard.getSystemPasteboard();
        let startTime = new Date().getTime();
        clearPromisePerfTest(0);

        function clearPromisePerfTest(index) {
            systemPasteboard.clearData().then(() => {
                if (index < BASE_CONUT) {
                    clearPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "clear_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      setData_Promise_performance_test_001
     * @tc.desc      setData interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('setData_Promise_performance_test_001', 0, async function (done) {
        let systemPasteboard = pasteboard.getSystemPasteboard();
        let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        let startTime = new Date().getTime();
        setDataPromisePerfTest(0);

        function setDataPromisePerfTest(index) {
            systemPasteboard.setData(pasteData).then(() => {
                if (index < BASE_CONUT) {
                    setDataPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "setData_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      setPasteData_Promise_performance_test_001
     * @tc.desc      setPasteData interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('setPasteData_Promise_performance_test_001', 0, async function (done) {
        let systemPasteboard = pasteboard.getSystemPasteboard();
        let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        let startTime = new Date().getTime();
        setPasteDataPromisePerfTest(0);

        function setPasteDataPromisePerfTest(index) {
            systemPasteboard.setPasteData(pasteData).then(() => {
                if (index < BASE_CONUT) {
                    setPasteDataPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "setPasteData_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      hasData_Promise_performance_test_001
     * @tc.desc      hasData interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('hasData_Promise_performance_test_001', 0, async function (done) {
        let systemPasteboard = pasteboard.getSystemPasteboard();
        let startTime = new Date().getTime();
        hasDataPromisePerfTest(0);

        function hasDataPromisePerfTest(index) {
            systemPasteboard.hasData().then(() => {
                if (index < BASE_CONUT) {
                    hasDataPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "hasData_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      hasPasteData_Promise_performance_test_001
     * @tc.desc      hasPasteData interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('hasPasteData_Promise_performance_test_001', 0, async function (done) {
        let systemPasteboard = pasteboard.getSystemPasteboard();
        let startTime = new Date().getTime();
        hasPasteDataPromisePerfTest(0);

        function hasPasteDataPromisePerfTest(index) {
            systemPasteboard.hasPasteData().then(() => {
                if (index < BASE_CONUT) {
                    hasPasteDataPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "hasPasteData_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      getData_Promise_performance_test_001
     * @tc.desc      getData interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('getData_Promise_performance_test_001', 0, async function (done) {
        let systemPasteboard = pasteboard.getSystemPasteboard();
        let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        await systemPasteboard.clearData();
        await systemPasteboard.setData(pasteData);
        let startTime = new Date().getTime();
        getDataPromisePerfTest(0);

        function getDataPromisePerfTest(index) {
            systemPasteboard.getData().then(() => {
                if (index < BASE_CONUT) {
                    getDataPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "getData_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      getPasteData_Promise_performance_test_001
     * @tc.desc      getPasteData interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('getPasteData_Promise_performance_test_001', 0, async function (done) {
        let systemPasteboard = pasteboard.getSystemPasteboard();
        let pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        await systemPasteboard.clearData();
        await systemPasteboard.setData(pasteData);
        let startTime = new Date().getTime();
        getPasteDataPromisePerfTest(0);

        function getPasteDataPromisePerfTest(index) {
            systemPasteboard.getData().then(() => {
                if (index < BASE_CONUT) {
                    getPasteDataPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "getPasteData_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      convertToText_Promise_performance_test_001
     * @tc.desc      convertToText interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('convertToText_Promise_performance_test_001', 0, async function (done) {
        let pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        let startTime = new Date().getTime();
        convertToTextPromisePerfTest(0);

        function convertToTextPromisePerfTest(index) {
            pasteDataRecord.convertToText().then(() => {
                if (index < BASE_CONUT) {
                    convertToTextPromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "convertToText_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      convertToTextV9_Promise_performance_test_001
     * @tc.desc      convertToTextV9 interface promise performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('convertToTextV9_Promise_performance_test_001', 0, async function (done) {
        let pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        let startTime = new Date().getTime();
        convertToTextV9PromisePerfTest(0);

        function convertToTextV9PromisePerfTest(index) {
            pasteDataRecord.convertToTextV9().then(() => {
                if (index < BASE_CONUT) {
                    convertToTextV9PromisePerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, "convertToTextV9_Promise_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    function computeAverageTime(startTime, baseCount, message) {
        let endTime = new Date().getTime();
        let averageTime = ((endTime - startTime) * 1000) / baseCount;
        console.info(message + averageTime);
    }

});
