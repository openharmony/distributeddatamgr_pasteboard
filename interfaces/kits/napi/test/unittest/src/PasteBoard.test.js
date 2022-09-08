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

describe('PasteBoardJSTest', function () {
    beforeAll(async function () {
        console.info('beforeAll');
    })

    afterAll(async function () {
        console.info('afterAll');
    })

    /**
     * @tc.name      pasteboard_function_test1
     * @tc.desc      Adds PlainTextData
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test1', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    var types = pasteData.getMimeTypes();
                    expect("text/plain").assertEqual(types[0]);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var primaryText = pasteData1.getPrimaryText();
                        expect(primaryText).assertEqual(textData);
                        expect(pasteboard.MAX_RECORD_NUM).assertEqual(128);
                        expect(pasteData1.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test2
     * @tc.desc      Adds PlainTextData = ''
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test2', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = '';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var recordCount = data.getRecordCount();
                        expect(recordCount).assertEqual(1);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test3
     * @tc.desc      Adds PlainTextData = 'Hello 中国!@#$%^&*()_+{}\?.'
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test3', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello 中国!@#$%^&*()_+{}\?.';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var primaryText = pasteData1.getPrimaryText()
                        expect(primaryText).assertEqual(textData);
                        expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_PLAIN)).assertEqual(true);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test4
     * @tc.desc      Adds 300K PlainTextData
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test4', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = ''
            for (var i = 0; i < 300; i++) {
                textData = textData + "A";
            }
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var primaryText = pasteData1.getPrimaryText();
                        expect(primaryText).assertEqual(textData);
                        expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_PLAIN)).assertEqual(true);
                        expect(pasteData1.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test5
     * @tc.desc      Adds htmlText
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test5', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var htmlText = '<html><head></head><body>Hello World!</body></html>';
            var pasteData = pasteboard.createHtmlData(htmlText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var primaryHtml = pasteData1.getPrimaryHtml();
                        expect(primaryHtml).assertEqual(htmlText);
                        expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
                        expect(pasteData1.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test6
     * @tc.desc      Adds htmlText = ''
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test6', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var htmlText = '';
            var pasteData = pasteboard.createHtmlData(htmlText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        expect(data.getRecordCount()).assertEqual(1);
                        var primaryHtml = data.getPrimaryHtml();
                        expect(primaryHtml).assertEqual(htmlText);
                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
                        expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test7
     * @tc.desc      Adds htmlText = 'Hello 中国!@#$%^&*()_+{}\?.'
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test7', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var htmlText = 'Hello 中国!@#$%^&*()_+{}\?.'
            var pasteData = pasteboard.createHtmlData(htmlText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        expect(data.getRecordCount()).assertEqual(1);
                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
                        expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
                        expect(data.getPrimaryHtml()).assertEqual(htmlText);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test8
     * @tc.desc      Adds uriText
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test8', 0, async function (done) {
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
                        var primaryUri = pasteData1.getPrimaryUri()
                        expect(primaryUri).assertEqual(uriText);
                        expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test9
     * @tc.desc      Adds uriText = ''
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test9', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var uriText = '';
            var pasteData = pasteboard.createUriData(uriText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        expect(pasteData1.getPrimaryUri()).assertEqual(uriText);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test10
     * @tc.desc      Set uriText = 'Hello //'
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test10', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var uriText = 'Hello//';
            var pasteData = pasteboard.createUriData(uriText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
                        expect(pasteData1.getPrimaryUri()).assertEqual(uriText);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test11
     * @tc.desc      Adds want
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test11', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var want = {
                bundleName: "com.example.myapplication8",
                abilityName: "com.example.myapplication8.MainAbility"
            }
            var pasteData = pasteboard.createWantData(want);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var primaryWant = pasteData1.getPrimaryWant()
                        expect(want.bundleName).assertEqual(primaryWant.bundleName);
                        expect(want.abilityName).assertEqual(primaryWant.abilityName);
                        expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_WANT)).assertEqual(true);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test12
     * @tc.desc      Adds one record(s)
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test12', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var recordText = pasteData1.getRecordAt(0).plainText;
                        expect(recordText).assertEqual(textData);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test13
     * @tc.desc      Adds 2 record(s)
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test13', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData0 = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData0);
            var textData1 = 'Hello World1';
            pasteData.addTextRecord(textData1);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        expect(data.getRecordCount()).assertEqual(2);
                        var dataRecord0 = data.getRecordAt(0);
                        var dataRecord1 = data.getRecordAt(1);
                        expect(dataRecord0.plainText).assertEqual(textData1);
                        expect(dataRecord1.plainText).assertEqual(textData0);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test14
     * @tc.desc      Adds 15 record(s)
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test14', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData0 = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData0);
            var textData = ''
            for (var i = 1; i < 15; i++) {
                textData = 'Hello World'
                textData = textData + i
                pasteData.addTextRecord(textData)
            }
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(15);
                        var dataRecord = pasteData1.getRecordAt(14);
                        expect(dataRecord.plainText).assertEqual(textData0);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test15
     * @tc.desc      Adds 30 record(s)
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test15', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData0 = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData0);
            var textData = ''
            for (var i = 1; i < 30; i++) {
                textData = 'Hello World'
                textData = textData + i
                pasteData.addTextRecord(textData)
            }
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(30);
                        var dataRecord = pasteData1.getRecordAt(0);
                        expect(dataRecord.plainText).assertEqual('Hello World29');
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test16
     * @tc.desc      Adds 31 record(s)
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test16', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData0 = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData0);
            var textData = ''
            for (var i = 1; i < 31; i++) {
                textData = 'Hello World'
                textData = textData + i
                pasteData.addTextRecord(textData)
            }
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        var recordCount = pasteData1.getRecordCount();
                        expect(recordCount).assertEqual(31);
                        var dataRecord = pasteData1.getRecordAt(0);
                        expect(dataRecord.plainText).assertEqual('Hello World30');
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test17
     * @tc.desc      Adds PlainText,HtmlText,UriText
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test17', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            var htmlText = '<html><head></head><body>Hello World!</body></html>';
            pasteData.addHtmlRecord(htmlText);
            var uriText = 'https://www.baidu.com/';
            pasteData.addUriRecord(uriText);
            var want = {
                bundleName: "com.example.myapplication8",
                abilityName: "com.example.myapplication8.MainAbility"
            }
            pasteData.addWantRecord(want);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(4);
                        var wantRecord = pasteData1.getPrimaryWant();
                        expect(wantRecord.bundleName).assertEqual(want.bundleName);
                        expect(wantRecord.abilityName).assertEqual(want.abilityName);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test18
     * @tc.desc      Delete one PlainTextData
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test18', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        systemPasteboard.setPasteData(pasteData1).then(() => {
                            systemPasteboard.getPasteData().then((data) => {
                                var pasteData2 = data;
                                var recordCount = pasteData2.getRecordCount();
                                expect(recordCount).assertEqual(0);
                                done();
                            });
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test19
     * @tc.desc      Delete one htmlText
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test19', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var htmlText = '<html><head></head><body>Hello World!</body></html>'
            var pasteData = pasteboard.createHtmlData(htmlText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        systemPasteboard.setPasteData(pasteData1).then(() => {
                            systemPasteboard.getPasteData().then((data) => {
                                var pasteData2 = data;
                                var recordCount = pasteData2.getRecordCount();
                                expect(recordCount).assertEqual(0);
                                done();
                            });
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test20
     * @tc.desc      Delete one uriText
     * @tc.type      Function
     * @tc.require   AR000H5HVI
     */
    it('pasteboard_function_test20', 0, async function (done) {
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
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        systemPasteboard.setPasteData(pasteData1).then(() => {
                            systemPasteboard.getPasteData().then((data) => {
                                var pasteData2 = data;
                                var recordCount = pasteData2.getRecordCount();
                                expect(recordCount).assertEqual(0);
                                done();
                            });
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test21
     * @tc.desc      Delete one want
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test21', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var want = {
                bundleName: "com.example.myapplication8",
                abilityName: "com.example.myapplication8.MainAbility"
            }
            var pasteData = pasteboard.createWantData(want);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        systemPasteboard.setPasteData(pasteData1).then(() => {
                            systemPasteboard.getPasteData().then((data) => {
                                var pasteData2 = data;
                                var recordCount = pasteData2.getRecordCount();
                                expect(recordCount).assertEqual(0);
                                done();
                            });
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test22
     * @tc.desc      Deletes 300K PlainTextData
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test22', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = ''
            for (var i = 0; i < 300; i++) {
                textData = textData + "A";
            }
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        systemPasteboard.setPasteData(pasteData1).then(() => {
                            systemPasteboard.getPasteData().then((data) => {
                                var pasteData2 = data;
                                var recordCount = pasteData2.getRecordCount();
                                expect(recordCount).assertEqual(0);
                                done();
                            });
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test23
     * @tc.desc      Deletes 30 record(s)
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test23', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData0 = 'Hello World';
            var pasteData = pasteboard.createPlainTextData(textData0);
            var textData = '';
            for (var i = 1; i < 30; i++) {
                textData = 'Hello World';
                textData = textData + i;
                pasteData.addTextRecord(textData);
            }
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(30);
                        for (var i = 0; i < 30; i++) {
                            expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        }
                        expect(pasteData1.getRecordCount()).assertEqual(0);
                        systemPasteboard.setPasteData(pasteData1).then(() => {
                            systemPasteboard.getPasteData().then((data) => {
                                var pasteData2 = data;
                                var recordCount = pasteData2.getRecordCount();
                                expect(recordCount).assertEqual(0);
                                done();
                            });
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test24
     * @tc.desc      Deletes replaced record
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test24', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var textData1 = 'Hello World1';
                        var pasteDataRecord = pasteboard.createPlainTextRecord(textData1);
                        var replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
                        expect(replace).assertEqual(true);
                        var primaryText = pasteData1.getPrimaryText();
                        expect(primaryText).assertEqual(textData1);
                        expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_PLAIN)).assertEqual(true);
                        var dataRecord = pasteData1.getRecordAt(0);
                        expect(dataRecord.plainText).assertEqual(textData1);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        systemPasteboard.setPasteData(pasteData1).then(() => {
                            systemPasteboard.getPasteData().then((data) => {
                                var pasteData2 = data;
                                var recordCount = pasteData2.getRecordCount();
                                expect(recordCount).assertEqual(0);
                                done();
                            });
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test25
     * @tc.desc      Deletes 文本、uri、html
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test25', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            var htmlText = '<html><head></head><body>Hello World!</body></html>'
            pasteData.addHtmlRecord(htmlText);
            var uriText = 'https://www.baidu.com/';
            pasteData.addUriRecord(uriText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(3);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        systemPasteboard.setPasteData(pasteData1).then(() => {
                            systemPasteboard.getPasteData().then((data) => {
                                var pasteData2 = data;
                                var recordCount = pasteData2.getRecordCount();
                                expect(recordCount).assertEqual(0);
                                done();
                            });
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test26
     * @tc.desc      Replaces 文本 record
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test26', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var textData1 = 'Hello World1';
                        var pasteDataRecord = pasteboard.createPlainTextRecord(textData1);
                        var replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
                        expect(replace).assertEqual(true);
                        var primaryText = pasteData1.getPrimaryText();
                        expect(primaryText).assertEqual(textData1);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test27
     * @tc.desc      Replaces htmlText record
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test27', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var htmlText = '<html><head></head><body>Hello World!</body></html>';
            var pasteData = pasteboard.createHtmlData(htmlText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var htmlText1 = '<html><head></head><body>Hello World 1</body></html>';
                        var pasteDataRecord = pasteboard.createHtmlTextRecord(htmlText1);
                        var replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
                        expect(replace).assertEqual(true);
                        var primaryHtml = pasteData1.getPrimaryHtml();
                        expect(primaryHtml).assertEqual(htmlText1);
                        expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
                        var primaryHtml = pasteData1.getPrimaryHtml();
                        expect(primaryHtml).assertEqual(htmlText1);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test28
     * @tc.desc      Replaces uri record
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test28', 0, async function (done) {
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
                        var pasteDataRecord = pasteboard.createUriRecord(uriText1);
                        var replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
                        expect(replace).assertEqual(true);
                        var primaryUri = pasteData1.getPrimaryUri();
                        expect(primaryUri).assertEqual(uriText1);
                        expect(pasteData1.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
                        var primaryUri = pasteData1.getPrimaryUri();
                        expect(primaryUri).assertEqual(uriText1);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test29
     * @tc.desc      Replaces want record
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test29', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var wantText0 = {
                bundleName: "com.example.myapplication3",
                abilityName: "com.example.myapplication3.MainAbility"
            }
            var pasteData = pasteboard.createWantData(wantText0);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var wantText1 = {
                            bundleName: "com.example.myapplication30",
                            abilityName: "com.example.myapplication30.MainAbility"
                        }
                        var pasteDataRecord = pasteboard.createWantRecord(wantText1)
                        var replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
                        expect(replace).assertEqual(true);
                        var primaryWant = pasteData1.getPrimaryWant();
                        expect(primaryWant.bundleName).assertEqual(wantText1.bundleName);
                        expect(primaryWant.abilityName).assertEqual(wantText1.abilityName);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test30
     * @tc.desc      Replaces 300k文本 record
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test30', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = ''
            for (var i = 0; i < 300; i++) {
                textData = textData + "A";
            }
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var textData1 = 'Hello World1'
                        var pasteDataRecord = pasteboard.createPlainTextRecord(textData1);
                        var replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
                        expect(replace).assertEqual(true);
                        var primaryText = pasteData1.getPrimaryText();
                        expect(primaryText).assertEqual(textData1);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test31
     * @tc.desc      Clears pasteBoard, gets record count
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test31', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        await systemPasteboard.clear();
                        systemPasteboard.getPasteData().then((data) => {
                            var pasteData2 = data;
                            var recordCount = pasteData2.getRecordCount();
                            expect(recordCount).assertEqual(0);
                            done();
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test32
     * @tc.desc      Adds Property
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test32', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var pasteDataProperty = pasteData1.getProperty()
                        expect(pasteDataProperty.shareOption).assertEqual(pasteboard.ShareOption.CrossDevice);
                        pasteDataProperty.shareOption = pasteboard.ShareOption.InApp;
                        pasteData1.setProperty(pasteDataProperty);
                        expect(pasteData1.getProperty().shareOption).assertEqual(pasteboard.ShareOption.InApp);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test33
     * @tc.desc      Clears pasteBoard and check property
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test33', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        expect(data.getRecordCount()).assertEqual(1);
                        await systemPasteboard.clear();
                        var newPasteData = await systemPasteboard.getPasteData();
                        expect(newPasteData.getProperty().shareOption).assertEqual(pasteboard.ShareOption.CrossDevice);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test34
     * @tc.desc      打开内容变化通知功能
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test34', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            systemPasteboard.on('update', contentChanges)
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        expect(pasteData1.getRecordCount()).assertEqual(0);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test35
     * @tc.desc      清除剪切板内的文本数据项
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test35', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        await systemPasteboard.clear();
                        systemPasteboard.getPasteData().then((data) => {
                            var pasteData2 = data;
                            var recordCount = pasteData2.getRecordCount();
                            expect(recordCount).assertEqual(0);
                            done();
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test36
     * @tc.desc      清除剪切板内的uri数据项
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test36', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var uriText = 'https://www.baidu.com/';
            var pasteData = pasteboard.createUriData(uriText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        await systemPasteboard.clear();
                        systemPasteboard.getPasteData().then((data) => {
                            var pasteData2 = data;
                            var recordCount = pasteData2.getRecordCount();
                            expect(recordCount).assertEqual(0);
                            done();
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test37
     * @tc.desc      清除剪切板内的html数据项
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test37', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var htmlText = '<html><head></head><body>Hello World!</body></html>';
            var pasteData = pasteboard.createHtmlData(htmlText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        await systemPasteboard.clear();
                        systemPasteboard.getPasteData().then((data) => {
                            var pasteData2 = data;
                            var recordCount = pasteData2.getRecordCount();
                            expect(recordCount).assertEqual(0);
                            done();
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test38
     * @tc.desc      清除剪切板内的want数据项
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test38', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var myWant = {
                bundleName: "com.example.myapplication55",
                abilityName: "com.example.myapplication55.MainAbility"
            }
            var pasteData = pasteboard.createWantData(myWant);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        await systemPasteboard.clear();
                        systemPasteboard.getPasteData().then((data) => {
                            var pasteData2 = data;
                            var recordCount = pasteData2.getRecordCount();
                            expect(recordCount).assertEqual(0);
                            done();
                        });
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test39
     * @tc.desc      向剪切板内增加30条数据项，然后清除
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test39', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData0 = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData0);
            var textData = '';
            for (var i = 1; i < 30; i++) {
                textData = 'Hello World';
                textData = textData + i;
                pasteData.addTextRecord(textData);
            }
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(30);
                        await systemPasteboard.clear();
                        systemPasteboard.getPasteData().then((data) => {
                            var pasteData2 = data;
                            var recordCount = pasteData2.getRecordCount();
                            expect(recordCount).assertEqual(0);
                            done();
                        })
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test40
     * @tc.desc      向剪贴板数据各增加5条文本、uri、html数据，然后清除
     * @tc.type      Function
     * @tc.require   AR000H5I1D
     */
    it('pasteboard_function_test40', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData0 = 'Hello World0';
            var pasteData = pasteboard.createPlainTextData(textData0);
            var textData = '';
            for (var i = 1; i < 5; i++) {
                textData = 'Hello World';
                textData = textData + i;
                pasteData.addTextRecord(textData);
            }
            var htmlText = '';
            for (var i = 0; i < 5; i++) {
                htmlText = '<html><head></head><body>Hello World!</body></html>';
                htmlText = htmlText + i;
                pasteData.addHtmlRecord(htmlText);
            }
            var uriText = '';
            for (var i = 0; i < 5; i++) {
                uriText = 'https://www.baidu.com/';
                uriText = uriText + i;
                pasteData.addUriRecord(uriText);
            }
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(15);
                        await systemPasteboard.clear();
                        systemPasteboard.getPasteData().then((data) => {
                            var pasteData2 = data;
                            var recordCount = pasteData2.getRecordCount();
                            expect(recordCount).assertEqual(0);
                            done();
                        })
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test41
     * @tc.desc      更新剪贴板数据，查询剪贴板存在剪贴板数据
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test41', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var textData1 = 'Hello World1';
                        var pasteDataRecord = pasteboard.createPlainTextRecord(textData1);
                        var replace = pasteData1.replaceRecordAt(0, pasteDataRecord);
                        expect(replace).assertEqual(true);
                        await systemPasteboard.setPasteData(pasteData1);
                        systemPasteboard.hasPasteData().then(async (data) => {
                            expect(data).assertEqual(true);
                            var newData = await systemPasteboard.getPasteData();
                            expect(newData.getPrimaryText()).assertEqual(textData1);
                            var newPasteDataRecord = newData.getRecordAt(0);
                            expect(newPasteDataRecord.plainText).assertEqual(textData1);
                            done();
                        })
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test42
     * @tc.desc      删除所有的剪贴板数据，查询剪贴板不存在剪贴板数据
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test42', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        var recordCount = pasteData1.getRecordCount();
                        expect(recordCount).assertEqual(1);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        expect(pasteData1.getRecordCount()).assertEqual(0);
                        var newData = await systemPasteboard.getPasteData();
                        expect(newData.getRecordCount()).assertEqual(1);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test43
     * @tc.desc      将文本数据强制转换为文本
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test43', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello World!';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then(async (data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var pasteDataRecord = pasteData1.getRecordAt(0);
                        var text = await pasteDataRecord.convertToText();
                        expect(text).assertEqual(textData);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test44
     * @tc.desc      将一条含有特殊字符、中英混杂的文本数据强制转换为文本
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test44', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = 'Hello 中国!@#$%^&*()_+{}\?.';
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var pasteDataRecord = pasteData1.getRecordAt(0);
                        pasteDataRecord.convertToText((err, text) => {
                            if (err) {
                                console.info('f_test44 pasteDataRecord.convertToText error: ' + error);
                            } else {
                                expect(textData).assertEqual(text);
                                done();
                            }
                        });
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test45
     * @tc.desc      将一条超长文本数据 (大小为301K)强制转换为文本
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test45', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData = ''
            for (var i = 0; i < 301; i++) {
                textData = textData + "A";
            }
            var pasteData = pasteboard.createPlainTextData(textData);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        var pasteDataRecord = pasteData1.getRecordAt(0);
                        pasteDataRecord.convertToText((err, text) => {
                            if (err) {
                                console.info('f_test45 pasteDataRecord.convertToText error: ' + error);
                            } else {
                                expect(textData).assertEqual(text);
                                done();
                            }
                        });
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test46
     * @tc.desc      将uri数据强制转换为文本
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test46', 0, async function (done) {
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
                        var pasteDataRecord = pasteData1.getRecordAt(0)
                        pasteDataRecord.convertToText().then((text) => {
                            expect(uriText).assertEqual(text);
                            done();
                        }).catch((error) => {
                            console.info('f_test46 pasteDataRecord.convertToText error: ' + error);
                        });
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test47
     * @tc.desc      复制文本、uri格式
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test47', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var textData0 = 'Hello World0';
            var pasteData = pasteboard.createPlainTextData(textData0);
            var uriText = pasteboard.createUriRecord("https://www.baidu.com/");
            pasteData.addRecord(uriText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((pasteData1) => {
                        var recordCount = pasteData1.getRecordCount();
                        expect(recordCount).assertEqual(2);
                        var pasteDataRecord1 = pasteData1.getRecordAt(0);
                        var pasteDataRecord2 = pasteData1.getRecordAt(1);
                        expect(pasteDataRecord1.uri).assertEqual(uriText.uri);
                        expect(pasteDataRecord2.plainText).assertEqual(textData0);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test48
     * @tc.desc      关闭内容变化通知功能：向剪贴板数据增加、删除等html数据项
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test48', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            systemPasteboard.off('update', contentChanges);
            var htmlText = '<html><head></head><body>Hello World!</body></html>';
            var pasteData = pasteboard.createHtmlData(htmlText);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((data) => {
                        var pasteData1 = data;
                        expect(pasteData1.getRecordCount()).assertEqual(1);
                        expect(pasteData1.removeRecordAt(0)).assertEqual(true);
                        expect(pasteData1.getRecordCount()).assertEqual(0);
                        done();
                    })
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test49
     * @tc.desc      创建pixelMap
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test49', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5},
                pixelFormat: 3,
                editable: true,
                alphaType: 1,
                scaleMode: 1
            };
            image.createPixelMap(buffer, opt).then((pixelMap) => {
                expect(pixelMap.getPixelBytesNumber()).assertEqual(100);
                var pasteData = pasteboard.createPixelMapData(pixelMap);
                systemPasteboard.setPasteData(pasteData).then(() => {
                    systemPasteboard.hasPasteData().then((data) => {
                        expect(data).assertEqual(true);
                        systemPasteboard.getPasteData().then((newPasteData) => {
                            var recordCount = newPasteData.getRecordCount();
                            expect(recordCount).assertEqual(1);
                            var newPixelMap = newPasteData.getPrimaryPixelMap();
                            var PixelMapBytesNumber = newPixelMap.getPixelBytesNumber();
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
    })

    /**
     * @tc.name      pasteboard_function_test50
     * @tc.desc      创建kv Record
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test50', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var dataXml = new ArrayBuffer(512);
            var int32view = new Int32Array(dataXml);
            for (var i = 0; i < int32view.length; i++) {
                int32view[i] = 65535 + i;
            }
            var pasteDataRecord = pasteboard.createRecord('app/xml', dataXml);
            var dataJpg = new ArrayBuffer(256);
            pasteDataRecord.data['image/ipg'] = dataJpg;
            var pasteData = pasteboard.createHtmlData('application/xml');
            var replace = pasteData.replaceRecordAt(0, pasteDataRecord);
            expect(replace).assertEqual(true);
            systemPasteboard.setPasteData(pasteData).then(() => {
                systemPasteboard.hasPasteData().then((data) => {
                    expect(data).assertEqual(true);
                    systemPasteboard.getPasteData().then((newPasteData) => {
                        var recordCount = newPasteData.getRecordCount();
                        expect(recordCount).assertEqual(1);
                        var newPasteDataRecord = newPasteData.getRecordAt(0);
                        var newAppXml = newPasteDataRecord.data['app/xml'];
                        var newImageIpg = newPasteDataRecord.data['image/ipg'];
                        expect(newAppXml.byteLength === 512 && newImageIpg.byteLength === 256).assertEqual(true);
                        var newAppXmlView = new Int32Array(newAppXml);
                        var newImageIpgView = new Int32Array(newImageIpg);
                        for (var i = 0; i < newAppXmlView.length; i++) {
                            console.info("newAppXml[" + i + "] = " + newAppXmlView[i]);
                        }
                        for (var i = 0; i < newImageIpgView.length; i++) {
                            console.info("newImageIpg[" + i + "] = " + newImageIpg[i]);
                        }
                        done();
                    });
                });
            });
        });
    })

    /**
     * @tc.name      pasteboard_function_test51
     * @tc.desc      测试addPixelMapRecord
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test51', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5},
                pixelFormat: 3,
                editable: true,
                alphaType: 1,
                scaleMode: 1
            };
            var pasteData = pasteboard.createHtmlData('application/xml');
            image.createPixelMap(buffer, opt).then((pixelMap) => {
                expect(pixelMap.getPixelBytesNumber() === 100).assertEqual(true);
                pasteData.addPixelMapRecord(pixelMap);
                systemPasteboard.setPasteData(pasteData).then(() => {
                    systemPasteboard.hasPasteData().then((data) => {
                        expect(data).assertEqual(true);
                        systemPasteboard.getPasteData().then((newPasteData) => {
                            var recordCount = newPasteData.getRecordCount();
                            expect(recordCount).assertEqual(2);
                            var newPixelMap = newPasteData.getPrimaryPixelMap();
                            var PixelMapBytesNumber = newPixelMap.getPixelBytesNumber();
                            expect(PixelMapBytesNumber).assertEqual(100);
                            var newHtmlData = newPasteData.getRecordAt(1);
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
    })

    /**
     * @tc.name      pasteboard_function_test52
     * @tc.desc      测试异步callback调用+createPlainTextData
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test52', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test52: systemPasteboard.clear callback error:' + err);
            } else {
                var textData = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test52: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test52: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test52: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var primaryText = data.getPrimaryText();
                                        expect(primaryText).assertEqual(textData);
                                        expect(pasteboard.MAX_RECORD_NUM).assertEqual(128);
                                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_PLAIN)).assertEqual(true);
                                        expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test53
     * @tc.desc      测试异步callback调用+createHtmlData
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test53', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test53: systemPasteboard.clear callback error:' + err);
            } else {
                var htmlText = '<html><head></head><body>Hello World!</body></html>';
                var pasteData = pasteboard.createHtmlData(htmlText);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test53: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test53: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test53: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var PrimaryHtml = data.getPrimaryHtml();
                                        console.info('f_test53: PrimaryHtml = ' + PrimaryHtml);
                                        expect(PrimaryHtml).assertEqual(htmlText);
                                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
                                        expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test54
     * @tc.desc      测试异步callback调用+createHtmlData,htmlText = ''.
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test54', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test54: systemPasteboard.clear callback error:' + err);
            } else {
                var htmlText = '';
                var pasteData = pasteboard.createHtmlData(htmlText);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test54: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test54: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test54: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var PrimaryHtml = data.getPrimaryHtml();
                                        console.info('f_test54: PrimaryHtml = ' + PrimaryHtml);
                                        expect(PrimaryHtml).assertEqual(htmlText);
                                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_HTML)).assertEqual(true);
                                        expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test55
     * @tc.desc      测试异步callback调用+createUriData,uriText = ''.
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test55', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test55: systemPasteboard.clear callback error:' + err);
            } else {
                var uriText = '';
                var pasteData = pasteboard.createUriData(uriText);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test55: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test55: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test55: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var PrimaryUri = data.getPrimaryUri();
                                        console.info('f_test55: PrimaryUri = ' + PrimaryUri);
                                        expect(PrimaryUri).assertEqual(uriText);
                                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
                                        expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_URI);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test56
     * @tc.desc      测试异步callback调用+createWantData.
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test56', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test56: systemPasteboard.clear callback error:' + err);
            } else {
                var want = {
                    bundleName: "com.example.myapplication8",
                    abilityName: "com.example.myapplication8.MainAbility"
                };
                var pasteData = pasteboard.createWantData(want);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test56: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test56: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test56: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var PrimaryWant = data.getPrimaryWant();
                                        expect(PrimaryWant.bundleName).assertEqual(want.bundleName);
                                        expect(PrimaryWant.abilityName).assertEqual(want.abilityName);
                                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_WANT)).assertEqual(true);
                                        expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_WANT);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test57
     * @tc.desc      测试异步callback调用+addTextRecord.
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test57', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test57: systemPasteboard.clear callback error:' + err);
            } else {
                var textData0 = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData0);
                var textData1 = 'Hello World1';
                pasteData.addTextRecord(textData1);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test57: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test57: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test57: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(2);
                                        var PrimaryText = data.getPrimaryText();
                                        expect(PrimaryText).assertEqual(textData1);
                                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_PLAIN)).assertEqual(true);
                                        expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test58
     * @tc.desc      测试异步callback调用+添加15条textRecord
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test58', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test58: systemPasteboard.clear callback error:' + err);
            } else {
                var textData0 = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData0);
                var textData = '';
                for (var i = 1; i < 15; i++) {
                    textData = 'Hello World';
                    textData = textData + i;
                    pasteData.addTextRecord(textData);
                }
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test58: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test58: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test58: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(15);
                                        var PrimaryText = data.getPrimaryText();
                                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_PLAIN)).assertEqual(true);
                                        expect(data.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
                                        expect(PrimaryText).assertEqual('Hello World14');
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test59
     * @tc.desc      测试异步callback调用+addHtmlRecord+addUriRecord+addWantRecord
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test59', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test59: systemPasteboard.clear callback error:' + err);
            } else {
                var textData = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData);
                var htmlText = '<html><head></head><body>Hello World!</body></html>';
                pasteData.addHtmlRecord(htmlText);
                var uriText = '';
                pasteData.addUriRecord(uriText);
                var want = {
                    bundleName: "com.example.myapplication8",
                    abilityName: "com.example.myapplication8.MainAbility"
                }
                pasteData.addWantRecord(want);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test59: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test59: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test59: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(4);
                                        var MimeTypes = data.getMimeTypes();
                                        expect(MimeTypes[0]).assertEqual(pasteboard.MIMETYPE_TEXT_WANT);
                                        expect(MimeTypes[1]).assertEqual(pasteboard.MIMETYPE_TEXT_URI);
                                        expect(MimeTypes[2]).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
                                        expect(MimeTypes[3]).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test60
     * @tc.desc      测试异步callback调用+addHtmlRecord+addUriRecord+removeRecordAt
     * @tc.type      Function
     * @tc.require   AR000HEECD
     */
    it('pasteboard_function_test60', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test60: systemPasteboard.clear callback error:' + err);
            } else {
                var textData = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData);
                var htmlText = '<html><head></head><body>Hello World!</body></html>';
                pasteData.addHtmlRecord(htmlText);
                var uriText = '';
                pasteData.addUriRecord(uriText);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test60: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test60: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test60: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(3);
                                        expect(data.removeRecordAt(0)).assertEqual(true);
                                        expect(data.getRecordCount()).assertEqual(2);
                                        systemPasteboard.setPasteData(data, (err, newdata) => {
                                            if (err) {
                                                console.error('f_test60: systemPasteboard.setPasteData callback error:' + err);
                                            } else {
                                                systemPasteboard.getPasteData((err, data) => {
                                                    if (err) {
                                                        console.error('f_test60: systemPasteboard.getPasteData callback error:' + err);
                                                    } else {
                                                        expect(data.getRecordCount()).assertEqual(2);
                                                        done();
                                                    }
                                                });
                                            }
                                        });
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test61
     * @tc.desc      测试异步callback调用+添加30条textRecord
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test61', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test61: systemPasteboard.clear callback error:' + err);
            } else {
                var textData0 = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData0);
                var textData = '';
                for (var i = 1; i < 30; i++) {
                    textData = 'Hello World';
                    textData = textData + i;
                    pasteData.addTextRecord(textData);
                }
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test61: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test61: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test61: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(30);
                                        for (var i = 0; i < 30; i++) {
                                            expect(data.removeRecordAt(0)).assertEqual(true);
                                        }
                                        expect(data.getRecordCount()).assertEqual(0);
                                        systemPasteboard.setPasteData(data, (err, newdata) => {
                                            if (err) {
                                                console.error('f_test61: systemPasteboard.setPasteData callback error:' + err);
                                            } else {
                                                systemPasteboard.getPasteData((err, data) => {
                                                    if (err) {
                                                        console.error('f_test61: systemPasteboard.getPasteData callback error:' + err);
                                                    } else {
                                                        expect(data.getRecordCount()).assertEqual(0);
                                                        done();
                                                    }
                                                });
                                            }
                                        });
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test62
     * @tc.desc      测试异步callback调用+替代text类型Record
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test62', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test62: systemPasteboard.clear callback error:' + err);
            } else {
                var textData = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test62: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test62: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test62: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var textData1 = 'Hello World1';
                                        var pasteDataRecord = pasteboard.createPlainTextRecord(textData1);
                                        var replace = data.replaceRecordAt(0, pasteDataRecord);
                                        expect(replace).assertEqual(true);
                                        var primaryText = data.getPrimaryText();
                                        expect(primaryText).assertEqual(textData1);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test63
     * @tc.desc      测试异步callback调用+替代html类型Record
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test63', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test63: systemPasteboard.clear callback error:' + err);
            } else {
                var htmlText = '<html><head></head><body>Hello World!</body></html>';
                var pasteData = pasteboard.createHtmlData(htmlText);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test63: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test63: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test63: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var htmlText1 = '<html><head></head><body>Hello World 1</body></html>';
                                        var pasteDataRecord = pasteboard.createHtmlTextRecord(htmlText1);
                                        var replace = data.replaceRecordAt(0, pasteDataRecord);
                                        expect(replace).assertEqual(true);
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var primaryHtml = data.getPrimaryHtml();
                                        expect(primaryHtml).assertEqual(htmlText1);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test64
     * @tc.desc      测试异步callback调用+替代want类型Record
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test64', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test64: systemPasteboard.clear callback error:' + err);
            } else {
                var wantText0 = {
                    bundleName: "com.example.myapplication3",
                    abilityName: "com.example.myapplication3.MainAbility"
                };
                var pasteData = pasteboard.createWantData(wantText0);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test64: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test64: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test64: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var wantText1 = {
                                            bundleName: "com.example.myapplication30",
                                            abilityName: "com.example.myapplication30.MainAbility"
                                        };
                                        var pasteDataRecord = pasteboard.createWantRecord(wantText1);
                                        var replace = data.replaceRecordAt(0, pasteDataRecord);
                                        expect(replace).assertEqual(true);
                                        var primaryWant = data.getPrimaryWant();
                                        expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_WANT)).assertEqual(true);
                                        expect(primaryWant.bundleName).assertEqual(wantText1.bundleName);
                                        expect(primaryWant.abilityName).assertEqual(wantText1.abilityName);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test65
     * @tc.desc      测试异步callback调用+clear want类型的pasteData后get
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test65', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test65: systemPasteboard.clear callback error:' + err);
            } else {
                var wantText0 = {
                    bundleName: "com.example.myapplication3",
                    abilityName: "com.example.myapplication3.MainAbility"
                };
                var pasteData = pasteboard.createWantData(wantText0);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test65: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test65: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test65: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        systemPasteboard.clear((err, data) => {
                                            if (err) {
                                                console.error('f_test65: systemPasteboard.clear callback error:' + err);
                                            } else {
                                                systemPasteboard.getPasteData((err, data) => {
                                                    if (err) {
                                                        console.error('f_test65: systemPasteboard.getPasteData callback error:' + err);
                                                    } else {
                                                        expect(data.getRecordCount()).assertEqual(0);
                                                        done();
                                                    }
                                                });
                                            }
                                        });
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test66
     * @tc.desc      测试异步callback调用+getProperty+setProperty
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test66', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test66: systemPasteboard.clear callback error:' + err);
            } else {
                var textData = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test66: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test66: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test66: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var pasteDataProperty = data.getProperty();
                                        expect(pasteDataProperty.shareOption).assertEqual(pasteboard.ShareOption.CrossDevice);
                                        pasteDataProperty.shareOption = pasteboard.ShareOption.InApp;
                                        data.setProperty(pasteDataProperty);
                                        var pasteDataProperty1 = data.getProperty();
                                        expect(pasteDataProperty1.shareOption).assertEqual(pasteboard.ShareOption.InApp);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test67
     * @tc.desc      测试异步callback调用+打开剪切板内容变化回调
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test67', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test67: systemPasteboard.clear callback error:' + err);
            } else {
                systemPasteboard.on('update', contentChanges);
                var textData = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test67: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test67: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data === true).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test67: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        expect(data.removeRecordAt(0)).assertEqual(true);
                                        expect(data.getRecordCount()).assertEqual(0);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test68
     * @tc.desc      测试异步callback调用+关闭剪切板内容变化回调
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test68', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test68: systemPasteboard.clear callback error:' + err);
            } else {
                systemPasteboard.off('update', contentChanges);
                var textData = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test68: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test68: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test68: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        expect(data.removeRecordAt(0)).assertEqual(true);
                                        expect(data.getRecordCount()).assertEqual(0);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test69
     * @tc.desc      promise方式+createPixelMapRecord
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test69', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear().then(() => {
            var buffer = new ArrayBuffer(128);
            var opt = {
                size: {height: 5, width: 5},
                pixelFormat: 3,
                editable: true,
                alphaType: 1,
                scaleMode: 1
            };
            var pasteData = pasteboard.createHtmlData('application/xml');
            image.createPixelMap(buffer, opt).then((pixelMap) => {
                expect(pixelMap.getPixelBytesNumber()).assertEqual(100);
                var pixelMapRecord = pasteboard.createPixelMapRecord(pixelMap);
                pasteData.addRecord(pixelMapRecord);
                systemPasteboard.setPasteData(pasteData).then(() => {
                    systemPasteboard.hasPasteData().then((data) => {
                        expect(data).assertEqual(true);
                        systemPasteboard.getPasteData().then((newPasteData) => {
                            var recordCount = newPasteData.getRecordCount();
                            expect(recordCount).assertEqual(2);
                            var newPixelMap = newPasteData.getPrimaryPixelMap();
                            var PixelMapBytesNumber = newPixelMap.getPixelBytesNumber();
                            expect(PixelMapBytesNumber).assertEqual(100);
                            var newHtmlData = newPasteData.getRecordAt(1);
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
    })

    /**
     * @tc.name      pasteboard_function_test70
     * @tc.desc      测试异步callback调用+分别添加5条plainText、htmlText、uriText类型的记录
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test70', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test70: systemPasteboard.clear callback error:' + err);
            } else {
                var textData0 = 'Hello World0';
                var pasteData = pasteboard.createPlainTextData(textData0);
                var textData = '';
                for (var i = 1; i < 5; i++) {
                    textData = 'Hello World';
                    textData = textData + i;
                    pasteData.addTextRecord(textData);
                }
                var htmlText = '';
                for (var i = 0; i < 5; i++) {
                    htmlText = '<html><head></head><body>Hello World!</body></html>';
                    htmlText = htmlText + i;
                    pasteData.addHtmlRecord(htmlText);
                }
                var uriText = '';
                for (var i = 0; i < 5; i++) {
                    uriText = 'https://www.baidu.com/';
                    uriText = uriText + i;
                    pasteData.addUriRecord(uriText);
                }
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test70: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test70: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData(async (err, data) => {
                                    if (err) {
                                        console.error('f_test70: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(15);
                                        await systemPasteboard.clear();
                                        var newData = await systemPasteboard.getPasteData();
                                        expect(newData.getRecordCount()).assertEqual(0);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test71
     * @tc.desc      promise方式调用convertToText
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test71', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test71: systemPasteboard.clear callback error:' + err);
            } else {
                var textData = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test71: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test71: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData(async (err, data) => {
                                    if (err) {
                                        console.error('f_test71: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var dataRecord = data.getRecordAt(0);
                                        var text = await dataRecord.convertToText();
                                        expect(text).assertEqual(textData);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test72
     * @tc.desc      callback方式调用convertToText
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test72', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test72: systemPasteboard.clear callback error:' + err);
            } else {
                var textData = 'Hello World!';
                var pasteData = pasteboard.createPlainTextData(textData);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test72: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test72: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData((err, data) => {
                                    if (err) {
                                        console.error('f_test72: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var dataRecord = data.getRecordAt(0);
                                        dataRecord.convertToText((err, data) => {
                                            if (err) {
                                                console.error('f_test72: PasteDataRecord.convertToText callback error:' + err);
                                            } else {
                                                expect(data).assertEqual(textData);
                                                done();
                                            }
                                        });
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test73
     * @tc.desc      promise方式调用convertToText
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test73', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test73: systemPasteboard.clear callback error:' + err);
            } else {
                var textData = 'Hello 中国!@#$%^&*()_+{}\\?.';
                var pasteData = pasteboard.createPlainTextData(textData);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test73: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test73: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData(async (err, data) => {
                                    if (err) {
                                        console.error('f_test73: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var dataRecord = data.getRecordAt(0);
                                        var text = await dataRecord.convertToText();
                                        expect(text).assertEqual(textData);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     * @tc.name      pasteboard_function_test74
     * @tc.desc      promise方式调用convertToText
     * @tc.type      Function
     * @tc.require   AR000H5GKU
     */
    it('pasteboard_function_test74', 0, async function (done) {
        var systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.clear((err, data) => {
            if (err) {
                console.error('f_test74: systemPasteboard.clear callback error:' + err);
            } else {
                var uriText = 'https://www.baidu.com/';
                var pasteData = pasteboard.createUriData(uriText);
                systemPasteboard.setPasteData(pasteData, (err, data) => {
                    if (err) {
                        console.error('f_test74: systemPasteboard.setPasteData callback error:' + err);
                    } else {
                        systemPasteboard.hasPasteData((err, data) => {
                            if (err) {
                                console.error('f_test74: systemPasteboard.hasPasteData callback error:' + err);
                            } else {
                                expect(data).assertEqual(true);
                                systemPasteboard.getPasteData(async (err, data) => {
                                    if (err) {
                                        console.error('f_test74: systemPasteboard.getPasteData callback error:' + err);
                                    } else {
                                        expect(data.getRecordCount()).assertEqual(1);
                                        var dataRecord = data.getRecordAt(0);
                                        var text = await dataRecord.convertToText();
                                        expect(text).assertEqual(uriText);
                                        done();
                                    }
                                });
                            }
                        });
                    }
                });
            }
        });
    })

    /**
     *  The callback function is used for pasteboard content changes
     */
    function contentChanges() {
        console.info('#EVENT: The content is changed in the pasteboard');
    }
});
