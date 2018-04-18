/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * this file contains all type test examples
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteMulticastResolver = require('../lib/RTRemoteMulticastResolver');
const RTRemoteConnectionManager = require('../lib/RTRemoteConnectionManager');
const RTValueHelper = require('../lib/RTValueHelper');
const RTValueType = require('../lib/RTValueType');
const RTConst = require('../lib/RTConst');
const logger = require('../lib/common/logger');
const helper = require('../lib/common/helper');
const BigNumber = require('bignumber.js');

/**
 * the RTRemoteMulticastResolver instance
 * @type {RTRemoteMulticastResolver}
 */
const resolve = new RTRemoteMulticastResolver('224.10.10.12', 10004);

/**
 * the total number of examples
 * @type {number}
 */
let total = 0;

/**
 * the successful number of examples
 * @type {number}
 */
let succeed = 0;

resolve.start()
  .then(() => resolve.locateObject('host_object')) // find remote object
  .then(uri => RTRemoteConnectionManager.getObjectProxy(uri)) // crate remote object
  .then((rtObject) => {
    const doTest = () => {
      total = 0;
      succeed = 0;

      Promise.resolve() // test all type in sequence mode

      // function test
        .then(() => doFunctionTest(rtObject, 'onTick'))

        // object test
        .then(() => doObjectTest(rtObject, 'objvar'))

        // in c++/java, float only had 7 valid digits
        .then(() => doBasicTest(rtObject, RTValueType.FLOAT, 1.23456789, 'ffloat'))
        .then(() => doBasicTest(rtObject, RTValueType.FLOAT, -1.234567, 'ffloat'))
        .then(() => doBasicTest(rtObject, RTValueType.FLOAT, 12.34567, 'ffloat'))
        .then(() => doBasicTest(rtObject, RTValueType.FLOAT, 123.4567, 'ffloat'))
        .then(() => doBasicTest(rtObject, RTValueType.FLOAT, 1234.567, 'ffloat'))
        .then(() => doBasicTest(rtObject, RTValueType.FLOAT, 123456.71, 'ffloat'))

        // test bool
        .then(() => doBasicTest(rtObject, RTValueType.BOOLEAN, true, 'bbool'))
        .then(() => doBasicTest(rtObject, RTValueType.BOOLEAN, false, 'bbool'))

        // int8 range [-128,127]
        .then(() => doBasicTest(rtObject, RTValueType.INT8, -128, 'int8'))
        .then(() => doBasicTest(rtObject, RTValueType.INT8, 0, 'int8'))
        .then(() => doBasicTest(rtObject, RTValueType.INT8, 127, 'int8'))

        // test uint8, the data range is[0,255]
        .then(() => doBasicTest(rtObject, RTValueType.UINT8, 0, 'uint8'))
        .then(() => doBasicTest(rtObject, RTValueType.UINT8, 255, 'uint8'))

        // test int32, range is  [–2147483648 , 2147483647]
        .then(() => doBasicTest(rtObject, RTValueType.INT32, -2147483648, 'int32'))
        .then(() => doBasicTest(rtObject, RTValueType.INT32, 0, 'int32'))
        .then(() => doBasicTest(rtObject, RTValueType.INT32, 123, 'int32'))
        .then(() => doBasicTest(rtObject, RTValueType.INT32, 2147483647, 'int32'))

        // test uint32, range is [0 - 4,294,967,295]
        .then(() => doBasicTest(rtObject, RTValueType.UINT32, 0, 'uint32'))
        .then(() => doBasicTest(rtObject, RTValueType.UINT32, 4294967295, 'uint32'))
        .then(() => doBasicTest(rtObject, RTValueType.UINT32, 123123, 'uint32'))

        // test int64, range is [–9223372036854775808  9223372036854775807]
        .then(() => doBasicTest(rtObject, RTValueType.INT64, new BigNumber('-9223372036854775808'), 'int64'))
        .then(() => doBasicTest(rtObject, RTValueType.INT64, new BigNumber('9223372036854775807'), 'int64'))
        .then(() => doBasicTest(rtObject, RTValueType.INT64, new BigNumber(0), 'int64'))
        .then(() => doBasicTest(rtObject, RTValueType.INT64, new BigNumber(123123), 'int64'))

        // test uint64, range is [0 - 18446744073709551615]
        .then(() => doBasicTest(rtObject, RTValueType.UINT64, new BigNumber('18446744073709551615'), 'uint64'))
        .then(() => doBasicTest(rtObject, RTValueType.UINT64, new BigNumber(0), 'uint64'))
        .then(() => doBasicTest(rtObject, RTValueType.UINT64, new BigNumber(123123123), 'uint64'))
        .then(() => doBasicTest(rtObject, RTValueType.UINT64, new BigNumber('123'), 'uint64'))

        // test double
        .then(() => doBasicTest(rtObject, RTValueType.DOUBLE, 1231.12312312312, 'ddouble'))
        .then(() => doBasicTest(rtObject, RTValueType.DOUBLE, -1231.12312312312, 'ddouble'))
        .then(() => doBasicTest(rtObject, RTValueType.DOUBLE, -0.12, 'ddouble'))

        // test string
        .then(() => doBasicTest(
          rtObject, RTValueType.STRING,
          'implemented in both /Library/Java/JavaVirtualMachines/jdk1.8.0_40.jdk/Conten', 'string',
        ))
        .then(() => doBasicTest(
          rtObject, RTValueType.STRING,
          '{"jsonKey":"values"}', 'string',
        ))
        .then(() => doBasicTest(rtObject, RTValueType.STRING, '1', 'string'))

        // void ptr is a uint32 or uint64
        .then(() => doBasicTest(rtObject, RTValueType.VOIDPTR, new BigNumber('723123231'), 'vptr'))
        .then(() => doBasicTest(rtObject, RTValueType.VOIDPTR, new BigNumber('789892349'), 'vptr'))
        .then(() => {
          logger.debug(`========= ${succeed} of ${total} example succeed, ${total - succeed} failed.`);
          logger.debug('test completed, next test will at 10s ...');
          setTimeout(doTest, 10 * 1000);
        })
        .catch(err => logger.error(err));
    };
    doTest();
  })
  .catch((err) => {
    logger.error(err);
  });

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
    printResult(type, value, rtValue.value, result);
  })).catch((err) => {
    logger.error(err);
  });
}

