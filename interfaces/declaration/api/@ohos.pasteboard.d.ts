/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import { AsyncCallback } from './basic';
import Want from './@ohos.application.Want';
import { image } from './@ohos.multimedia.image';

/**
 * systemPasteboard
 * @syscap SystemCapability.MiscServices.Pasteboard
 * @import import pasteboard from '@ohos.pasteboard';
 */
declare namespace pasteboard {
  /**
   * Indicates the maximum number of records allowed in a PasteData object.
   * @since 7
   */
  const MAX_RECORD_NUM: number;
  /**
   * Indicates MIME types of HTML text.
   * @since 7
   */
  const MIMETYPE_TEXT_HTML: string;
  /**
   * Indicates MIME types of wants.
   * @since 7
   */
  const MIMETYPE_TEXT_WANT: string;
  /**
   * Indicates MIME types of plain text.
   * @since 7
   */
  const MIMETYPE_TEXT_PLAIN: string;
  /**
   * Indicates MIME types of URIs.
   * @since 7
   */
  const MIMETYPE_TEXT_URI: string;
  /**
   * Indicates MIME type of PixelMap.
   * @since 9
   */
  const MIMETYPE_PIXELMAP: string;

  /**
   * Indicates type of value.
   * @since 9
   */
  type ValueType = string | image.PixelMap | Want | ArrayBuffer;

  /**
   * Creates a PasteData object for PasteData#MIMETYPE_TEXT_HTML.
   * @param htmlText To save the Html text content.
   * @return Containing the contents of the clipboard content object.
   * @since 7
   * @deprecated since 9
   * @useinstead createData
   */
  function createHtmlData(htmlText: string): PasteData;

  /**
   * Creates a PasteData object for PasteData#MIMETYPE_TEXT_WANT.
   * @param want To save the want of content.
   * @return Containing the contents of the clipboard content object.
   * @since 7
   * @deprecated since 9
   * @useinstead createData
   */
  function createWantData(want: Want): PasteData;

  /**
   * Creates a PasteData object for PasteData#MIMETYPE_TEXT_PLAIN.
   * @param text To save the text of content.
   * @return Containing the contents of the clipboard content object.
   * @since 6
   * @deprecated since 9
   * @useinstead createData
   */
  function createPlainTextData(text: string): PasteData;

  /**
   * Creates a PasteData object for PasteData#MIMETYPE_TEXT_URI.
   * @param uri To save the uri of content.
   * @return Containing the contents of the clipboard content object.
   * @since 7
   * @deprecated since 9
   * @useinstead createData
   */
  function createUriData(uri: string): PasteData;

  /**
   * Creates a PasteData object with MIME type and value.
   * @param { string } mimeType - indicates MIME type of value.
   * @param { ValueType } value - indicates the content that is set to PasteData.
   * @returns { PasteData } a new PasteData object which contains mimeType and value.
   * @throws { BusinessError } if type of value is not one of ValueType.
   * @since 9
   */
  function createData(mimeType: string, value: ValueType): PasteData;

  /**
   * Creates a Record object for PasteData#MIMETYPE_TEXT_HTML.
   * @param htmlText To save the Html text content.
   * @return The content of a new record
   * @since 7
   * @deprecated since 9
   * @useinstead createRecord
   */
  function createHtmlTextRecord(htmlText: string): PasteDataRecord;

  /**
   * Creates a Record object for PasteData#MIMETYPE_TEXT_WANT.
   * @param want To save the want of content.
   * @return The content of a new record
   * @since 7
   * @deprecated since 9
   * @useinstead createRecord
   */
  function createWantRecord(want: Want): PasteDataRecord;

  /**
   * Creates a Record object for PasteData#MIMETYPE_TEXT_PLAIN.
   * @param text To save the text of content.
   * @return The content of a new record
   * @since 7
   * @deprecated since 9
   * @useinstead createRecord
   */
  function createPlainTextRecord(text: string): PasteDataRecord;

  /**
   * Creates a Record object for PasteData#MIMETYPE_TEXT_URI.
   * @param uri To save the uri of content.
   * @return The content of a new record
   * @since 7
   * @deprecated since 9
   * @useinstead createRecord
   */
  function createUriRecord(uri: string): PasteDataRecord;

  /**
   * Creates a record object with MIME type and value.
   * @param { string } mimeType - indicates MIME type of value.
   * @param { ValueType } value - content to be saved.
   * @returns { PasteDataRecord } a new PasteDataRecord object which contains mimeType and value.
   * @throws { BusinessError } if type of mimeType is not one of ValueType.
   * @since 9
   */
  function createRecord(mimeType: string, value: ValueType): PasteDataRecord;

