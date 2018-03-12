/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * this file contains method test examples
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteMulticastResolver = require('../lib/RTRemoteMulticastResolver');
const RTRemoteConnectionManager = require('../lib/RTRemoteConnectionManager');
const RTValueHelper = require('../lib/RTValueHelper');
const RTValueType = require('../lib/RTValueType');
const helper = require('../lib/common/helper');
const logger = require('../lib/common/logger');

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
 * the succeed number of examples
 * @type {number}
 */
let succeed = 0;

resolve.start()
  .then(() => resolve.locateObject('host_object'))
  .then(uri => RTRemoteConnectionManager.getObjectProxy(uri))
  .then((rtObject) => {
    const doTest = () => {
      total = 0;
      succeed = 0;

      Promise.resolve()
      // test no args passed, and return 10 , rtMethod1ArgAndReturn
        .then(() => checkMethod(rtObject, 'method0AndReturn10', RTValueHelper.create(10, RTValueType.INT32)))

        // test method passed two int and return the sum, rtMethod2ArgAndReturn
        .then(() => checkMethod(
          rtObject, 'twoIntNumberSum',
          RTValueHelper.create(123 + 12, RTValueType.INT32),
          RTValueHelper.create(123, RTValueType.INT32),
          RTValueHelper.create(12, RTValueType.INT32),
        ))

        // test method passed two float and return the sum, rtMethod2ArgAndReturn
        .then(() => checkMethod(
          rtObject, 'twoFloatNumberSum',
          RTValueHelper.create(123.3 + 12.3, RTValueType.FLOAT),
          RTValueHelper.create(123.3, RTValueType.FLOAT),
          RTValueHelper.create(12.3, RTValueType.FLOAT),
        ))

        // test method that passed 11 arg and no return, rtMethod1ArgAndNoReturn
        .then(() => checkMethodNoReturn(rtObject, 'method1IntAndNoReturn', RTValueHelper.create(11, RTValueType.INT32)))

        // test method that passed RtFunction and invoke this function , rtMethod2ArgAndNoReturn
        .then(() => checkMethodNoReturn(
          rtObject, 'method2FunctionAndNoReturn',
          RTValueHelper.create((rtValueList) => {
            logger.debug(`function invoke by remote, args count =  + ${rtValueList.length}`);
            rtValueList.forEach((rtValue) => {
              logger.debug(`value=${rtValue.value}, type=${helper.getTypeStringByType(rtValue.type)}`);
            });
            logger.debug('function invoke by remote done');
          }, RTValueType.FUNCTION),
          RTValueHelper.create(10, RTValueType.INT32),
        ))
        .then(() => {
          logger.debug(`========= ${succeed} of ${total} example succeed, ${total - succeed} failed.`);
          logger.debug('test completed, next test will at 10s ...');
          setTimeout(doTest, 10 * 1000);
        });
    };

    doTest();
  })
  .catch(err => logger.error(err));

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
    total += 1;
    if (expectedValue.type === RTValueType.FLOAT) {
      result = checkEqualsFloat(expectedValue.value, rtValue.value);
    } else {
      result = expectedValue.value === rtValue.value;
    }
    succeed += result ? 1 : 0;
    logger[result ? 'debug' : 'error'](`test method ${methodName} result = [${result}]`);
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
    logger.debug(`test method ${methodName} result = [true]`);
    total += 1;
    succeed += 1;
  });
}

/**
 * floating point values can be off by a little bit, so they may not report as exactly equal.
 * so i need use eps to check equal
 */
function checkEqualsFloat(v1, v2) {
  const eps = 0.001;
  return Math.abs(v1 - v2) < eps;
}
