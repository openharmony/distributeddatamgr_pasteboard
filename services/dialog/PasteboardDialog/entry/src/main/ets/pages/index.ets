/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
import prompt from '@ohos.promptAction';

@Entry
@Component
struct Index {
  @State appName: string = 'Unknown'
  @State deviceType: string = 'Unknown'
  @State progressAngle: number = 0

  async aboutToAppear() {
    this.appName = globalThis.dialogInfo.appName
    this.deviceType = globalThis.dialogInfo.deviceType
    this.playRingAnimation()
  }

  playRingAnimation() {
    let action = () => {
      let keyframeAngle = this.progressAngle + 15
      this.progressAngle = keyframeAngle > 360 ? keyframeAngle - 360 : keyframeAngle
      this.playRingAnimation()
    }
    setTimeout(action, 500)
  }

  build() {
    Row() {
      Column() {
        Row() {
          Progress({
            value: 50,
            total: 100,
            type: ProgressType.Ring
          }).height(24)
            .width(24)
            .margin({
              top: 16,
              bottom: 16,
              left: 16
            })
            .style({
              strokeWidth: 2.67,
            })
            .linearGradient({ colors: ['#ff86c1ff', '#ff254FF7'] })
            .rotate({ angle: this.progressAngle })
            .animation({
              duration: 500,
              curve: Curve.Linear,
              playMode: PlayMode.Normal,
            })

          Column() {
            Text() {
              Span($r('app.string.title')).fontSize(12).fontWeight(FontWeight.Bold)
              Span(' ' + this.appName).fontSize(12).fontWeight(FontWeight.Bold)
            }
            .height(16)

            Text() {
              Span($r('app.string.from')).fontSize(10)
              Span(' "' + this.deviceType + '"').fontSize(10)
            }
            .height(16)
            .margin({ top: 2 })
            .opacity(0.6)
          }.padding({
            top: 12,
            left: 8,
            bottom: 12
          })

          Column() {
            Image($r('app.media.btn_cancel'))
              .width(18)
              .height(18)
              .objectFit(ImageFit.Fill)
              .margin({
                top: 19,
                bottom: 19,
                right: 19,
                left: 8
              })
          }.onClick(() => {
            prompt.showToast({
              message: $r('app.string.cancel'),
              duration: 1000
            });
            setTimeout(() => {
              globalThis.extensionWin.destroy()
              globalThis.context.terminateSelf()
            }, 1000)
          })
        }
        .height(56)
        .backgroundColor('#ffffff')
        .shadow({
          radius: 33,
          color: '#4d000000',
        })
        .borderRadius(33)
      }
      .alignItems(HorizontalAlign.Center)
      .justifyContent(FlexAlign.Center)
      .padding({ top: 10 })
    }
    .alignItems(VerticalAlign.Top)
    .justifyContent(FlexAlign.Center)
    .height(74)
    .width('100%')
  }
}