  /**
   * get SystemPasteboard
   * @return The system clipboard object
   * @since 6
   */
  function getSystemPasteboard(): SystemPasteboard;

  /**
   * Types of scope that PasteData can be pasted.
   * @enum { number }
   * @since 9
   */
  enum ShareOption {
    /**
     * InApp indicates that only paste in the same app is allowed.
     * @since 9
     */
    InApp,
    /**
     * LocalDevice indicates that paste in any app in this device is allowed.
     * @since 9
     */
    LocalDevice,
    /**
     * CrossDevice indicates that paste in any app across devices is allowed.
     * @since9
     */
     CrossDevice
  }

  interface PasteDataProperty {
    /**
     * additional property data. key-value pairs.
     * @since 7
     */
    additions: {
      [key: string]: object
    }
    /**
     * non-repeating MIME types of all records in PasteData.
     * @since 7
     */
    readonly mimeTypes: Array<string>;
    /**
     * the user-defined tag of a PasteData object.
     * @since 7
     */
    tag: string;
    /**
     * a timestamp, which indicates when data is written to the system pasteboard.
     * @since 7
     */
    readonly timestamp: number;
    /**
     * Checks whether PasteData is set for local access only.
     * @since 7
     */
    localOnly: boolean;
    /**
     * Indicates the scope of clipboard data which can be pasted.
     * If it is not set or is incorrectly set, The default value is CrossDevice.
     * @type { ShareOption }
     * @since 9
     */
    shareOption: ShareOption;
  }

  interface PasteDataRecord {
    /**
     * HTML text in a record.
     * @since 7
     */
    htmlText: string;
    /**
     * an want in a record.
     * @since 7
     */
    want: Want;
    /**
     * MIME types of a record.
     * @since 7
     */
    mimeType: string;
    /**
     * plain text in a record.
     * @since 7
     */
    plainText: string;
    /**
     * an URI in a record.
     * @since 7
     */
    uri: string;
    /**
     * PixelMap in a record.
     * @type { image.PixelMap }
     * @since 9
     */
    pixelMap: image.PixelMap;
    /**
     * Custom data in a record, mimeType indicates the MIME type of custom data, ArrayBuffer indicates the value of custom data.
     * @type { object }
     * @since 9
     */
    data: {
        [mimeType: string]: ArrayBuffer
    }

    /**
     * Will a PasteData cast to the content of text content
     * @return callback Type string callback function
     * @since 7
     * @deprecated since 9
     * @useinstead convertToTextV9
     */
    convertToText(callback: AsyncCallback<string>): void;
    convertToText(): Promise<string>;

    /**
     * Will a PasteData cast to the content of text content.
     * @param { AsyncCallback<string> } callback - the callback of convertToTextV9.
     * @throws { BusinessError } if type of callback is not AsyncCallback<string>.
     * @since 9
     */
    convertToTextV9(callback: AsyncCallback<string>): void;

    /**
     * Will a PasteData cast to the content of text content
     * @returns { Promise<string> } the promise returned by the function.
     * @since 9
     */
    convertToTextV9(): Promise<string>;
  }

  interface PasteData {
    /**
     * Adds a Record for HTML text to a PasteData object, and updates the MIME type to PasteData#MIMETYPE_TEXT_HTML in DataProperty.
     * @param htmlText To save the Html text content.
     * @since 7
     * @deprecated since 9
     * @useinstead addRecord
     */
    addHtmlRecord(htmlText: string): void;

    /**
     * Adds an want Record to a PasteData object, and updates the MIME type to PasteData#MIMETYPE_TEXT_WANT in DataProperty.
     * @param want To save the want content.
     * @since 7
     * @deprecated since 9
     * @useinstead addRecord
     */
    addWantRecord(want: Want): void;

    /**
     * Adds a PasteRecord to a PasteData object and updates MIME types in DataProperty.
     * @param record The content of a new record.
     * @since 7
     */
    addRecord(record: PasteDataRecord): void;

    /**
     * Adds a Record for plain text to a PasteData object, and updates the MIME type to PasteData#MIMETYPE_TEXT_PLAIN in DataProperty.
     * @param text To save the text of content.
     * @since 7
     * @deprecated since 9
     * @useinstead addRecord
     */
    addTextRecord(text: string): void;

    /**
     * Adds a URI Record to a PasteData object, and updates the MIME type to PasteData#MIMETYPE_TEXT_URI in DataProperty.
     * @param uri To save the uri of content.
     * @since 7
     * @deprecated since 9
     * @useinstead addRecord
     */
    addUriRecord(uri: string): void;