/**
 * print result and add total/success example number
 * @param type the rtValue type
 * @param old the old value
 * @param newValue the rpc value
 * @param result the test result
 */
function printResult(type, old, newValue, result) {
  total += 1;
  succeed += result ? 1 : 0;
  logger[result ? 'debug' : 'error'](`${helper.getTypeStringByType(type)} test => set val = ${old}, rpc result = ${
    newValue}, passed = [${result}]`);
}

/**
 * do function test
 * @param rtObject the remote object
 * @param propertyName the property name
 * @return {Promise<void>} the promise with void
 */
function doFunctionTest(rtObject, propertyName) {
  const oldRtValue = RTValueHelper.create((rtValueList) => {
    logger.debug('doFunctionTest test...');
    logger.debug(rtValueList);
  }, RTValueType.FUNCTION);

  return rtObject.set(propertyName, oldRtValue).then(() => rtObject.get(propertyName).then((rtValue) => {
    rtValue.value(null);
    const result = oldRtValue[RTConst.FUNCTION_KEY] === rtValue[RTConst.FUNCTION_KEY];
    printResult(RTValueType.FUNCTION, oldRtValue[RTConst.FUNCTION_KEY], rtValue[RTConst.FUNCTION_KEY], result);
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
      logger.debug('hello from test obj');
    },
  };

  const rtOldObj = RTValueHelper.create(testObj, RTValueType.OBJECT);
  return rtObject.set(propertyName, rtOldObj).then(() => rtObject.get(propertyName).then((rtValue) => {
    const oldObjId = rtOldObj[RTConst.VALUE][RTConst.OBJECT_ID_KEY];
    const newObjId = rtValue[RTConst.VALUE][RTConst.OBJECT_ID_KEY];
    rtValue.value.hello();
    printResult(RTValueType.OBJECT, oldObjId, newObjId, oldObjId === newObjId);
  }));
}
