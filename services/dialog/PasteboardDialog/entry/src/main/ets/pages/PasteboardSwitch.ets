/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
import common from '@ohos.app.ability.common';
import display from '@ohos.display';
import settings from '@ohos.settings';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import deviceInfo from '@ohos.deviceInfo';
import { BusinessError } from '@ohos.base';
import { EditableLeftIconType, EditableTitleBar } from '@ohos.arkui.advanced.EditableTitleBar';
import mediaQuery from '@ohos.mediaquery';
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import CommonConstants from '../common/constants/CommonConstants';
import { TipsJumpUtils } from '../utils/TipsJumpUtils';
import { LengthMetrics } from '@ohos.arkui.node';

const TAG = '[PasteboardSwitch_Page] : ';
let context = getContext(this) as common.UIAbilityContext;
let localStorage = LocalStorage.getShared();

interface switchStatus {
  open: string;
  close: string;
}

let switchState: switchStatus = {
  open: CommonConstants.SWITCH_STATUS_OPEN,
  close: CommonConstants.SWITCH_STATUS_CLOSE
}

@Entry
@Component
struct PasteboardSwitch {
  @StorageLink('isSwitchOn') isSwitchOn: boolean | undefined = true;
  @StorageLink('pasteboardSession') pasteboardSession: UIExtensionContentSession | undefined = undefined;
  @State title: string = '';
  @State screenHeight: number = 0;
  @State screenWidth: number = 0;
  @State shortSideSize: number = 0;
  @State imageAnimatorHeight: number = 0;
  @State imageAnimatorWidth: number = 0;
  @State textWidth: number = 0;
  @State gapLength: number = 0;
  @State isShow: boolean = false;
  @State is2in1: boolean = false;
  @State portraitFunc: mediaQuery.MediaQueryResult | void | null = null;
  @State contentHeight: number = 0;
  @State imageArray: Array<ImageFrameInfo> = [];
  @State animationState: AnimationStatus = AnimationStatus.Running;
  @State reverse: boolean = false;
  @State iterations: number = -1;
  @State isShowBack: boolean = true;
  @State titleBarMarginNormal: LocalizedMargin = {
    start: LengthMetrics.resource($r('sys.float.margin_left')),
  };
  @State titleBarMarginPc: LocalizedMargin = {
    start: LengthMetrics.vp(16),
  };
  @State paddingStartNormal: LengthMetrics = LengthMetrics.resource($r('sys.float.margin_left'));
  @State paddingStartPc: LengthMetrics = LengthMetrics.vp(16);
  private listener: mediaQuery.MediaQueryListener = mediaQuery.matchMediaSync('(dark-mode:true)');
  private extContext?: common.UIExtensionContext;
  private scroller: Scroller = new Scroller();
  private startReason?: string = '';
  private learnMore: ResourceStr = $r('app.string.learn_more');
  private pasteboardDesc: ResourceStr = $r('app.string.pasteboard_desc_text', '');