    /**
     * Adds a record with mimeType and value to a PasteData object.
     * @param { string } mimeType - indicates the MIME type of value.
     * @param { ValueType } value - content to be saved.
     * @throws { BusinessError } if type of mimeType is not one of ValueType.
     * @throws { BusinessError } if the count of records in PasteData exceeds MAX_RECORD_NUM.
     * @since 9
     */
    addRecord(mimeType: string, value: ValueType): void;

    /**
     * MIME types of all content on the pasteboard.
     * @return string type of array
     * @since 7
     */
    getMimeTypes(): Array<string>;

    /**
     * HTML text of the primary record in a PasteData object.
     * @return string type of htmltext
     * @since 7
     */
    getPrimaryHtml(): string;

    /**
     * the want of the primary record in a PasteData object.
     * @return want type of want
     * @since 7
     */
    getPrimaryWant(): Want;

    /**
     * the MIME type of the primary record in a PasteData object.
     * @return string type of mimetype
     * @since 7
     */
    getPrimaryMimeType(): string;

    /**
     * the plain text of the primary record in a PasteData object.
     * @return string type of text
     * @since 6
     */
    getPrimaryText(): string;

    /**
     * the URI of the primary record in a PasteData object.
     * @return string type of uri
     * @since 7
     */
    getPrimaryUri(): string;

    /**
     * Gets the primary PixelMap record in a PasteData object.
     * @returns {image.PixelMap} pixelMap
     * @since 9
     */
    getPrimaryPixelMap(): image.PixelMap;

    /**
     * DataProperty of a PasteData object.
     * @return PasteDataProperty type of PasteDataProperty
     * @since 7
     */
    getProperty(): PasteDataProperty;

    /**
     * Sets PasteDataProperty to a PasteData object, Modifying shareOption is supported only.
     * @param { PasteDataProperty } property - save property to PasteData object.
     * @throws { BusinessError } if type of property is not PasteDataProperty.
     * @since 9
     */
    setProperty(property: PasteDataProperty): void;

    /**
     * a Record based on a specified index.
     * @param index The index to specify the content item
     * @return PasteDataRecord type of PasteDataRecord
     * @since 7
     * @deprecated since 9
     * @useinstead getRecord
     */
    getRecordAt(index: number): PasteDataRecord;

    /**
     * Gets record by index in PasteData.
     * @param { number } index - indicates the record index in PasteData.
     * @returns { PasteDataRecord } the record in PasteData with index.
     * @throws { BusinessError } if type of index is not number.
     * @throws { BusinessError } if index is out of the record count of PasteData.
     * @since 9
     */
    getRecord(index: number): PasteDataRecord;

    /**
     * the number of records in a PasteData object.
     * @return The number of the clipboard contents
     * @since 7
     */
    getRecordCount(): number;

    /**
     * the user-defined tag of a PasteData object.
     * @return string type of tag
     * @since 7
     */
    getTag(): string;

    /**
     * Checks whether there is a specified MIME type of data in DataProperty.
     * @param mimeType To query data types.
     * @return if having mimeType in PasteData returns true, else returns false.
     * @since 7
     * @deprecated since 9
     * @useinstead hasType
     */
    hasMimeType(mimeType: string): boolean;

    /**
     * Checks whether there is a specified MIME type of data in DataProperty.
     * @param { string } mimeType - indicates to query data type.
     * @returns { boolean } if having mimeType in PasteData returns true, else returns false.
     * @throws { BusinessError } if type of path is not string.
     * @since 9
     */
    hasType(mimeType: string): boolean;

    /**
     * Removes a Record based on a specified index.
     * @param index The index to specify the content item.
     * @return The query returns True on success, or False on failure.
     * @since 7
     * @deprecated since 9
     * @useinstead removeRecord
     */
    removeRecordAt(index: number): boolean;

    /**
     * Removes a Record based on a specified index.
     * @param { number } index - indicates the record index in PasteData.
     * @throws { BusinessError } if type of index is not number.
     * @throws { BusinessError } if index is out of the record count of PasteData.
     * @since 9
     */
    removeRecord(index: number): void;

    /**
     * Replaces a specified record with a new one.
     * @param index The index to specify the content item. record record The content of a new record.
     * @return The query returns True on success, or False on failure.
     * @since 7
     * @deprecated since 9
     * @useinstead replaceRecord
     */
    replaceRecordAt(index: number, record: PasteDataRecord): boolean;

