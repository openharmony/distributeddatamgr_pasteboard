/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index';
import pasteboard from '@ohos.pasteboard';
import UDC from '@ohos.data.unifiedDataChannel';
import UTD from '@ohos.data.uniformTypeDescriptor'
import image from '@ohos.multimedia.image';

const KEY_TEST_ELEMENT = 'TestKey';
const VALUE_TEST_ELEMENT = 'TestValue';
const TEST_BUNDLE_NAME = 'MyBundleName';
const TEST_ID = 123456;
const TEST_ABILITY_NAME = 'MyAbilityName';

let U8_ARRAY = new Uint8Array([1, 2, 3, 4, 5, 6, 7, 8, 9, 10]);

var textData = new UDC.UnifiedData();
var plainTextData = new UDC.UnifiedData();
var hyperlinkData = new UDC.UnifiedData();
var htmlData = new UDC.UnifiedData();
var fileData = new UDC.UnifiedData();
var folderData = new UDC.UnifiedData();
var imageData = new UDC.UnifiedData();
var videoData = new UDC.UnifiedData();
var audioData = new UDC.UnifiedData();
var systemDefinedFormData = new UDC.UnifiedData();
var systemDefinedAppItemData = new UDC.UnifiedData();
var applicationDefinedRecordData = new UDC.UnifiedData();
var wantData = new UDC.UnifiedData();
var pixelMapData = new UDC.UnifiedData();

