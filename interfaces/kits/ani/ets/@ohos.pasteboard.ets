/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import Want from "@ohos.app.ability.Want";
import image from "@ohos.multimedia.image";

export namespace pasteboard {

    loadLibrary("pasteboard_ani");

    export const MIMETYPE_TEXT_URI: string = "text/uri";
    export type ValueType = string | image.PixelMap | Want | ArrayBuffer;

    export native function createDataTypeValue(mimeType: string, value: ValueType): PasteData;
    export native function createDataRecord(data: Record<string, ValueType>): PasteData;
    export native function getSystemPasteboard(): SystemPasteboard;

    export function createData(mimeType: string, value: ValueType): PasteData {
        return createDataTypeValue(mimeType, value);
    }

    export function createData(data: Record<string, ValueType>): PasteData {
        return createDataRecord(data);
    }

    export enum ShareOption {
        INAPP,
        LOCALDEVICE,
        CROSSDEVICE
    }

    export interface PasteDataProperty {
        //shareOption: ShareOption;
        shareOption: number;
    }

    export interface PasteDataRecord {
        uri: string;
    }

    export interface PasteData {
        addRecord(record: PasteDataRecord): void;
        //addRecord(mimeType: string, value: ValueType): void;
        getRecordCount(): number;
        getRecord(index: number): PasteDataRecord;
        setProperty(property: PasteDataProperty): void;
        getProperty(): PasteDataProperty;
    }

    export interface SystemPasteboard {
        hasDataType(mimeType: string): boolean;
        //setData(data: PasteData, callback: AsyncCallback<void>): void;
        //setData(data: PasteData): Promise<void>;
        //clearData(callback: AsyncCallback<void>): void;
        //clearData(): Promise<void>;
        getDataSync(): PasteData;
        //getDataSource(): string;
    }
}

export class PasteDataRecordImpl implements pasteboard.PasteDataRecord {
    uri: string = "";
}

export class PasteDataPropertyImpl implements pasteboard.PasteDataProperty {
    shareOption: number = 0;
}

export class PasteDataImpl implements pasteboard.PasteData {
    private nativePtr: long = 0;
    constructor(nativePtr: long) {
        this.nativePtr = nativePtr;
    }

    getNativePtr() : long {
        return this.nativePtr;
    }

    native addRecord(record: pasteboard.PasteDataRecord): void;
    native getRecordCount(): number;
    native getRecord(index: number): PasteDataRecordImpl;
    native setProperty(property: pasteboard.PasteDataProperty): void;
    native getProperty(): pasteboard.PasteDataProperty;
}

export class SystemPasteboardImpl implements pasteboard.SystemPasteboard {
    private nativePtr: long = 0;
    constructor(nativePtr: long) {
        //console.println("nativePtr:" + nativePtr);
        //console.println("this.nativePtr:" + this.nativePtr);
        this.nativePtr = nativePtr;
    }

    getNativePtr() : long {
        return this.nativePtr;
    }

    native hasDataType(mimeType: string): boolean;
    //setData(data: PasteData, callback: AsyncCallback<void>): void;
    //setData(data: PasteData): Promise<void>;
    //clearData(callback: AsyncCallback<void>): void;
    //clearData(): Promise<void>;
    native getDataSync(): pasteboard.PasteData;
    //getDataSource(): string;
}