    /**
     * Replaces a specified record with a new one.
     * @param { number } index - indicates the record index in PasteData.
     * @param { PasteDataRecord } record - the content of a new record.
     * @throws { BusinessError } if type of index is not number or type of record is not PasteDataRecord.
     * @throws { BusinessError } if index is out of the record count of PasteData.
     * @since 9
     */
    replaceRecord(index: number, record: PasteDataRecord): void;
  }

  interface SystemPasteboard {
    /**
     * Callback invoked when pasteboard content changes.
     * @param { string } type - indicates pasteboard content changed.
     * @param { () => void } callback - the callback to add.
     * @throws { BusinessError } if type is not string or callback is not () => void.
     * @since 7
     */
    on(type: 'update', callback: () => void): void;
    /**
     * Remove a callback invoked when pasteboard content changes.
     * @param { string } type - indicates pasteboard content changed.
     * @param { () => void } [callback] - the callback to remove.
     * @throws { BusinessError } if type is not string or callback is not () => void.
     * @since 7
     */
    off(type: 'update', callback?: () => void): void;

    /**
     * Clears the pasteboard.
     * @since 7
     * @deprecated since 9
     * @useinstead clearData
     */
    clear(callback: AsyncCallback<void>): void;
    clear(): Promise<void>;

    /**
     * Clears the pasteboard.
     * @param { AsyncCallback<void> } callback - the callback of clearData.
     * @throws { BusinessError } if callback is not AsyncCallback<void>.
     * @since 9
     */
    clearData(callback: AsyncCallback<void>): void;

    /**
     * Clears the pasteboard.
     * @returns {Promise<void> } the promise returned by the clearData.
     * @since 9
     */
    clearData(): Promise<void>;

    /**
     * data in a PasteData object.
     * @return PasteData callback data in a PasteData object.
     * @since 6
     * @deprecated since 9
     * @useinstead getData
     */
    getPasteData(callback: AsyncCallback<PasteData>): void;
    getPasteData(): Promise<PasteData>;

    /**
     * Gets pastedata from the system pasteboard.
     * @param { AsyncCallback<PasteData> } callback - the callback of getData.
     * @throws { BusinessError } if type of callback is not AsyncCallback<PasteData>.
     * @throws { BusinessError } if another getData is being processed.
     * @since 9
     */
    getData(callback: AsyncCallback<PasteData>): void;

    /**
     * Gets pastedata from the system pasteboard.
     * @returns { Promise<PasteData> } the promise returned by the getData.
     * @throws { BusinessError } if another getData is being processed.
     * @since 9
     */
    getData(): Promise<PasteData>;

    /**
     * Checks whether there is content in the pasteboard.
     * @return boolean The callback success to true to false failure
     * @since 7
     * @deprecated since 9
     * @useinstead hasData
     */
    hasPasteData(callback: AsyncCallback<boolean>): void;
    hasPasteData(): Promise<boolean>;

    /**
     * Checks whether there is content in the system pasteboard.
     * @param { AsyncCallback<boolean> } callback - the callback of hasData.
     * @throws { BusinessError } if type of callback is not AsyncCallback<boolean>.
     * @since 9
     */
    hasData(callback: AsyncCallback<boolean>): void;

    /**
     * Checks whether there is content in the system pasteboard.
     * @returns { promise<boolean> } the promise returned by the function.
     * since 9
     */
    hasData(): promise<boolean>;

    /**
     * Writes PasteData to the pasteboard.
     * @param  data Containing the contents of the clipboard content object.
     * @since 6
     * @deprecated since 9
     * @useinstead setData
     */
    setPasteData(data: PasteData, callback: AsyncCallback<void>): void;
    setPasteData(data: PasteData): Promise<void>;

    /**
     * Writes PasteData to the system pasteboard.
     * @param { PasteData } data - PasteData will be written to the clipboard
     * @param { AsyncCallback<void> } callback - the callback of setData.
     * @throws { BusinessError } if type of data is not PasteData or type of callback is not AsyncCallback<void>.
     * @throws { BusinessError } if another setData is being processed.
     * @since 9
     */
    setData(data: PasteData, callback: AsyncCallback<void>): void;

    /**
     * Writes PasteData to the system pasteboard.
     * @param { PasteData } data - PasteData will be written to the clipboard.
     * @returns { Promise<void> } the promise returned by the function.
     * @throws { BusinessError } if type of data is not PasteData.
     * @throws { BusinessError } if another setData is being processed.
     * @since 9
     */
    setData(data: PasteData): Promise<void>;
  }
}

export default pasteboard;