describe('PasteBoardUnifiedDataSyncJSTest', function () {
    beforeAll(async function () {
        console.info('beforeAll');
    });

    afterAll(async function () {
        console.info('afterAll');
    });

    beforeEach(async function () {
        console.info('beforeEach');
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.clearData();
    });

    afterEach(async function () {
        console.info('afterEach');
    });

    function getTextData() {
        let text = new UDC.Text();
        text.details = {
            Key: 'text' + KEY_TEST_ELEMENT,
            Value: 'text' + VALUE_TEST_ELEMENT,
        };
        textData.addRecord(text);
        return textData;
    }

    function getPlainTextData() {
        let plainText = new UDC.PlainText();
        plainText.details = {
            Key: 'plainText' + KEY_TEST_ELEMENT,
            Value: 'plainText' + VALUE_TEST_ELEMENT,
        };
        plainText.textContent = 'textContent';
        plainText.abstract = 'abstract';
        plainTextData.addRecord(plainText);
        return plainTextData;
    }

    function getHyperlinkData() {
        let link = new UDC.Hyperlink();
        link.details = {
            Key: 'hyperLink' + KEY_TEST_ELEMENT,
            Value: 'hyperLink' + VALUE_TEST_ELEMENT,
        };
        link.url = 'url';
        link.description = 'description';
        hyperlinkData.addRecord(link);
        return hyperlinkData;
    }

    function getHtmlData() {
        let html = new UDC.HTML();
        html.details = {
            Key: 'html' + KEY_TEST_ELEMENT,
            Value: 'html' + VALUE_TEST_ELEMENT,
        };
        html.htmlContent = 'htmlContent';
        html.plainContent = 'plainContent';
        htmlData.addRecord(html);
        return htmlData;
    }

    function getFileData() {
        let file = new UDC.File();
        file.details = {
            Key: 'file' + KEY_TEST_ELEMENT,
            Value: 'file' + VALUE_TEST_ELEMENT,
        };
        file.uri = 'uri';
        fileData.addRecord(file);
        return fileData;
    }

    function getFolderData() {
        let folder = new UDC.Folder();
        folder.details = {
            Key: 'folder' + KEY_TEST_ELEMENT,
            Value: 'folder' + VALUE_TEST_ELEMENT,
        };
        folder.uri = 'folderUri';
        folderData.addRecord(folder);
        return folderData;
    }

    function getImageData() {
        let image = new UDC.Image();
        image.details = {
            Key: 'image' + KEY_TEST_ELEMENT,
            Value: 'image' + VALUE_TEST_ELEMENT,
        };
        image.imageUri = 'imageUri';
        imageData.addRecord(image);
        return imageData;
    }

    function getVideoData() {
        let video = new UDC.Video();
        video.details = {
            Key: 'video' + KEY_TEST_ELEMENT,
            Value: 'video' + VALUE_TEST_ELEMENT,
        };
        video.videoUri = 'videoUri';
        videoData.addRecord(video);
        return videoData;
    }

    function getAudioData() {
        let audio = new UDC.Audio();
        audio.details = {
            Key: 'audio' + KEY_TEST_ELEMENT,
            Value: 'audio' + VALUE_TEST_ELEMENT,
        };
        audio.audioUri = 'audioUri';
        audioData.addRecord(audio);
        return audioData;
    }

    function getSystemDefinedAppItemDataData() {
        let appItem = new UDC.SystemDefinedAppItem();
        appItem.appId = 'MyAppId';
        appItem.appName = 'MyAppName';
        appItem.abilityName = TEST_ABILITY_NAME;
        appItem.bundleName = TEST_BUNDLE_NAME;
        appItem.appIconId = 'MyAppIconId';
        appItem.appLabelId = 'MyAppLabelId';
        appItem.details = {
            appItemKey1: 1,
            appItemKey2: 'appItem' + VALUE_TEST_ELEMENT,
            appItemKey3: U8_ARRAY,
        };
        systemDefinedAppItemData.addRecord(appItem);
        return systemDefinedAppItemData;
    }

    function getApplicationDefinedRecordData() {
        let applicationDefinedRecord = new UDC.ApplicationDefinedRecord();
        applicationDefinedRecord.applicationDefinedType = 'applicationDefinedType';
        applicationDefinedRecord.rawData = U8_ARRAY;
        applicationDefinedRecordData.addRecord(applicationDefinedRecord);
        return applicationDefinedRecordData;
    }

    function getWantData() {
        let object = {
            bundleName: 'bundleName',
            abilityName: 'abilityName'
        }
        let wantRecord = new UDC.UnifiedRecord(UTD.UniformDataType.OPENHARMONY_WANT, object);
        wantData.addRecord(wantRecord);
        return wantData;
    }

    function getPixelMapData() {
        const buffer = new ArrayBuffer(128);
        const opt = {
            size: { height: 5, width: 5 },
            pixelFormat: 3,
            editable: true,
            alphaType: 1,
            scaleMode: 1,
        };
        const pixelMap = image.createPixelMapSync(buffer, opt);
        let pixelMapRecord = new UDC.UnifiedRecord(UTD.UniformDataType.OPENHARMONY_PIXEL_MAP, pixelMap);
        pixelMapData.addRecord(pixelMapRecord);
        return pixelMapData;
    }

    /**
     * @tc.name TextTestSync001
     * @tc.desc Test Unified Record of Text
     * @tc.type FUNC
     */
    it('TextTestSync001', 0, async function (done) {
        getTextData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(textData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('text' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('text' + VALUE_TEST_ELEMENT);

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(outputData.getPrimaryMimeType()).assertEqual('general.text');
            done();
        });
        console.info('TextTestSync001 end');
    });

    /**
     * @tc.name PlainTextTestSync001
     * @tc.desc Test Unified Record of Plain Text
     * @tc.type FUNC
     */
    it('PlainTextTestSync001', 0, async function (done) {
        console.info('PlainTextTestSync001 begin');
        getPlainTextData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(plainTextData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('plainText' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('plainText' + VALUE_TEST_ELEMENT);
        expect(records[0].textContent).assertEqual('textContent');
        expect(records[0].abstract).assertEqual('abstract');

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(outputData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
            const primaryText = outputData.getPrimaryText();
            expect(primaryText).assertEqual('textContent');
            done();
        });
        console.info('PlainTextTestSync001 end');
    });

    /**
     * @tc.name HyperlinkTestSync001
     * @tc.desc Test Unified Record of Hyper Link
     * @tc.type FUNC
     */
    it('HyperlinkTestSync001', 0, async function (done) {
        getHyperlinkData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(hyperlinkData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('hyperLink' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('hyperLink' + VALUE_TEST_ELEMENT);
        expect(records[0].url).assertEqual('url');
        expect(records[0].description).assertEqual('description');

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(outputData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
            const primaryText = outputData.getPrimaryText();
            expect(primaryText).assertEqual('url');
            done();
        });
        console.info('HyperlinkTestSync001 end');
    });

    /**
     * @tc.name HtmlTestSync001
     * @tc.desc Test Unified Record of Html
     * @tc.type FUNC
     */
    it('HtmlTestSync001', 0, async function (done) {
        getHtmlData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(htmlData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('html' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('html' + VALUE_TEST_ELEMENT);
        expect(records[0].htmlContent).assertEqual('htmlContent');
        expect(records[0].plainContent).assertEqual('plainContent');
        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(outputData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
            const primaryHtml = outputData.getPrimaryHtml();
            expect(primaryHtml).assertEqual('htmlContent');
            done();
        });
        console.info('HtmlTestSync001 end');
    });

    /**
     * @tc.name FileTestSync001
     * @tc.desc Test Unified Record of File
     * @tc.type FUNC
     */
    it('FileTestSync001', 0, async function (done) {
        console.info('FileTestSync001 begin');
        getFileData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(fileData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('file' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('file' + VALUE_TEST_ELEMENT);
        expect(records[0].uri).assertEqual('uri');

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
            const primaryUri = data.getPrimaryUri();
            expect(primaryUri).assertEqual('uri');
            done();
        });
        console.info('FileTestSync001 end');
    });

    /**
     * @tc.name FolderTestSync001
     * @tc.desc Test Unified Record of Folder
     * @tc.type FUNC
     */
    it('FolderTestSync001', 0, async function (done) {
        console.info('FolderTestSync001 begin');
        getFolderData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(folderData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('folder' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('folder' + VALUE_TEST_ELEMENT);
        expect(records[0].uri).assertEqual('folderUri');

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
            const primaryUri = data.getPrimaryUri();
            expect(primaryUri).assertEqual('folderUri');
            done();
        });
        console.info('FolderTestSync001 end');
    });

    /**
     * @tc.name ImageTestSync001
     * @tc.desc Test Unified Record of Image
     * @tc.type FUNC
     */
    it('ImageTestSync001', 0, async function (done) {
        console.info('ImageTestSync001 begin');
        getImageData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(imageData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('image' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('image' + VALUE_TEST_ELEMENT);
        expect(records[0].imageUri).assertEqual('imageUri');

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
            const primaryUri = data.getPrimaryUri();
            expect(primaryUri).assertEqual('imageUri');
            done();
        });
        console.info('ImageTestSync001 end');
    });

    /**
     * @tc.name VideoTestSync001
     * @tc.desc Test Unified Record of Video
     * @tc.type FUNC
     */
    it('VideoTestSync001', 0, async function (done) {
        console.info('VideoTestSync001 begin');
        getVideoData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(videoData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('video' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('video' + VALUE_TEST_ELEMENT);
        expect(records[0].videoUri).assertEqual('videoUri');

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
            const primaryUri = data.getPrimaryUri();
            expect(primaryUri).assertEqual('videoUri');
            done();
        });
        console.info('VideoTestSync001 end');
    });

    /**
     * @tc.name AudioTestSync001
     * @tc.desc Test Unified Record of Audio
     * @tc.type FUNC
     */
    it('AudioTestSync001', 0, async function (done) {
        console.info('AudioTestSync001 begin');
        getAudioData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(audioData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('audio' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('audio' + VALUE_TEST_ELEMENT);
        expect(records[0].audioUri).assertEqual('audioUri');

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(data.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
            const primaryUri = data.getPrimaryUri();
            expect(primaryUri).assertEqual('audioUri');
            done();
        });
        console.info('AudioTestSync001 end');
    });

    /**
     * @tc.name SystemDefinedAppItemTestSync001
     * @tc.desc Test Unified Record of SystemDefinedAppItem
     * @tc.type FUNC
     */
    it('SystemDefinedAppItemTestSync001', 0, async function (done) {
        console.info('SystemDefinedAppItemTestSync001 begin');
        getSystemDefinedAppItemDataData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(systemDefinedAppItemData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.appItemKey1).assertEqual(1);
        expect(records[0].details.appItemKey2).assertEqual('appItem' + VALUE_TEST_ELEMENT);
        for (let i = 0; i < U8_ARRAY.length; i++) {
            expect(records[0].details.appItemKey3[i]).assertEqual(U8_ARRAY[i]);
        }
        expect(records[0].appId).assertEqual('MyAppId');
        expect(records[0].appName).assertEqual('MyAppName');
        expect(records[0].abilityName).assertEqual(TEST_ABILITY_NAME);
        expect(records[0].bundleName).assertEqual(TEST_BUNDLE_NAME);
        expect(records[0].appIconId).assertEqual('MyAppIconId');
        expect(records[0].appLabelId).assertEqual('MyAppLabelId');

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(outputData.getPrimaryMimeType()).assertEqual('openharmony.app-item');
            done();
        });
        console.info('SystemDefinedAppItemTestSync001 end');
    });

    /**
     * @tc.name ApplicationDefinedRecordTestSync001
     * @tc.desc Test Unified Record of ApplicationDefinedRecord
     * @tc.type FUNC
     */
    it('ApplicationDefinedRecordTestSync001', 0, async function (done) {
        console.info('ApplicationDefinedRecordTestSync001 begin');
        getApplicationDefinedRecordData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(applicationDefinedRecordData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].applicationDefinedType).assertEqual('applicationDefinedType');
        for (let i = 0; i < U8_ARRAY.length; i++) {
            expect(records[0].rawData[i]).assertEqual(U8_ARRAY[i]);
        }
        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(outputData.getPrimaryMimeType()).assertEqual('applicationDefinedType');
            done();
        });
        console.info('ApplicationDefinedRecordTestSync001 end');
    });

    /**
     * @tc.name WantTestSync001
     * @tc.desc Test Unified Record of Want
     * @tc.type FUNC
     */
    it('WantTestSync001', 0, async function (done) {
        console.info('WantTestSync001 begin');
        getWantData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(wantData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(outputData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_WANT);
            done();
        });
        console.info('WantTestSync001 end');
    });

    /**
     * @tc.name PixelMapTestSync001
     * @tc.desc Test Unified Record of PixelMap
     * @tc.type FUNC
     */
    it('PixelMapTestSync001', 0, async function (done) {
        console.info('PixelMapTestSync001 begin');
        getPixelMapData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setUnifiedDataSync(pixelMapData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        const outputData = systemPasteboard.getUnifiedDataSync();
        let records = outputData.getRecords();
        expect(records.length).assertEqual(1);

        systemPasteboard.getData().then((data) => {
            const outputData = data;
            expect(outputData.getRecordCount()).assertEqual(1);
            expect(outputData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_PIXELMAP);
            done();
        });
        console.info('PixelMapTestSync001 end');
    });
});