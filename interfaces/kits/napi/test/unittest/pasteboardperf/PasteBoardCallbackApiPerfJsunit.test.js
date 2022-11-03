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

    const BASE_CONUT = 1000;
    const BASELINE = 5000;

    const htmlText = '<html><head></head><body>Hello!</body></html>';


    /**
     * @tc.name      clearData_Callback_performance_test_001
     * @tc.desc      clearData interface Callback performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('clearData_Callback_performance_test_001', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        var startTime = new Date().getTime();
        clearDataCallbackPerfTest(0);

        function clearDataCallbackPerfTest(index) {
            systemPasteboard.clearData(() => {
                if (index < BASE_CONUT) {
                    clearDataCallbackPerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, BASELINE, "clearData_Callback_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      clear_Callback_performance_test_001
     * @tc.desc      clear interface Callback performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('clear_Callback_performance_test_001', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        var startTime = new Date().getTime();
        clearCallbackPerfTest(0);

        function clearCallbackPerfTest(index) {
            systemPasteboard.clear(() => {
                if (index < BASE_CONUT) {
                    clearCallbackPerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, BASELINE, "clear_Callback_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      setData_Callback_performance_test_001
     * @tc.desc      setData interface Callback performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('setData_Callback_performance_test_001', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        var startTime = new Date().getTime();
        setDataCallbackPerfTest(0);

        function setDataCallbackPerfTest(index) {
            systemPasteboard.setData(pasteData, () => {
                if (index < BASE_CONUT) {
                    setDataCallbackPerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, BASELINE, "setData_Callback_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      setPasteData_Callback_performance_test_001
     * @tc.desc      setPasteData interface Callback performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('setPasteData_Callback_performance_test_001', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        var startTime = new Date().getTime();
        setPasteDataCallbackPerfTest(0);

        function setPasteDataCallbackPerfTest(index) {
            systemPasteboard.setPasteData(pasteData, () => {
                if (index < BASE_CONUT) {
                    setPasteDataCallbackPerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, BASELINE, "setPasteData_Callback_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      hasData_Callback_performance_test_001
     * @tc.desc      hasData interface Callback performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('hasData_Callback_performance_test_001', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        var startTime = new Date().getTime();
        hasDataCallbackPerfTest(0);

        function hasDataCallbackPerfTest(index) {
            systemPasteboard.hasData(() => {
                if (index < BASE_CONUT) {
                    hasDataCallbackPerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, BASELINE, "hasData_Callback_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      hasPasteData_Callback_performance_test_001
     * @tc.desc      hasPasteData interface Callback performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('hasPasteData_Callback_performance_test_001', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        var startTime = new Date().getTime();
        hasPasteDataCallbackPerfTest(0);

        function hasPasteDataCallbackPerfTest(index) {
            systemPasteboard.hasPasteData(() => {
                if (index < BASE_CONUT) {
                    hasPasteDataCallbackPerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, BASELINE, "hasPasteData_Callback_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      getData_Callback_performance_test_001
     * @tc.desc      getData interface Callback performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('getData_Callback_performance_test_001', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        await systemPasteboard.clearData();
        await systemPasteboard.setData(pasteData);
        var startTime = new Date().getTime();
        getDataCallbackPerfTest(0);

        function getDataCallbackPerfTest(index) {
            systemPasteboard.getPasteData(() => {
                if (index < BASE_CONUT) {
                    getDataCallbackPerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, BASELINE, "getData_Callback_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })

    /**
     * @tc.name      getPasteData_Callback_performance_test_001
     * @tc.desc      getPasteData interface Callback performance test
     * @tc.type      PERF
     * @tc.require   I5YP4X
     */
    it('getPasteData_Callback_performance_test_001', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_HTML, htmlText);
        await systemPasteboard.clearData();
        await systemPasteboard.setData(pasteData);
        var startTime = new Date().getTime();
        getPasteDataCallbackPerfTest(0);

        function getPasteDataCallbackPerfTest(index) {
            systemPasteboard.getPasteData(() => {
                if (index < BASE_CONUT) {
                    getPasteDataCallbackPerfTest(index + 1);
                } else {
                    computeAverageTime(startTime, BASE_CONUT, BASELINE, "getPasteData_Callback_performance_test_001 averageTime:");
                    done();
                }
            });
        }
    })


    function computeAverageTime(startTime, baseCount, baseTime, message) {
        var endTime = new Date().getTime();
        var averageTime = ((endTime - startTime) * 1000) / baseCount;
        console.info(message + averageTime);
        expect(averageTime < baseTime).assertTrue();
    }

});
