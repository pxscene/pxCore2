/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The multi exmaples
 *
 * @author      TCSCODER
 * @version     1.0
 */

var RTValueHelper = RTRemote.RTValueHelper;
var RTValueType = RTRemote.RTValueType;
var createMethodTest = common.createMethodTest;
var createTypeTest = common.createTypeTest;
var helper = RTRemote.helper;

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
 * get remote object and return multi test items
 * @param uri the websocket server uri
 * @param objectNames the object names
 * @return {Promise<T>} the promise with items and remote object
 */
window.createMultiTestExamples = function (uri, objectNames) {
  return new Promise((resolve, reject) => {
    var objects = [];
    var items = [];
    for (var i = 0; i < objectNames.length; i++) {
      RTRemote.RTRemoteConnectionManager.getObjectProxy(uri, objectNames[ i ]).then(rtObject => {
        const ia = getRandomInt();
        const ib = getRandomInt();
        const a = RTValueHelper.create(ia, RTValueType.INT32);
        const b = RTValueHelper.create(ib, RTValueType.INT32);
        // do get set test
        items.push(createTypeTest(rtObject, RTValueType.INT32, getRandomInt(), 'int32'));
        items.push(createTypeTest(rtObject, RTValueType.INT8, getRandomByte(), 'int8'));
        items.push(createTypeTest(rtObject, RTValueType.INT64, getRandomLong(), 'int64'));
        items.push(createTypeTest(rtObject, RTValueType.STRING, 'SampleString', 'string'));

        // do method test
        items.push(createMethodTest(rtObject, 'twoIntNumberSum', RTValueHelper.create(ia + ib, RTValueType.INT32), 'return', a, b));

        objects.push(rtObject);

        // fetch all remote objects
        if (objects.length === objectNames.length) {
          resolve({ items, rtObject });
        }
      }).catch(err => reject(err));
    }
  });
};
