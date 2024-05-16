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
var systemDefinedAppItemData = new UDC.UnifiedData();
var applicationDefinedRecordData = new UDC.UnifiedData();
var wantData = new UDC.UnifiedData();
var pixelMapData = new UDC.UnifiedData();

describe('PasteBoardUnifiedDataJSTest', function () {
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
     * @tc.name TextTest001
     * @tc.desc Test Unified Record of Text
     * @tc.type FUNC
     */
    it('TextTest001', 0, async function (done) {
        console.info('TextTest001 begin');
        getTextData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(textData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('text' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('text' + VALUE_TEST_ELEMENT);
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.getPrimaryMimeType()).assertEqual('general.text');
        done();
        console.info('TextTest001 end');
    });

    /**
     * @tc.name PlainTextTest001
     * @tc.desc Test Unified Record of Plain Text
     * @tc.type FUNC
     */
    it('PlainTextTest001', 0, async function (done) {
        console.info('PlainTextTest001 begin');
        getPlainTextData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(plainTextData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('plainText' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('plainText' + VALUE_TEST_ELEMENT);
        expect(records[0].textContent).assertEqual('textContent');
        expect(records[0].abstract).assertEqual('abstract');
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
        const primaryText = pasteData.getPrimaryText();
        expect(primaryText).assertEqual('textContent');
        done();
        console.info('PlainTextTest001 end');
    });

    /**
     * @tc.name HyperlinkTest001
     * @tc.desc Test Unified Record of Hyper Link
     * @tc.type FUNC
     */
    it('HyperlinkTest001', 0, async function (done) {
        console.info('HyperlinkTest001 begin');
        getHyperlinkData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(hyperlinkData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('hyperLink' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('hyperLink' + VALUE_TEST_ELEMENT);
        expect(records[0].url).assertEqual('url');
        expect(records[0].description).assertEqual('description');
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_PLAIN);
        const primaryText = pasteData.getPrimaryText();
        expect(primaryText).assertEqual('url');
        done();
        console.info('HyperlinkTest001 end');
    });

    /**
     * @tc.name HtmlTest001
     * @tc.desc Test Unified Record of Html
     * @tc.type FUNC
     */
    it('HtmlTest001', 0, async function (done) {
        console.info('HtmlTest001 begin');
        getHtmlData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(htmlData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('html' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('html' + VALUE_TEST_ELEMENT);
        expect(records[0].htmlContent).assertEqual('htmlContent');
        expect(records[0].plainContent).assertEqual('plainContent');
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_HTML);
        const primaryHtml = pasteData.getPrimaryHtml();
        expect(primaryHtml).assertEqual('htmlContent');
        done();
        console.info('HtmlTest001 end');
    });

    /**
     * @tc.name FileTest001
     * @tc.desc Test Unified Record of File
     * @tc.type FUNC
     */
    it('FileTest001', 0, async function (done) {
        console.info('FileTest001 begin');
        getFileData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(fileData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('file' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('file' + VALUE_TEST_ELEMENT);
        expect(records[0].uri).assertEqual('uri');
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
        const primaryUri = pasteData.getPrimaryUri();
        expect(primaryUri).assertEqual('uri');
        done();
        console.info('FileTest001 end');
    });

    /**
     * @tc.name FolderTest001
     * @tc.desc Test Unified Record of Folder
     * @tc.type FUNC
     */
    it('FolderTest001', 0, async function (done) {
        console.info('FolderTest001 begin');
        getFolderData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(folderData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('folder' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('folder' + VALUE_TEST_ELEMENT);
        expect(records[0].uri).assertEqual('folderUri');
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
        const primaryUri = pasteData.getPrimaryUri();
        expect(primaryUri).assertEqual('folderUri');
        done();
        console.info('FolderTest001 end');
    });

    /**
     * @tc.name ImageTest001
     * @tc.desc Test Unified Record of Image
     * @tc.type FUNC
     */
    it('ImageTest001', 0, async function (done) {
        console.info('ImageTest001 begin');
        getImageData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(imageData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('image' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('image' + VALUE_TEST_ELEMENT);
        expect(records[0].imageUri).assertEqual('imageUri');
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
        const primaryUri = pasteData.getPrimaryUri();
        expect(primaryUri).assertEqual('imageUri');
        done();
        console.info('ImageTest001 end');
    });

    /**
     * @tc.name VideoTest001
     * @tc.desc Test Unified Record of Video
     * @tc.type FUNC
     */
    it('VideoTest001', 0, async function (done) {
        console.info('VideoTest001 begin');
        getVideoData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(videoData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('video' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('video' + VALUE_TEST_ELEMENT);
        expect(records[0].videoUri).assertEqual('videoUri');
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
        const primaryUri = pasteData.getPrimaryUri();
        expect(primaryUri).assertEqual('videoUri');
        done();
        console.info('VideoTest001 end');
    });

    /**
     * @tc.name AudioTest001
     * @tc.desc Test Unified Record of Audio
     * @tc.type FUNC
     */
    it('AudioTest001', 0, async function (done) {
        console.info('AudioTest001 begin');
        getAudioData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(audioData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].details.Key).assertEqual('audio' + KEY_TEST_ELEMENT);
        expect(records[0].details.Value).assertEqual('audio' + VALUE_TEST_ELEMENT);
        expect(records[0].audioUri).assertEqual('audioUri');
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.hasMimeType(pasteboard.MIMETYPE_TEXT_URI)).assertEqual(true);
        const primaryUri = pasteData.getPrimaryUri();
        expect(primaryUri).assertEqual('audioUri');
        done();
        console.info('AudioTest001 end');
    });

    /**
     * @tc.name SystemDefinedAppItemTest001
     * @tc.desc Test Unified Record of SystemDefinedAppItem
     * @tc.type FUNC
     */
    it('SystemDefinedAppItemTest001', 0, async function (done) {
        console.info('SystemDefinedAppItemTest001 begin');
        getSystemDefinedAppItemDataData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(systemDefinedAppItemData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
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
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.getPrimaryMimeType()).assertEqual('openharmony.app-item');
        done();
        console.info('SystemDefinedAppItemTest001 end');
    });

    /**
     * @tc.name ApplicationDefinedRecordTest001
     * @tc.desc Test Unified Record of ApplicationDefinedRecord
     * @tc.type FUNC
     */
    it('ApplicationDefinedRecordTest001', 0, async function (done) {
        console.info('ApplicationDefinedRecordTest001 begin');
        getApplicationDefinedRecordData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(applicationDefinedRecordData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        expect(records[0].applicationDefinedType).assertEqual('applicationDefinedType');
        for (let i = 0; i < U8_ARRAY.length; i++) {
            expect(records[0].rawData[i]).assertEqual(U8_ARRAY[i]);
        }
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.getPrimaryMimeType()).assertEqual('applicationDefinedType');
        done();
        console.info('ApplicationDefinedRecordTest001 end');
    });

    /**
     * @tc.name WantTest001
     * @tc.desc Test Unified Record of Want
     * @tc.type FUNC
     */
    it('WantTest001', 0, async function (done) {
        console.info('WantTest001 begin');
        getWantData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(wantData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_TEXT_WANT);
        done();
        console.info('WantTest001 end');
    });

    /**
     * @tc.name PixelMapTest001
     * @tc.desc Test Unified Record of PixelMap
     * @tc.type FUNC
     */
    it('PixelMapTest001', 0, async function (done) {
        console.info('PixelMapTest001 begin');
        getPixelMapData();
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(pixelMapData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        let unifiedData = await systemPasteboard.getUnifiedData();
        let records = unifiedData.getRecords();
        expect(records.length).assertEqual(1);
        let pasteData = await systemPasteboard.getData();
        expect(pasteData.getRecordCount()).assertEqual(1);
        expect(pasteData.getPrimaryMimeType()).assertEqual(pasteboard.MIMETYPE_PIXELMAP);
        done();
        console.info('PixelMapTest001 end');
    });
});