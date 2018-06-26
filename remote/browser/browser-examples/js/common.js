/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * test common functions
 *
 * @author      TCSCODER
 * @version     1.0
 */

var RTValueHelper = RTRemote.RTValueHelper;
var RTValueType = RTRemote.RTValueType;
var helper = RTRemote.helper;
var RTConst = RTRemote.RTConst;
var messageTimerHander = null;

/**
 * floating point values can be off by a little bit, so they may not report as exactly equal.
 * so i need use eps to check equal
 */
function checkEqualsFloat(v1, v2) {
  const eps = 0.001;
  return Math.abs(v1 - v2) < eps;
}

/**
 * double values can be off by a little bit, so they may not report as exactly equal.
 */
function checkEqualsDouble(v1, v2) {
  const eps = 0.0001;
  return Math.abs(v1 - v2) < eps;
}

/**
 * do basic test example
 * @param rtObject the remote object
 * @param type the rtValue type
 * @param value the value
 * @param propertyName the property name
 * @return {Promise<void>} the promise with void
 */
function doBasicTest(rtObject, type, value, propertyName) {
  return rtObject.set(propertyName, RTValueHelper.create(value, type)).then(() => rtObject.get(propertyName).then((rtValue) => {
    let result = false;
    if (type === RTValueType.FLOAT) {
      result = checkEqualsFloat(rtValue.value, value);
    } else if (type === RTValueType.DOUBLE) {
      result = checkEqualsDouble(rtValue.value, value);
    } else if (type === RTValueType.UINT64 || type === RTValueType.INT64 || type === RTValueType.VOIDPTR) {
      result = value.toString() === rtValue.value.toString(); //
    } else {
      result = rtValue.value === value;
    }
    return {
      type,
      source: value,
      response: rtValue.value,
      result: result
    }
  }));
}

/**
 * do basic test with index
 * @param rtObject the remote object
 * @param type the rt value type
 * @param value the value
 * @param index the index
 */
function doBasicTestWithIndex(rtObject, type, value, index) {
  return rtObject.get('arr') // get array object first
    .then(arrValue => arrValue.value.set(index, RTValueHelper.create(value, type)).then(() => arrValue))
    .then(arrValue => arrValue.value.get(index).then((rtValue) => {
      let result = false;
      if (type === RTValueType.FLOAT) {
        result = checkEqualsFloat(rtValue.value, value);
      } else {
        result = rtValue.value === value;
      }
      return {
        type,
        source: value,
        response: rtValue.value,
        result: result
      }
    }));
}

/**
 * do object test
 * @param rtObject the remote object
 * @param propertyName the property name
 * @return {Promise<void>} the promise with void
 */
function doObjectTest(rtObject, propertyName) {
  const testObj = {
    hello: () => {
      console.log('hello from test obj');
    },
  };
  const rtOldObj = RTValueHelper.create(testObj, RTValueType.OBJECT);
  return rtObject.set(propertyName, rtOldObj).then(() => rtObject.get(propertyName).then((rtValue) => {
    const oldObjId = rtOldObj[ RTConst.VALUE ][ RTConst.OBJECT_ID_KEY ];
    const newObjId = rtValue[ RTConst.VALUE ][ RTConst.OBJECT_ID_KEY ];
    rtValue.value.hello();
    return {
      type: RTValueType.OBJECT,
      source: oldObjId,
      response: newObjId,
      result: oldObjId === newObjId,
    }
  }));
}

/**
 * do function test
 * @param rtObject the remote object
 * @param propertyName the property name
 * @return {Promise<void>} the promise with void
 */
function doFunctionTest(rtObject, propertyName) {
  const oldRtValue = RTValueHelper.create((rtValueList) => {
    console.log('doFunctionTest test...');
    console.log(rtValueList);
  }, RTValueType.FUNCTION);
  
  return rtObject.set(propertyName, oldRtValue).then(() => rtObject.get(propertyName).then((rtValue) => {
    rtValue.value.value(null);
    const result = oldRtValue[ RTConst.VALUE ][ RTConst.FUNCTION_KEY ] === rtValue[ RTConst.VALUE ][ RTConst.FUNCTION_KEY ];
    return {
      type: RTValueType.FUNCTION,
      source: oldRtValue[ RTConst.FUNCTION_KEY ],
      response: rtValue[ RTConst.VALUE ][ RTConst.FUNCTION_KEY ],
      result: result,
    };
  }));
}

/**
 * check method returned rtValue is expected or not
 * @param rtObject the remote object
 * @param methodName the method name
 * @param expectedValue the expected rtValue
 * @param args the call function args
 * @return {Promise<void>} the promise when done
 */
