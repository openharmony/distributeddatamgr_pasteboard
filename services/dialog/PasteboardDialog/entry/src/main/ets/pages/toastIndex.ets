/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

 import { ToastInfo } from '../ServiceExtAbility/ToastExtensionAbility';

@Entry
@Component
struct Index {
  @State fromAppName: string = 'Unknown';
  @State toAppName: string = 'Unknown';
  @State toastHeight: number = 0;

  async aboutToAppear() {
    this.fromAppName = ToastInfo.getInstance().fromAppName;
    this.toAppName = ToastInfo.getInstance().toAppName;
    this.toastHeight = ToastInfo.getInstance().displayHeight;
  }

  build() {
    Row() {
      Column() {
        Text() {
          Span(this.toAppName).fontSize(14)
            .fontWeight(FontWeight.Regular).fontColor($r('app.color.white')).fontFamily($r('app.string.typeface'))
          Span($r('app.string.read')).fontSize(14)
            .fontWeight(FontWeight.Regular).fontColor($r('app.color.white')).fontFamily($r('app.string.typeface'))
          Span(this.fromAppName).fontSize(14)
            .fontWeight(FontWeight.Regular).fontColor($r('app.color.white')).fontFamily($r('app.string.typeface'))
          Span($r('app.string.info')).fontSize(14)
            .fontWeight(FontWeight.Regular).fontColor($r('app.color.white')).fontFamily($r('app.string.typeface'))
        }
        .height(19)
        .opacity(0.6)
        .margin({ left: 16, right: 16, top:8, bottom:8 })
      }
      .height(36)
      .backgroundColor($r('app.color.grey'))
      .borderRadius(18)
      .alignItems(HorizontalAlign.Center)
      .justifyContent(FlexAlign.Center)
    }
    .alignItems(VerticalAlign.Bottom)
    .justifyContent(FlexAlign.Center)
    .height(this.toastHeight)
    .width('100%')
  }
}