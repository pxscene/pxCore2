/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * This module contains some common helper methods
 *
 * @author      TCSCODER
 * @version     1.0
 */

const uuidV4 = require('uuid/v4');
const RTStatusCode = require('../RTStatusCode');
const RTValueType = require('../RTValueType');
const RTConst = require('../RTConst');
const RTEnvironment = require('../RTEnvironment');
const RTRemoteMessageType = require('../RTRemoteMessageType');
const logger = require('./logger');

/**
 * the RTMessageHelper module
 * @type {RTMessageHelper}
 */
let rtMessageHelper = null;

/**
 * the array object hash map
 * @type {object|map}
 */
const arrayObjectHashmap = {};

/**
 * get a random uuid
 * @return {string} the random uuid value
 */
function getRandomUUID() {
  return uuidV4();
}

/**
 * get status string from status code
 * @param statusCode the status code
 * @return {string} the status code name
 */
function getStatusStringByCode(statusCode) {
  for (const key in RTStatusCode) {  // eslint-disable-line
    if (RTStatusCode[key] === statusCode) {
      return key;
    }
  }
  return RTConst.UNKNOWN_CODE;
}

/**
 * get type name string by type value
 * @param type the type value
 * @return {string} the type name
 */
function getTypeStringByType(type) {
  for (const key in RTValueType) {  // eslint-disable-line
    if (RTValueType[key] === type) {
      return key;
    }
  }
  return RTConst.UNKNOWN_TYPE;
}

/**
 * check the object is a big number
 * @param {object} obj the object
 * @return {*} the result true/false
 */
function isBigNumber(obj) {
  return !!obj && obj.isBigNumber;
}

/**
 * get the rt value type by object
 * @param {object} obj the object
 * @return {RTValueType} the result type
 */
function getValueType(obj) {
  if (!!obj && obj[RTConst.TYPE] && getTypeStringByType(obj.type) !== RTConst.UNKNOWN_TYPE) {
    return obj.type;
  }
  return null;
}

/**
 * check the dump object
 * @param {string} objectName the object name
 * @param {object} object the object entity
 */
function checkAndDumpObject(objectName, object) {
  for (const key in object) { // eslint-disable-line
    const value = object[key];
    const valueType = typeof object[key];

    if (valueType === 'function') {
      logger.debug(`object name=${objectName}, method = ${key}`);
    } else {
      const type = getValueType(value);
      if (type) {
        logger.debug(`object name = ${objectName}, field = ${key}, type = ${getTypeStringByType(type)}`);
      } else {
        logger.debug(`unsupported property ${key} for object ${objectName}, ignored this for remote`);
      }
    }
  }
}

/**
 * create new listener if function listener is null
 *
 * @param {RTRemoteProtocol}protocol the protocol that send call request
 * @param {object} rtFunction the old rt remote function
 * @return {object} the new rt function
 */
function updateListenerForRTFuction(protocol, rtFunction) {
  if (rtFunction && rtFunction[RTConst.VALUE] && rtFunction[RTConst.VALUE][RTConst.VALUE]) {
    return rtFunction;
  }

  const newRtFunction = {};
  const rtValue = {};
  newRtFunction[RTConst.VALUE] = (rtValueList) => {
    const args = rtValueList || [];
    return protocol.sendCallByName(
      rtFunction[RTConst.VALUE][RTConst.OBJECT_ID_KEY],
      rtFunction[RTConst.VALUE][RTConst.FUNCTION_KEY], ...args,
    );
  };
  newRtFunction[RTConst.FUNCTION_KEY] = rtFunction[RTConst.VALUE][RTConst.FUNCTION_KEY];
  newRtFunction[RTConst.OBJECT_ID_KEY] = rtFunction[RTConst.VALUE][RTConst.OBJECT_ID_KEY];
  rtValue[RTConst.VALUE] = newRtFunction;
  rtValue[RTConst.TYPE] = RTValueType.FUNCTION;
  RTEnvironment.getRtFunctionMap()[newRtFunction[RTConst.FUNCTION_KEY]] = newRtFunction[RTConst.VALUE];
  return rtValue;
}

/**
 * get RTMessageHelper module
 * @return {RTMessageHelper} the RTMessageHelper
 */
function getRTMessageHelper() {
  if (!rtMessageHelper) {
    rtMessageHelper = require('../RTMessageHelper'); // eslint-disable-line
  }
  return rtMessageHelper;
}

/**
 * set object value
 *
 * @param object the object value
 * @param requestMessage the set request message
 * @return {object} the set property response
 */
