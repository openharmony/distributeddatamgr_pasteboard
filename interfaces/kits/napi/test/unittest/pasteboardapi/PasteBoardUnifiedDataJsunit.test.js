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
import image from '@ohos.multimedia.image';

const KEY_TEST_ELEMENT = 'TestKey';
const VALUE_TEST_ELEMENT = 'TestValue';

describe('PasteBoardUnifiedDataJSTest', function () {
    beforeAll(async function () {
        console.info('beforeAll');
    });

    afterAll(async function () {
        console.info('afterAll');
    });

    /**
     * @tc.name TextTest001
     * @tc.desc Test Unified Record of Text
     * @tc.type FUNC
     */
    it('TextTest001', 0, async function (done) {
        console.info('TextTest001 begin');
        let text = new UDC.Text();
        text.details = {
            Key: 'text' + KEY_TEST_ELEMENT,
            Value: 'text' + VALUE_TEST_ELEMENT,
        };
        let inputData = new UDC.UnifiedData(text);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('text' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('text' + VALUE_TEST_ELEMENT);
            done();
        });
        console.info('TextTest001 end');
    });

    /**
     * @tc.name PlainTextTest001
     * @tc.desc Test Unified Record of Plain Text
     * @tc.type FUNC
     */
    it('PlainTextTest001', 0, async function (done) {
        console.info('PlainTextTest001 begin');
        let plainText = new UDC.PlainText();
        plainText.details = {
            Key: 'plainText' + KEY_TEST_ELEMENT,
            Value: 'plainText' + VALUE_TEST_ELEMENT,
        };
        plainText.textContent = 'textContent';
        plainText.abstract = 'abstract';
        let inputData = new UDC.UnifiedData(plainText);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('plainText' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('plainText' + VALUE_TEST_ELEMENT);
            expect(records[0].textContent).assertEqual('textContent');
            expect(records[0].abstract).assertEqual('abstract');
            done();
        });
        console.info('PlainTextTest001 end');
    });

    /**
     * @tc.name PlainTextTest001
     * @tc.desc Test Unified Record of Plain Text
     * @tc.type FUNC
     */
    it('PlainTextTest001', 0, async function (done) {
        console.info('PlainTextTest001 begin');
        let plainText = new UDC.PlainText();
        plainText.details = {
            Key: 'plainText' + KEY_TEST_ELEMENT,
            Value: 'plainText' + VALUE_TEST_ELEMENT,
        };
        plainText.textContent = 'textContent';
        plainText.abstract = 'abstract';
        let inputData = new UDC.UnifiedData(plainText);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('plainText' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('plainText' + VALUE_TEST_ELEMENT);
            expect(records[0].textContent).assertEqual('textContent');
            expect(records[0].abstract).assertEqual('abstract');
            done();
        });
        console.info('PlainTextTest001 end');
    });

    /**
     * @tc.name HyperlinkTest001
     * @tc.desc Test Unified Record of Hyper Link
     * @tc.type FUNC
     */
    it('HyperlinkTest001', 0, async function (done) {
        console.info('HyperlinkTest001 begin');
        let link = new UDC.Hyperlink();
        link.details = {
            Key: 'hyperLink' + KEY_TEST_ELEMENT,
            Value: 'hyperLink' + VALUE_TEST_ELEMENT,
        };
        link.url = 'url';
        link.description = 'description';
        let inputData = new UDC.UnifiedData(link);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('hyperLink' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('hyperLink' + VALUE_TEST_ELEMENT);
            expect(records[0].url).assertEqual('url');
            expect(records[0].description).assertEqual('description');
            done();
        });
        console.info('HyperlinkTest001 end');
    });

    /**
     * @tc.name HtmlTest001
     * @tc.desc Test Unified Record of Html
     * @tc.type FUNC
     */
    it('HtmlTest001', 0, async function (done) {
        console.info('HtmlTest001 begin');
        let html = new UDC.HTML();
        html.details = {
            Key: 'html' + KEY_TEST_ELEMENT,
            Value: 'html' + VALUE_TEST_ELEMENT,
        };
        html.htmlContent = 'htmlContent';
        html.plainContent = 'plainContent';
        let inputData = new UDC.UnifiedData(html);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('html' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('html' + VALUE_TEST_ELEMENT);
            expect(records[0].htmlContent).assertEqual('htmlContent');
            expect(records[0].plainContent).assertEqual('plainContent');
            done();
        });
        console.info('HtmlTest001 end');
    });

    /**
     * @tc.name FileTest001
     * @tc.desc Test Unified Record of File
     * @tc.type FUNC
     */
    it('FileTest001', 0, async function (done) {
        console.info('FileTest001 begin');
        let file = new UDC.File();
        file.details = {
            Key: 'file' + KEY_TEST_ELEMENT,
            Value: 'file' + VALUE_TEST_ELEMENT,
        };
        file.uri = 'uri';
        let inputData = new UDC.UnifiedData(file);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('file' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('file' + VALUE_TEST_ELEMENT);
            expect(records[0].uri).assertEqual('uri');
            done();
        });
        console.info('FileTest001 end');
    });

    /**
     * @tc.name FolderTest001
     * @tc.desc Test Unified Record of Folder
     * @tc.type FUNC
     */
    it('FolderTest001', 0, async function (done) {
        console.info('FolderTest001 begin');
        let folder = new UDC.Folder();
        folder.details = {
            Key: 'folder' + KEY_TEST_ELEMENT,
            Value: 'folder' + VALUE_TEST_ELEMENT,
        };
        folder.uri = 'folderUri';
        let inputData = new UDC.UnifiedData(folder);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('folder' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('folder' + VALUE_TEST_ELEMENT);
            expect(records[0].uri).assertEqual('folderUri');
            done();
        });
        console.info('FolderTest001 end');
    });

    /**
     * @tc.name ImageTest001
     * @tc.desc Test Unified Record of Image
     * @tc.type FUNC
     */
    it('ImageTest001', 0, async function (done) {
        console.info('ImageTest001 begin');
        let image = new UDC.Image();
        image.details = {
            Key: 'image' + KEY_TEST_ELEMENT,
            Value: 'image' + VALUE_TEST_ELEMENT,
        };
        image.imageUri = 'imageUri';
        let inputData = new UDC.UnifiedData(image);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('image' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('image' + VALUE_TEST_ELEMENT);
            expect(records[0].imageUri).assertEqual('imageUri');
            done();
        });
        console.info('ImageTest001 end');
    });

    /**
     * @tc.name VideoTest001
     * @tc.desc Test Unified Record of Video
     * @tc.type FUNC
     */
    it('VideoTest001', 0, async function (done) {
        console.info('VideoTest001 begin');
        let video = new UDC.Video();
        video.details = {
            Key: 'video' + KEY_TEST_ELEMENT,
            Value: 'video' + VALUE_TEST_ELEMENT,
        };
        video.videoUri = 'videoUri';
        let inputData = new UDC.UnifiedData(video);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('video' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('video' + VALUE_TEST_ELEMENT);
            expect(records[0].videoUri).assertEqual('videoUri');
            done();
        });
        console.info('VideoTest001 end');
    });

    /**
     * @tc.name AudioTest001
     * @tc.desc Test Unified Record of Audio
     * @tc.type FUNC
     */
    it('AudioTest001', 0, async function (done) {
        console.info('AudioTest001 begin');
        let audio = new UDC.Audio();
        audio.details = {
            Key: 'audio' + KEY_TEST_ELEMENT,
            Value: 'audio' + VALUE_TEST_ELEMENT,
        };
        audio.audioUri = 'audioUri';
        let inputData = new UDC.UnifiedData(audio);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('audio' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('audio' + VALUE_TEST_ELEMENT);
            expect(records[0].audioUri).assertEqual('audioUri');
            done();
        });
        console.info('AudioTest001 end');
    });



    /**
     * @tc.name AudioTest001
     * @tc.desc Test Unified Record of Audio
     * @tc.type FUNC
     */
    it('AudioTest001', 0, async function (done) {
        console.info('AudioTest001 begin');
        let audio = new UDC.Audio();
        audio.details = {
            Key: 'audio' + KEY_TEST_ELEMENT,
            Value: 'audio' + VALUE_TEST_ELEMENT,
        };
        audio.audioUri = 'audioUri';
        let inputData = new UDC.UnifiedData(audio);
        const systemPasteboard = pasteboard.getSystemPasteboard();
        await systemPasteboard.setUnifiedData(inputData);
        const flag = await systemPasteboard.hasPasteData();
        expect(flag).assertEqual(true);
        systemPasteboard.getUnifiedData().then((data) => {
            const outputData = data;
            let records = outputData.getRecords();
            expect(records.length).assertEqual(1);
            expect(records[0].details.Key).assertEqual('audio' + KEY_TEST_ELEMENT);
            expect(records[0].details.Value).assertEqual('audio' + VALUE_TEST_ELEMENT);
            expect(records[0].audioUri).assertEqual('audioUri');
            done();
        });
        console.info('AudioTest001 end');
    });

});