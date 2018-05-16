/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The method exmaples
 *
 * @author      TCSCODER
 * @version     1.0
 */

var RTValueHelper = RTRemote.RTValueHelper;
var RTValueType = RTRemote.RTValueType;
var createMethodTest = common.createMethodTest;
var helper = RTRemote.helper;

/**
 * get remote object and return method test items
 * @param uri the websocket server uri
 * @param objectNames the object names
 * @return {Promise<T>} the promise with items and remote object
 */
window.createMethodTestExamples = function (uri, objectNames) {
  return RTRemote.RTRemoteConnectionManager.getObjectProxy(uri, objectNames[ 0 ]).then(rtObject => {
    var items = [];
    // test no args passed, and return 10 , rtMethod1ArgAndReturn
    items.push(createMethodTest(rtObject, 'method0AndReturn10', RTValueHelper.create(10, RTValueType.INT32), "noreturn"));

    // test method passed two int and return the sum, rtMethod2ArgAndReturn
    items.push(createMethodTest(rtObject, 'twoIntNumberSum', RTValueHelper.create(123 + 12, RTValueType.INT32), 'return',
      RTValueHelper.create(123, RTValueType.INT32),
      RTValueHelper.create(12, RTValueType.INT32)
    ));

    // test method passed two float and return the sum, rtMethod2ArgAndReturn
    items.push(createMethodTest(
      rtObject, 'twoFloatNumberSum', RTValueHelper.create(123.3 + 12.3, RTValueType.FLOAT), 'return',
      RTValueHelper.create(123.3, RTValueType.FLOAT),
      RTValueHelper.create(12.3, RTValueType.FLOAT),
    ));

    // test method that passed 11 arg and no return, rtMethod1ArgAndNoReturn
    items.push(createMethodTest(rtObject, 'method1IntAndNoReturn', RTValueHelper.create(11, RTValueType.INT32), 'noreturn'));

    // test method that passed RtFunction and invoke this function , rtMethod2ArgAndNoReturn
    items.push(createMethodTest(
      rtObject, 'method2FunctionAndNoReturn', null, 'noreturn',
      RTValueHelper.create((rtValueList) => {
        console.log('function invoke by remote, args count = ' + rtValueList.length);
        rtValueList.forEach((rtValue) => {
          console.log(`value=${rtValue.value}, type=${helper.getTypeStringByType(rtValue.type)}`);
        });
        console.log('function invoke by remote done');
      }, RTValueType.FUNCTION), RTValueHelper.create(10, RTValueType.INT32),
    ));
    return {
      items, rtObject
    };
  });
};