function checkMethod(rtObject, methodName, expectedValue, ...args) {
  return rtObject.sendReturns(methodName, ...args).then((rtValue) => {
    let result = false;
    if (expectedValue.type === RTValueType.FLOAT) {
      result = checkEqualsFloat(expectedValue.value, rtValue.value);
    } else {
      result = expectedValue.value === rtValue.value;
    }
    return {
      methodName: methodName,
      expected: expectedValue.value,
      response: rtValue.value,
      result: result,
    };
  });
}

/**
 * check no returns method
 * @param rtObject the remote object
 * @param methodName the method name
 * @param args the call function args
 * @return {Promise<void>} the promise when done
 */
function checkMethodNoReturn(rtObject, methodName, ...args) {
  return rtObject.send(methodName, ...args).then(() => {
    return {
      methodName: methodName,
      result: true,
    };
  });
}

/**
 * create set/get type test item
 * @param rtObject the remote object
 * @param type the test value type
 * @param value the value
 * @param propertyName the test property name
 * @param testType the test type, 4 types, 'basic','index','object','function'
 * @return {Object} the test item entity
 */
function createTypeTest(rtObject, type, value, propertyName, testType) {
  testType = testType || 'basic';
  var pName = testType === 'index' ? ('arr.' + propertyName) : propertyName;
  var item = {
    title: rtObject.id + ' - test set/get type = ' + helper.getTypeStringByType(type) + ' for property ' + pName,
    description: 'Test with value <strong> ' + (value ? value : '') + '</strong> for property ' + pName,
    additionText: '',
    status: 'pending'
  };
  
  function warpperItem(dItem, rsp) {
    dItem.status = rsp.result ? 'passed' : 'failed';
    dItem.additionText = ', response value = <strong>' + rsp.response + '</strong>';
    return dItem;
  }
  
  if (testType === 'basic' || testType === 'index') {
    item.task = (dItem) => {
      return new Promise((resolve, reject) => {
        return (testType === 'index' ? doBasicTestWithIndex : doBasicTest)
        (rtObject, type, value, propertyName).then(rsp => {
          resolve(warpperItem(dItem, rsp));
        }).catch(err => {
          reject({ err: err, item: dItem });
        });
      });
    };
  } else if (testType === 'object' || testType === 'function') {
    item.task = (dItem) => {
      return new Promise((resolve, reject) => {
        return (testType === 'object' ? doObjectTest : doFunctionTest)(rtObject, propertyName).then(rsp => {
          resolve(warpperItem(dItem, rsp));
        }).catch(err => {
          reject({ err: err, item: dItem });
        });
      });
    };
  }
  return item;
}

/**
 * create method test item
 * @param rtObject  the remote object
 * @param methodName the method name
 * @param expectedValue the method returned exptected value
 * @param testType the test type , 'return' and 'noreturn'
 * @param args the method args
 * @return {Object} the test item entity
 */
function createMethodTest(rtObject, methodName, expectedValue, testType, ...args) {
  var description = '';
  if (testType === 'return') {
    description = "Test invoke method " + methodName + ', the expected value is <strong>' + expectedValue.value + '</strong>';
  } else {
    description = "Test invoke method with no returned value";
  }
  var item = {
    title: rtObject.id + ' - test invoke method <strong>' + methodName + '</strong>',
    description: description,
    additionText: '',
    status: 'pending'
  };
  
  if (testType === 'return') {
    item.task = (dItem) => {
      return new Promise((resolve, reject) => {
        checkMethod(rtObject, methodName, expectedValue, ...args).then(rsp => {
          dItem.status = rsp.result ? 'passed' : 'failed';
          dItem.additionText = ', the returned value is <strong>' + rsp.response + '</strong>';
          resolve(dItem);
        }).catch(err => {
          reject({ err: err, item: dItem });
        });
      });
    };
  } else {
    item.task = (dItem) => {
      return new Promise((resolve, reject) => {
        checkMethodNoReturn(rtObject, methodName, ...args).then(rsp => {
          dItem.status = rsp.result ? 'passed' : 'failed';
          dItem.additionText = ', succeed with no returned value';
          resolve(dItem);
        }).catch(err => {
          reject({ err: err, item: dItem });
        });
      });
    };
  }
  return item;
}

function showMessage(type, message) {
  
  function clear() {
    $('#message-root').remove();
    if (messageTimerHander) {
      clearTimeout(messageTimerHander);
      messageTimerHander = null;
    }
  }
  
  clear();
  
  var node = $('  <div id="message-root" class="alert-body alert alert-' + (type || 'danger') + '" role="alert">\n' + message +
    '\n' +
    '    <button type="button" class="close" data-dismiss="modal" aria-label="Close">\n' +
    '      <span aria-hidden="true">&times;</span>\n' +
    '    </button>\n' +
    '  </div>');
  node.find('.close').click(function () {
    clear();
  });
  $('main').prepend(node);
  messageTimerHander = setTimeout(() => clear(), 8000);
}

window.common = {
  createTypeTest, createMethodTest, showMessage
};