function setProperty(object, requestMessage) {
  const response = getRTMessageHelper().newSetPropertyResponse(
    requestMessage[RTConst.CORRELATION_KEY],
    RTStatusCode.UNKNOWN,
    requestMessage[RTConst.OBJECT_ID_KEY],
  );

  if (!object) { // object not found
    response[RTConst.STATUS_CODE] = RTStatusCode.OBJECT_NOT_FOUND;
  } else {
    const propName = requestMessage[RTConst.PROPERTY_NAME];
    const rtValue = requestMessage[RTConst.VALUE];

    // set array element by index
    if (requestMessage[RTConst.MESSAGE_TYPE] === RTRemoteMessageType.SET_PROPERTY_BYINDEX_REQUEST) {
      const index = requestMessage[RTConst.PROPERTY_INDEX];
      if (!Array.isArray(object)) { // is an array
        response[RTConst.STATUS_CODE] = RTStatusCode.TYPE_MISMATCH;
      } else if (index >= object.length) {
        response[RTConst.STATUS_CODE] = RTStatusCode.INVALID_ARGUMENT;
      } else {
        object[index] = rtValue;
        response[RTConst.STATUS_CODE] = RTStatusCode.OK;
      }
      return response;
    }

    if (!object[propName]) { // not found
      response[RTConst.STATUS_CODE] = RTStatusCode.PROPERTY_NOT_FOUND;
    } else if (getValueType(object[propName]) !== rtValue[RTConst.TYPE]) { // type mismatch
      response[RTConst.STATUS_CODE] = RTStatusCode.TYPE_MISMATCH;
    } else { // ok
      if (rtValue[RTConst.TYPE] === RTValueType.OBJECT) { // object need add object.id to client
        rtValue[RTConst.VALUE][RTConst.OBJECT_ID_KEY] = rtValue[RTConst.VALUE].id
          || rtValue[RTConst.VALUE][RTConst.OBJECT_ID_KEY];
      }
      object[propName] = rtValue;
      response[RTConst.STATUS_CODE] = RTStatusCode.OK;
    }
  }
  return response;
}

/**
 * get value by property name/index
 * @param {object} object the object value
 * @param {object} getRequest the request message
 * @param {RTRemoteServer} server the server instance
 * @return {object} the get response with value
 */
function getProperty(object, getRequest, server) {
  const response = getRTMessageHelper().newGetPropertyResponse(
    getRequest[RTConst.CORRELATION_KEY],
    RTStatusCode.UNKNOWN,
    getRequest[RTConst.OBJECT_ID_KEY],
    null,
  );

  if (!object) { // object not found
    response[RTConst.STATUS_CODE] = RTStatusCode.OBJECT_NOT_FOUND;
  } else {
    const propName = getRequest[RTConst.PROPERTY_NAME];

    // get element by index in array
    if (getRequest[RTConst.MESSAGE_TYPE] === RTRemoteMessageType.GET_PROPERTY_BYINDEX_REQUEST) { // get by index
      const index = getRequest[RTConst.PROPERTY_INDEX];
      const arr = object;
      if (!Array.isArray(arr)) { // should be an array
        response[RTConst.STATUS_CODE] = RTStatusCode.TYPE_MISMATCH;
      } else if (index >= arr.length) { // check index
        response[RTConst.STATUS_CODE] = RTStatusCode.INVALID_ARGUMENT;
      } else {
        response[RTConst.VALUE] = arr[index];
        response[RTConst.STATUS_CODE] = RTStatusCode.OK;
      }
      return response;
    }

    if (!object[propName]) { // not found
      response[RTConst.STATUS_CODE] = RTStatusCode.PROPERTY_NOT_FOUND;
    } else { // ok
      if (Array.isArray(object[propName])) { // array object
        let objectId = arrayObjectHashmap[object[propName]];
        if (!objectId) {
          objectId = `obj://${getRandomUUID()}`;
          arrayObjectHashmap[object[propName]] = objectId;
        }
        if (!server.isRegister(objectId)) {
          server.registerObject(objectId, object[propName]);
        }
        const v = {};
        v[RTConst.TYPE] = RTValueType.OBJECT;
        v[RTConst.VALUE] = {};
        v[RTConst.VALUE][RTConst.OBJECT_ID_KEY] = objectId;
        response[RTConst.VALUE] = v;
      } else if (typeof object[propName] === 'function') {
        const v = {};
        v[RTConst.TYPE] = RTValueType.FUNCTION;
        v[RTConst.OBJECT_ID_KEY] = getRequest[RTConst.OBJECT_ID_KEY];
        v[RTConst.FUNCTION_KEY] = propName;
        response[RTConst.VALUE] = v;
      } else {
        response[RTConst.VALUE] = object[propName];
      }
      response[RTConst.STATUS_CODE] = RTStatusCode.OK;
    }
  }
  return response;
}

/**
 * invoke object method by method name
 *
 * @param {object} object the object value
 * @param {object} callRequest the invoke request message
 * @return {object} the response with value
 */
function invokeMethod(object, callRequest) {
  const response = getRTMessageHelper().newCallResponse(
    callRequest[RTConst.CORRELATION_KEY],
    null,
    RTStatusCode.UNKNOWN,
  );
  if (!object) { // object not found
    response[RTConst.STATUS_CODE] = RTStatusCode.OBJECT_NOT_FOUND;
  } else {
    const destMethod = object[callRequest[RTConst.FUNCTION_KEY]];

    if (!destMethod) { // method not found
      response[RTConst.STATUS_CODE] = RTStatusCode.PROPERTY_NOT_FOUND;
    } else {
      response[RTConst.FUNCTION_RETURN_VALUE] = destMethod.apply(object, callRequest[RTConst.FUNCTION_ARGS]);
      response[RTConst.STATUS_CODE] = RTStatusCode.OK;
    }
  }
  return response;
}

module.exports = {
  getRandomUUID,
  getStatusStringByCode,
  setProperty,
  getTypeStringByType,
  getProperty,
  checkAndDumpObject,
  isBigNumber,
  invokeMethod,
  updateListenerForRTFuction,
};
