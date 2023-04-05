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
import { describe, beforeAll, beforeEach, afterEach, afterAll, it, expect } from 'deccjsunit/index';
import pasteboard from '@ohos.pasteboard';
import image from '@ohos.multimedia.image';

describe('PasteBoardJSTest', function () {
  beforeAll(async function () {
    console.info('beforeAll');
  });

  afterAll(async function () {
    console.info('afterAll');
  });

  /**
   * @tc.name      pasteboard_function_test1
   * @tc.desc      createPlainTextData test
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_function_test1', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test1: systemPasteboard.clear callback error:' + err);
      } else {
        var textData = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test1: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test1: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test1: systemPasteboard.getPasteData callback error:' + err);
                  } else {
                    expect(data.getRecordCount()).assertEqual(1);
                    var primaryText = data.getPrimaryText();
                    expect(primaryText).assertEqual(textData);
                    expect(pasteboard.MAX_RECORD_NUM).assertEqual(512);
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
  });

  /**
   * @tc.name      pasteboard_function_test2
   * @tc.desc      createHtmlData test
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_function_test2', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test2: systemPasteboard.clear callback error:' + err);
      } else {
        var htmlText = '<html><head></head><body>Hello World!</body></html>';
        var pasteData = pasteboard.createHtmlData(htmlText);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test2: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test2: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test2: systemPasteboard.getPasteData callback error:' + err);
                  } else {
                    expect(data.getRecordCount()).assertEqual(1);
                    var PrimaryHtml = data.getPrimaryHtml();
                    console.info('f_test2: PrimaryHtml = ' + PrimaryHtml);
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
  });

  /**
   * @tc.name      pasteboard_function_test3
   * @tc.desc      测试异步callback调用+createHtmlData,htmlText = ''.
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_function_test3', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test3: systemPasteboard.clear callback error:' + err);
      } else {
        var htmlText = '';
        var pasteData = pasteboard.createHtmlData(htmlText);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test3: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test3: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test3: systemPasteboard.getPasteData callback error:' + err);
                  } else {
                    expect(data.getRecordCount()).assertEqual(1);
                    var PrimaryHtml = data.getPrimaryHtml();
                    console.info('f_test3: PrimaryHtml = ' + PrimaryHtml);
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
  });

  /**
   * @tc.name      pasteboard_function_test4
   * @tc.desc      createUriData test
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_function_test4', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test4: systemPasteboard.clear callback error:' + err);
      } else {
        var uriText = '';
        var pasteData = pasteboard.createUriData(uriText);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test4: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test4: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test4: systemPasteboard.getPasteData callback error:' + err);
                  } else {
                    expect(data.getRecordCount()).assertEqual(1);
                    var PrimaryUri = data.getPrimaryUri();
                    console.info('f_test4: PrimaryUri = ' + PrimaryUri);
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
  });

  /**
   * @tc.name      pasteboard_function_test5
   * @tc.desc      createWantData test
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_function_test5', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test5: systemPasteboard.clear callback error:' + err);
      } else {
        var want = {
          bundleName: 'com.example.myapplication8',
          abilityName: 'com.example.myapplication8.MainAbility',
        };
        var pasteData = pasteboard.createWantData(want);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test5: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test5: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test5: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test6
   * @tc.desc      addTextRecord test
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_function_test6', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test6: systemPasteboard.clear callback error:' + err);
      } else {
        var textData0 = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData0);
        var textData1 = 'Hello World1';
        pasteData.addTextRecord(textData1);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test6: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test6: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test6: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test7
   * @tc.desc      addTextRecord test
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_function_test7', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test7: systemPasteboard.clear callback error:' + err);
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
            console.error('f_test7: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test7: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test7: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test8
   * @tc.desc      addHtmlRecord+addUriRecord+addWantRecord
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_function_test8', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test8: systemPasteboard.clear callback error:' + err);
      } else {
        var textData = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData);
        var htmlText = '<html><head></head><body>Hello World!</body></html>';
        pasteData.addHtmlRecord(htmlText);
        var uriText = '';
        pasteData.addUriRecord(uriText);
        var want = {
          bundleName: 'com.example.myapplication8',
          abilityName: 'com.example.myapplication8.MainAbility',
        };
        pasteData.addWantRecord(want);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test8: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test8: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test8: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test9
   * @tc.desc      addHtmlRecord+addUriRecord+removeRecordAt
   * @tc.type      Function
   * @tc.require   AR000HEECD
   */
  it('pasteboard_function_test9', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test9: systemPasteboard.clear callback error:' + err);
      } else {
        var textData = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData);
        var htmlText = '<html><head></head><body>Hello World!</body></html>';
        pasteData.addHtmlRecord(htmlText);
        var uriText = '';
        pasteData.addUriRecord(uriText);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test9: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test9: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test9: systemPasteboard.getPasteData callback error:' + err);
                  } else {
                    expect(data.getRecordCount()).assertEqual(3);
                    expect(data.removeRecordAt(0)).assertEqual(true);
                    expect(data.getRecordCount()).assertEqual(2);
                    systemPasteboard.setPasteData(data, (err, newdata) => {
                      if (err) {
                        console.error('f_test9: systemPasteboard.setPasteData callback error:' + err);
                      } else {
                        systemPasteboard.getPasteData((err, data) => {
                          if (err) {
                            console.error('f_test9: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test10
   * @tc.desc      Add 30 TextRecords
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test10', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test10: systemPasteboard.clear callback error:' + err);
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
            console.error('f_test10: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test10: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test10: systemPasteboard.getPasteData callback error:' + err);
                  } else {
                    expect(data.getRecordCount()).assertEqual(30);
                    for (var i = 0; i < 30; i++) {
                      expect(data.removeRecordAt(0)).assertEqual(true);
                    }
                    expect(data.getRecordCount()).assertEqual(0);
                    systemPasteboard.setPasteData(data, (err, newdata) => {
                      if (err) {
                        console.error('f_test10: systemPasteboard.setPasteData callback error:' + err);
                      } else {
                        systemPasteboard.getPasteData((err, data) => {
                          if (err) {
                            console.error('f_test10: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test11
   * @tc.desc      Replcae textRecord
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test11', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test11: systemPasteboard.clear callback error:' + err);
      } else {
        var textData = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test11: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test11: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test11: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test12
   * @tc.desc      Replcae htmlRecord
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test12', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test12: systemPasteboard.clear callback error:' + err);
      } else {
        var htmlText = '<html><head></head><body>Hello World!</body></html>';
        var pasteData = pasteboard.createHtmlData(htmlText);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test12: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test12: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test12: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test13
   * @tc.desc      Replcae wantRecord
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test13', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test13: systemPasteboard.clear callback error:' + err);
      } else {
        var wantText0 = {
          bundleName: 'com.example.myapplication3',
          abilityName: 'com.example.myapplication3.MainAbility',
        };
        var pasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_WANT, wantText0);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test13: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test13: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test13: systemPasteboard.getPasteData callback error:' + err);
                  } else {
                    expect(data.getRecordCount()).assertEqual(1);
                    var wantText1 = {
                      bundleName: 'com.example.myapplication30',
                      abilityName: 'com.example.myapplication30.MainAbility',
                    };
                    var pasteDataRecord = pasteboard.createRecord(pasteboard.MIMETYPE_TEXT_WANT, wantText1);
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
  });

  /**
   * @tc.name      pasteboard_function_test14
   * @tc.desc      get pasteData after clear wantData
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test14', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test14: systemPasteboard.clear callback error:' + err);
      } else {
        var wantText0 = {
          bundleName: 'com.example.myapplication3',
          abilityName: 'com.example.myapplication3.MainAbility',
        };
        var pasteData = pasteboard.createWantData(wantText0);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test14: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test14: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test14: systemPasteboard.getPasteData callback error:' + err);
                  } else {
                    expect(data.getRecordCount()).assertEqual(1);
                    systemPasteboard.clear((err, data) => {
                      if (err) {
                        console.error('f_test14: systemPasteboard.clear callback error:' + err);
                      } else {
                        systemPasteboard.getPasteData((err, data) => {
                          if (err) {
                            console.error('f_test14: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test15
   * @tc.desc      getProperty and setProperty test
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test15', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test15: systemPasteboard.clear callback error:' + err);
      } else {
        var textData = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test15: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test15: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test15: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test16
   * @tc.desc      on test
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test16', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test16: systemPasteboard.clear callback error:' + err);
      } else {
        systemPasteboard.on('update', contentChanges);
        var textData = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test16: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test16: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data === true).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test16: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test17
   * @tc.desc      off test
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test17', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test17: systemPasteboard.clear callback error:' + err);
      } else {
        systemPasteboard.off('update', contentChanges);
        var textData = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test17: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test17: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test17: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test18
   * @tc.desc      createRecord test
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test18', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear().then(() => {
      var buffer = new ArrayBuffer(128);
      var opt = {
        size: { height: 5, width: 5 },
        pixelFormat: 3,
        editable: true,
        alphaType: 1,
        scaleMode: 1,
      };
      var pasteData = pasteboard.createHtmlData('application/xml');
      image.createPixelMap(buffer, opt).then((pixelMap) => {
        expect(pixelMap.getPixelBytesNumber()).assertEqual(100);
        var pixelMapRecord = pasteboard.createRecord(pasteboard.MIMETYPE_PIXELMAP, pixelMap);
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
  });

  /**
   * @tc.name      pasteboard_function_test19
   * @tc.desc      Add plainText、htmlText、uriText record
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test19', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test19: systemPasteboard.clear callback error:' + err);
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
            console.error('f_test19: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test19: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData(async (err, data) => {
                  if (err) {
                    console.error('f_test19: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test20
   * @tc.desc      convertToText test
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test20', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test20: systemPasteboard.clear callback error:' + err);
      } else {
        var textData = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test20: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test20: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData(async (err, data) => {
                  if (err) {
                    console.error('f_test20: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test21
   * @tc.desc      convertToText test
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test21', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test21: systemPasteboard.clear callback error:' + err);
      } else {
        var textData = 'Hello World!';
        var pasteData = pasteboard.createPlainTextData(textData);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test21: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test21: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData((err, data) => {
                  if (err) {
                    console.error('f_test21: systemPasteboard.getPasteData callback error:' + err);
                  } else {
                    expect(data.getRecordCount()).assertEqual(1);
                    var dataRecord = data.getRecordAt(0);
                    dataRecord.convertToText((err, data) => {
                      if (err) {
                        console.error('f_test21: PasteDataRecord.convertToText callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test22
   * @tc.desc      convertToText test
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test22', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test22: systemPasteboard.clear callback error:' + err);
      } else {
        var textData = 'Hello 中国!@#$%^&*()_+{}\\?.';
        var pasteData = pasteboard.createPlainTextData(textData);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test22: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test22: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData(async (err, data) => {
                  if (err) {
                    console.error('f_test22: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   * @tc.name      pasteboard_function_test23
   * @tc.desc      convertToText test
   * @tc.type      Function
   * @tc.require   AR000H5GKU
   */
  it('pasteboard_function_test23', 0, async function (done) {
    var systemPasteboard = pasteboard.getSystemPasteboard();
    systemPasteboard.clear((err, data) => {
      if (err) {
        console.error('f_test23: systemPasteboard.clear callback error:' + err);
      } else {
        var uriText = 'https://www.baidu.com/';
        var pasteData = pasteboard.createUriData(uriText);
        systemPasteboard.setPasteData(pasteData, (err, data) => {
          if (err) {
            console.error('f_test23: systemPasteboard.setPasteData callback error:' + err);
          } else {
            systemPasteboard.hasPasteData((err, data) => {
              if (err) {
                console.error('f_test23: systemPasteboard.hasPasteData callback error:' + err);
              } else {
                expect(data).assertEqual(true);
                systemPasteboard.getPasteData(async (err, data) => {
                  if (err) {
                    console.error('f_test23: systemPasteboard.getPasteData callback error:' + err);
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
  });

  /**
   *  The callback function is used for pasteboard content changes
   */
  function contentChanges() {
    console.info('#EVENT: The content is changed in the pasteboard');
  }
});