  onPortrait(mediaQueryResult: mediaQuery.MediaQueryResult) {
    console.log(TAG + 'onPortrait in');
    if (mediaQueryResult.matches as boolean) {
      this.extContext?.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_DARK);
    } else {
      this.extContext?.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_LIGHT);
    }
  }

  /**
   * @description Calculate the ImageAnimator size
   *   Calculation rule:
   *    1.general phone & pad : screen height/2 compare with width,shorter is ImageAnimator width
   *    2.pc: screen height/2 compare with width,shorter is ImageAnimator height
   *    3.ratio: 3:2
   * @param height
   * @param width
   */
  getImageAnimatorSize(height: Length, width: Length) {
    console.log(TAG + 'getImageAnimatorSize in, deviceInfo.deviceType : ' + deviceInfo.deviceType);
    let totalHeight = height as number;
    this.shortSideSize = width < totalHeight / 2 ? width as number : totalHeight / 2 as number;
    if (deviceInfo.deviceType === 'phone' || deviceInfo.deviceType === 'tablet') {
      this.imageAnimatorWidth = this.shortSideSize * 0.8;
      this.imageAnimatorHeight = this.shortSideSize * 0.8 * 2 / 3;
    } else if (deviceInfo.deviceType === '2in1') {
      this.imageAnimatorWidth = this.shortSideSize * 0.8 * 3 / 2;
      this.imageAnimatorHeight = this.shortSideSize * 0.8;
    }
    console.log(TAG + 'this.shortSideSize = ' + this.shortSideSize + ', this.imageAnimatorWidth = ' +
    this.imageAnimatorWidth + ', this.imageAnimatorHeight = ' + this.imageAnimatorHeight);
  }

  getStringSync() {
    console.log(TAG + 'getStringSync in');
    try {
      context.resourceManager.getStringValue($r('app.string.pasteboard_title')
        .id, (error: BusinessError, value: string) => {
        if (error != null) {
          console.error(TAG + 'error is ' + error);
        } else {
          this.title = value;
          console.info(TAG + '<aboutToAppear> this.title : ' + this.title);
        }
      })
    } catch (error) {
      let code: number = (error as BusinessError).code;
      let message: string = (error as BusinessError).message;
      console.error(TAG + `callback getStringValue failed,error code: ${code},message: ${message}.`);
    }
  }

  getImageArray() {
    console.info(TAG + 'getImageArray in');
    for (let i = 1; i <= CommonConstants.IMAGE_COUNT; i++) {
      this.imageArray.push({
        src: $r(`app.media.pasteboard_${i}`),
        duration: (i == CommonConstants.IMAGE_COUNT) ? CommonConstants.IMG_ANIMATOR_OVER_DURATION
          : CommonConstants.IMG_ANIMATOR_NORMAL_DURATION
      })
    }
  }

  onPageShow() {
    console.log(TAG + 'onPageShow in');
    this.getGapLength();
    display.getAllDisplays((err, data) => {
      this.screenWidth = px2vp(data[0].width);
      this.screenHeight = px2vp(data[0].height);
      this.contentHeight = this.screenHeight;
      console.log(TAG + 'screenWidth = ' + this.screenWidth + '; screenHeight = ' + this.screenHeight);
    })
    this.is2in1 = deviceInfo.deviceType === '2in1' ? true : false;
  }

  aboutToAppear() {
    console.log(TAG + 'aboutToAppear in');
    // Switch State Initialization
    let value = settings.getValueSync(context, 'distributed_pasteboard_switch', switchState.open);
    this.isSwitchOn = value != switchState.close ? true : false;
    console.log(TAG + '<aboutToAppear> this.isSwitchOn : ' + this.isSwitchOn + '; value: ' + value);

    AppStorage.setOrCreate('isSwitchOn', this.isSwitchOn);
    console.log(TAG + 'AppStorage.get<boolean>(isSwitchOn) : ' + AppStorage.get<boolean>('isSwitchOn'));

    if (this.isSwitchOn) {
      let status: boolean = settings.setValueSync(context, 'distributed_pasteboard_switch', switchState.open);
      console.log(TAG + 'set value success :' + status + '; set:distributed_pasteboard_switch is 1');
    }

    this.getStringSync();
    this.getImageArray();

    this.listener.on('change', (mediaQueryResult: mediaQuery.MediaQueryResult) => {
      this.onPortrait(mediaQueryResult);
    })
    this.extContext = localStorage.get<common.UIExtensionContext>('context');
    this.startReason = AppStorage.get<string>('startReason');
    this.isShowBack = AppStorage.get<boolean>('isShowBack') ?? true;
    console.info(`${TAG} aboutToAppear: startReason is ${this.startReason}, isShowBack: ${this.isShowBack}`);
    if (this.isPhone()) {
      this.checkFoldBackButton();
    }
    this.checkPcPadBackButton();
  }

  aboutToDisappear() {
    console.info(`${TAG} aboutToDisappear in`);
  }

  getGapLength() {
    console.log(TAG + 'getGapLength in, deviceInfo.deviceType : ' + deviceInfo.deviceType);
    if (deviceInfo.deviceType == 'phone') {
      this.gapLength = CommonConstants.GENERAL_PHONE_GAP_LENGTH;
    } else if (deviceInfo.deviceType == '2in1' || deviceInfo.deviceType == 'tablet') {
      this.gapLength = CommonConstants.PC_PAD_GAP_LENGTH;
    }
    console.log(TAG + 'this.gapLength : ' + this.gapLength);
  }

  onBackPress() {
    console.log(TAG + 'onBackPress in');
  }

  @Builder
  NormalRootContent() {
    this.titleBar();
    this.ContentBuilder();
  }

  @Builder
  SearchRootContent() {
    NavDestination() {
      this.ContentBuilder();
    }
    .hideTitleBar(false)
    .title($r('app.string.pasteboard_title'), {
      paddingStart: this.is2in1 ? this.paddingStartPc : this.paddingStartNormal,
    })
    .backgroundColor($r('sys.color.ohos_id_color_titlebar_sub_bg'))
  }

  @Builder
  titleBar() {
    Column() {
      EditableTitleBar({
        leftIconStyle: EditableLeftIconType.Back,
        title: $r('app.string.pasteboard_title'),
        isSaveIconRequired: false,
        contentMargin: this.is2in1 ? this.titleBarMarginPc : this.titleBarMarginNormal,
        onCancel: () => {
          try {
            if (this.pasteboardSession && this.pasteboardSession.sendData) {
              this.pasteboardSession.sendData({ 'action': 'pop' })
            } else {
              console.error(TAG + 'pasteboardSession is undefined');
            }
          } catch (error) {
            let code: number = (error as BusinessError).code;
            let message: string = (error as BusinessError).message;
            console.error(`${TAG} call onCancel failed, error code: ${code}, message: ${message}.`);
          }
        }
      })
    }
  }

  @Builder
  ContentBuilder() {
    Scroll(this.scroller) {
      Column() {
        ImageAnimator()
          .images(this.imageArray)
          .state(this.animationState)
          .reverse(this.reverse)
          .fillMode(this.iterations)
          .iterations(this.iterations)
          .width(this.imageAnimatorWidth)
          .height(this.imageAnimatorHeight)
          .onStart(() => {
            console.info(TAG + 'ImageAnimator Start');
          })
          .onFinish(() => {
            console.info(TAG + 'ImageAnimator Finish');
          })

        Text() {
          Span(this.pasteboardDesc)
            .fontFamily('HarmonyHeiTi')
            .fontSize($r('sys.float.ohos_id_text_size_body2'))
            .fontColor($r('sys.color.ohos_id_color_text_secondary'))
            .fontWeight(FontWeight.Regular)
          Span(this.learnMore)
            .fontFamily('HarmonyHeiTi')
            .fontSize($r('sys.float.ohos_id_text_size_body2'))
            .fontColor($r('sys.color.ohos_id_color_text_primary_activated'))
            .fontWeight(FontWeight.Medium)
            .onClick(() => {
              TipsJumpUtils.jumpTips(getContext(this) as common.UIAbilityContext, CommonConstants.FUN_NUM,
                CommonConstants.TIPS_TYPE);
            })
        }
        .margin({
          bottom: CommonConstants.PASTEBOARD_DESC_TEXT_MARGIN_BOTTOM,
          top: CommonConstants.PASTEBOARD_DESC_TEXT_MARGIN_TOP
        })
        .textAlign(TextAlign.Center)
        .width('100%')

        Column() {
          Flex({
            direction: FlexDirection.Row,
            justifyContent: FlexAlign.SpaceBetween,
            alignItems: ItemAlign.Center
          }) {
            Text($r('app.string.pasteboard_title'))
              .fontSize($r('sys.float.ohos_id_text_size_sub_title2'))
              .fontWeight(FontWeight.Medium)
              .fontColor($r('sys.color.ohos_id_color_text_primary'))
              .accessibilityLevel('no')
              .maxFontScale(2.8)
              .padding({
                top: this.is2in1 ? CommonConstants.ITEM_LIST_PADDING_TOP_PC : CommonConstants.ITEM_LIST_PADDING_TOP,
                bottom: this.is2in1 ? CommonConstants.ITEM_LIST_PADDING_BOTTOM_PC :
                CommonConstants.ITEM_LIST_PADDING_BOTTOM
              })

            Toggle({ type: ToggleType.Switch, isOn: this.isSwitchOn })
              .width(CommonConstants.PASTEBOARD_SWITCH_WIDTH)
              .height(CommonConstants.PASTEBOARD_SWITCH_HEIGHT)
              .hoverEffect(HoverEffect.None)
              .onChange((isOn: boolean) => {
                console.log(TAG + 'isOn:' + isOn);
                this.isSwitchOn = isOn;
                AppStorage.setAndLink('isSwitchOn', isOn);
                if (isOn) {
                  let status: boolean =
                    settings.setValueSync(context, 'distributed_pasteboard_switch', switchState.open);
                  console.log(TAG + 'is set success :' + status + '; set:distributed_pasteboard_switch is on');
                } else {
                  let status: boolean =
                    settings.setValueSync(context, 'distributed_pasteboard_switch', switchState.close);
                  console.log(TAG + 'is set success :' + status + '; set:distributed_pasteboard_switch is close');
                }
              })
          }
          .width('100%')
          .padding({
            left: CommonConstants.TEXT_LIST_ALIGN_DISTANCE,
            right: CommonConstants.TEXT_LIST_ALIGN_DISTANCE
          })
          .backgroundColor($r('sys.color.ohos_id_color_list_card_bg'))
          .borderRadius(this.is2in1 ? CommonConstants.PC_BORDER_RADIUS : CommonConstants.NON_PC_BORDER_RADIUS)
          .accessibilityText(this.title)
        }
        .width('100%')
        .constraintSize({
          minHeight: CommonConstants.PC_LIST_HEIGHT
        })

        Column() {
          Flex({
            direction: FlexDirection.Row,
            justifyContent: FlexAlign.Start,
            alignItems: ItemAlign.Center
          }) {
            SymbolGlyph($r('sys.symbol.info_circle_fill'))
              .fontWeight(FontWeight.Medium)
              .fontSize(CommonConstants.SYMBOL_INFO_CIRCLE)
              .fontColor([$r('sys.color.ohos_id_color_activated')])
              .margin({ right: CommonConstants.SYMBOL_MARGIN_RIGHT })

            Text($r('app.string.update_version_prompt'))
              .fontSize($r('sys.float.ohos_id_text_size_body3'))
              .fontWeight(FontWeight.Medium)
              .fontColor($r('sys.color.ohos_id_color_text_primary'))
              .textAlign(TextAlign.Start)
              .lineHeight(CommonConstants.UPDATE_PROMPT_LINE_HEIGHT)
          }
          .margin({ top: CommonConstants.UPDATE_PROMPT_MARGIN_TOP })
        }
        .padding({
          left: CommonConstants.TEXT_LIST_ALIGN_DISTANCE,
          right: CommonConstants.TEXT_LIST_ALIGN_DISTANCE
        })
      }
      .width('100%')
      .padding({ left: this.gapLength, right: this.gapLength })
      .margin({ bottom: this.contentHeight * 0.09 })
      .backgroundColor($r('sys.color.ohos_id_color_sub_background'))
    }
    .width('100%')
    .height(this.screenHeight)
    .scrollable(ScrollDirection.Vertical)
    .scrollBar(BarState.Off)
    .align(Alignment.TopStart)
    .friction(0.6)
    .edgeEffect(EdgeEffect.Spring)
    .onWillScroll((xOffset: number, yOffset: number) => {
      console.info(TAG + 'onScroll : xOffset:' + xOffset + ' ; yOffset' + yOffset);
    })
    .onScrollEdge(() => {
      console.info('To the edge');
    })
    .onScrollStop(() => {
      console.info('Scroll Stop');
    })
    .onAreaChange((oldArea: Area, newArea: Area) => {
      console.log(TAG + 'Scroll, oldArea.height = ' + oldArea.height + ', newArea.height = ' + newArea.height);
    })
  }

  build() {
    Column() {
      if (this.isShowBack) {
        this.NormalRootContent();
      } else {
        this.SearchRootContent();
      }
    }
    .width('100%')
    .height('100%')
    .backgroundColor($r('sys.color.ohos_id_color_sub_background'))
    .onAreaChange((oldArea: Area, newArea: Area) => {
      console.log(TAG + 'build column , oldArea.height = ' + oldArea.height + ', newArea.height = ' + newArea.height);
      this.getImageAnimatorSize(newArea.height, newArea.width);
      console.info(TAG + 'this.shortSideSize = ' + this.shortSideSize + ', this.imageAnimatorWidth = ' +
      this.imageAnimatorWidth + ', this.imageAnimatorHeight = ' + this.imageAnimatorHeight);
    })
  }

  private checkPcPadBackButton(): void {
    console.info(`${TAG} checkPcPadBackButton in`);
    if (this.startReason === 'from_search') {
      if (deviceInfo.deviceType === '2in1' || deviceInfo.deviceType === 'tablet') {
        this.isShowBack = false;
      }
    }
  }

  private isPhone(): boolean {
    console.info(`${TAG} isPhone in`);
    return (deviceInfo.deviceType === 'phone' || deviceInfo.deviceType === 'default');
  }

  private refreshFoldStatus(foldStatus: display.FoldStatus): void {
    console.info(`${TAG} refreshFoldStatus in. foldStatus: ${foldStatus}. startReason: ${this.startReason}`);
    if (this.startReason === 'from_search') {
      if (foldStatus === display.FoldStatus.FOLD_STATUS_FOLDED) {
        this.isShowBack = true;
        console.info(`${TAG} foldStatus: ${foldStatus}. this.isShowBack: ${this.isShowBack}`);
      } else {
        this.isShowBack = false;
        console.info(`${TAG} foldStatus: ${foldStatus}. this.isShowBack: ${this.isShowBack}`);
      }
    } else {
      this.isShowBack = true;
      console.info(`${TAG} startReason: ${this.startReason}. this.isShowBack: ${this.isShowBack}`);
    }
  }

  private checkFoldBackButton(): void {
    console.info(`${TAG} checkFoldBackButton in`);
    if (display.isFoldable()) {
      try {
        display.on('foldStatusChange', (foldStatus: display.FoldStatus) => {
          let foldStatusValue = foldStatus.valueOf();
          console.info(`${TAG} checkFoldBackButton: foldStatusValue is ${foldStatusValue}`);
          this.refreshFoldStatus(foldStatusValue);
        })
        let data: display.FoldStatus = display.getFoldStatus();
        this.refreshFoldStatus(data);
      } catch (err) {
        console.error(`${TAG} Register failed. exception: ${err.message}`);
      }
    }
  }
}