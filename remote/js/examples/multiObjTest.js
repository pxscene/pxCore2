/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * this file contains multi objects test examples
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
const BigNumber = require('bignumber.js');

/**
 * the RTRemoteMulticastResolver instance
 * @type {RTRemoteMulticastResolver}
 */
const resolve = new RTRemoteMulticastResolver('224.10.10.12', 10004);

resolve.start().then(() => {
  const objs = ['host_object', 'obj2', 'obj3', 'obj4']; // 4 objects test examples in parallel mode
  for (let i = 0; i < objs.length; i += 1) {
    resolve.locateObject(objs[i]).then(uri => RTRemoteConnectionManager.getObjectProxy(uri)).then((rtObject) => {
      const doTest = () => {
        const ia = getRandomInt();
        const ib = getRandomInt();
        const a = RTValueHelper.create(ia, RTValueType.INT32);
        const b = RTValueHelper.create(ib, RTValueType.INT32);

        Promise.resolve()
          .then(() => doBasicTest(rtObject, RTValueType.INT32, getRandomInt(), 'int32'))

          // do get set test
          .then(() => doBasicTest(rtObject, RTValueType.INT32, getRandomInt(), 'int32'))
          .then(() => doBasicTest(rtObject, RTValueType.INT8, getRandomByte(), 'int8'))
          .then(() => doBasicTest(rtObject, RTValueType.INT64, getRandomLong(), 'int64'))
          .then(() => doBasicTest(rtObject, RTValueType.STRING, 'SampleString', 'string'))

          // do method test
          .then(() => checkMethod(
            rtObject, 'twoIntNumberSum', RTValueHelper.create(ia + ib, RTValueType.INT32), a,
            b,
          ))
          .then(() => {
            logger.debug(` =========> ${rtObject.id} test completed, next test will at 10s ...`);
            setTimeout(doTest, 10 * 1000);
          });
      };
      doTest();
    }).catch(err => logger.err(err));
  }
}).catch(err => logger.error(err));

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
    if (type === RTValueType.UINT64 || type === RTValueType.INT64) {
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
 * get random Integer value
 *
 * @return {number | int} the Integer value
 */
function getRandomInt() {
  return Math.ceil(Math.random() * 10000000.0);
}

/**
 * get random Short value
 *
 * @return {number} the Short value
 */
function getRandomByte() {
  return Math.ceil(Math.random() * 120);
}

/**
 * get random Long value
 *
 * @return {number} the Long value
 */
function getRandomLong() {
  return new BigNumber(Date.now() + Math.ceil(Math.random() * 100));
}

/**
 * print result and add total/suceed example number
 * @param type the rtValue type
 * @param old the old value
 * @param newValue the rpc value
 * @param result the test result
 */
function printResult(type, old, newValue, result) {
  logger[result ? 'debug' : 'error'](`${helper.getTypeStringByType(type)} test => set val = ${old}, rpc result = ${
    newValue}, passed = [${result}]`);
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
    const result = expectedValue.value === rtValue.value;
    logger[result ? 'debug' : 'error'](`test method ${methodName} result = [${result}]`);
  });
}
