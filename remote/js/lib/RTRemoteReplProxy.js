/*
 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

/**
 * the rt repl mode remote proxy function
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteMulticastResolver = require('../lib/RTRemoteMulticastResolver');
const RTRemoteConnectionManager = require('../lib/RTRemoteConnectionManager');
const RTValueHelper = require('../lib/RTValueHelper');
const RTValueType = require('../lib/RTValueType');
const logger = require('../lib/common/logger');
const RTPromise = require('./RTPromise');

/**
 * the end function when promise work done
 */
let endFunction = () => {
  process.stdout.write('\n> ');
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
 * unpack rtValue to value
 * @param rtValue the rt value
 */
const unpackRTValue = (rtValue) => {
  if (!rtValue || !rtValue.value) {
    return null;
  }

  let { value } = rtValue;
  switch (rtValue.type) {
    case RTValueType.STRING:
      // try to unpack as json string
      try {
        value = JSON.parse(value);
      } catch (e) {
        // ignore, unpack failed,return string
      }
      break;
    case RTValueType.OBJECT:
      if (value.protocol) { // it is a remote object, put it in proxy
        value = new Proxy(value, proxyHandler);
      }
      break;
    default:
      break;
  }
  return value;
};

/**
 * the rtObject proxy handler
 */
const proxyHandler = {
  get(target, propKey) {
    if (['id', 'protocol', 'then', 'inspect', 'valueOf'] // inner properties
      .findIndex(v => v === propKey) >= 0) {
      return target[propKey];
    }
    if (typeof propKey === 'symbol') { // return self
      return target[propKey];
    }
    return (...args) => {
      let promise = null;
      if (['getProperty', 'get'].indexOf(propKey) >= 0) {
        if (args.length <= 0 || typeof args[0] !== 'string') {
          logger.error('get method only need one parameter, and the type must be string');
          endFunction();
          return Promise.resolve();
        }
        promise = target.get(args[0]);
      } else if (['setProperty', 'set'].indexOf(propKey) >= 0) {
        if (args.length < 2 || typeof args[0] !== 'string') {
          logger.error('set method must be had two parameters, and the first must be string');
          endFunction();
          return Promise.resolve();
        }
        promise = target.set(args[0], wrapperNativeValueToRTValue(args[1]));
      } else { // think it is a method function
        const newArgs = [];
        args.forEach(v => newArgs.push(wrapperNativeValueToRTValue(v)));
        promise = target.send(propKey, ...newArgs);
      }
      return new RTPromise(promise, propKey).then(rtValue => unpackRTValue(rtValue))
        .catch((err) => {
          logger.error(`invoke remote method ${propKey} failed, error  = ${err}`);
          endFunction();
        });
    };
  },
};

module.exports = {

  /**
   * locate object
   * @param objectId the object id
   * @param options the connection options {multicastAddress:,multicastPort:}
   */
  locateObject: (objectId, options) => {
    options = options || {};
    options.multicastAddress = options.multicastAddress || '224.10.10.12';
    options.multicastPort = options.multicastPort || '10004';

    return new RTPromise(new Promise(((resolve, reject) => {
      const resolver = new RTRemoteMulticastResolver(options.multicastAddress, options.multicastPort);
      return resolver.start()
        .then(() => resolver.locateObject(objectId))
        .then(uri => RTRemoteConnectionManager.getObjectProxy(uri))
        .then((rtObject) => {
          endFunction();
          resolve(new Proxy(rtObject, proxyHandler)); // wrapper rtObject into proxy
        })
        .catch(err => reject(err));
    })), 'locateObject');
  },
  /**
   * send repl end function
   * @param cb
   * @return {null}
   */
  setEndFunction: (cb) => {
    endFunction = cb;
    return null;
  },
};
