/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the rt remote proxy function
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteMulticastResolver = require('../lib/RTRemoteMulticastResolver');
const RTRemoteConnectionManager = require('../lib/RTRemoteConnectionManager');
const RTValueHelper = require('../lib/RTValueHelper');
const RTValueType = require('../lib/RTValueType');

/**
 * the end function when promise work done
 */
let endFunction = () => {
};


/**
 * warpper native value to rtValue
 * @param v the native value
 * @return {object} the rtValue
 */
const wrapperNativeValueToRTValue = (v) => {
  const isInt = n => Number(n) === n && n % 1 === 0;
  const isFloat = n => Number(n) === n && n % 1 !== 0;
  if (isInt(v)) {
    return RTValueHelper.create(v, RTValueType.INT32);
  } else if (isFloat(v)) {
    return RTValueHelper.create(v, RTValueType.FLOAT);
  } else if (typeof v === 'string') {
    return RTValueHelper.create(v, RTValueType.STRING);
  }
  throw new Error(`cannot identify rtValue type for ${v}`);
};

/**
 * the rtObject proxy handler
 */
const proxyHandler = {
  get(target, propKey) {
    if (['id', 'protocol', 'then'].findIndex(v => v === propKey) >= 0) {
      return target[propKey];
    }
    return (...args) => {
      const newArgs = [];
      args.forEach(v => newArgs.push(wrapperNativeValueToRTValue(v)));
      return target.send(propKey, ...newArgs)
        .then((rtValue) => {
          endFunction();
          return Promise.resolve(rtValue);
        })
        .catch((err) => {
          console.error(`invoke method ${propKey} failed, error  = ${err}`);
          endFunction();
        });
    };
  },
};

module.exports = {
  connect: (host, port, objectId) => {
    const resolve = new RTRemoteMulticastResolver(host, port);
    return resolve.start()
      .then(() => resolve.locateObject(objectId))
      .then(uri => RTRemoteConnectionManager.getObjectProxy(uri))
      .then((rtObject) => {
        endFunction();
        return Promise.resolve(new Proxy(rtObject, proxyHandler)); // wrapper rtObject into proxy
      });
  },
  setEndFunction: (cb) => {
    endFunction = cb;
    return null;
  },
};
