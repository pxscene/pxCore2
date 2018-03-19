/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the RTRemote Serializer, used to serializer/unserializer message object
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTConst = require('./RTConst');
const RTValueType = require('./RTValueType');
const RTEnvironment = require('./RTEnvironment');
const RTRemoteObject = require('./RTRemoteObject');
const RTRemoteMessageType = require('./RTRemoteMessageType');
const helper = require('./common/helper');
const JSONbig = require('json-bigint');

/**
 * convert message object to buffer
 * @param {Object} messageObj the message object
 * @returns {Buffer2 | Buffer} the node buffer
 */
function toBuffer(messageObj) {
  return Buffer.from(JSONbig.stringify(messageObj), RTConst.DEFAULT_CHARSET);
}

/**
 * proccess rtValue , convert json parsed object to rtValue
 * @param {object} rtValue the rtValue
 */
function processRTValue(rtValue) {
  if (!rtValue) return;
  const valueType = rtValue[RTConst.TYPE];
  if (valueType === RTValueType.FUNCTION) {
    const functionCb = RTEnvironment.getRtFunctionMap()[rtValue[RTConst.FUNCTION_KEY]];
    // sometimes  RTEnvironment didn't cache the rtFunction, that's mean the rtFunction from remote
    // 1. client get backend rtFunction, but client didn't cache it, that's mean rtFunction located in backend
    // 2. server receive client set->rtFunction, but server didn't cache it, that's mean rtFunction located in client
    if (functionCb) {
      rtValue.value = functionCb;
    }
  } else if (valueType === RTValueType.OBJECT) {
    const obj = RTEnvironment.getRtObjectMap()[rtValue[RTConst.VALUE][RTConst.OBJECT_ID_KEY]];
    if (obj) { // object found in cache
      rtValue[RTConst.VALUE] = obj;
    } else {
      rtValue[RTConst.VALUE] = new RTRemoteObject(null, rtValue[RTConst.VALUE][RTConst.OBJECT_ID_KEY]);
    }
  } else if (valueType === RTValueType.VOIDPTR) {
    // this should be a bug from c++ remote,  the property name should be "value", not "Value"
    rtValue[RTConst.VALUE] = rtValue[RTConst.VALUE] || rtValue.Value;
  } else if (valueType === RTValueType.FLOAT || valueType === RTValueType.DOUBLE) {
    const v = rtValue[RTConst.VALUE];
    rtValue[RTConst.VALUE] = helper.isBigNumber(v) ? v.toNumber() : v;
  }
}

/**
 * convert buffer to message object
 * @param messageBuffer {Buffer} the message buffer
 * @returns {Object} the message object
 */
function fromBuffer(messageBuffer) {
  const messageObj = JSONbig.parse(messageBuffer.toString(RTConst.DEFAULT_CHARSET));
  const mType = messageObj[RTConst.MESSAGE_TYPE];

  switch (mType) {
    case RTRemoteMessageType.SET_PROPERTY_BYNAME_REQUEST:
    case RTRemoteMessageType.SET_PROPERTY_BYNAME_RESPONSE:
    case RTRemoteMessageType.GET_PROPERTY_BYNAME_RESPONSE:
    case RTRemoteMessageType.GET_PROPERTY_BYNAME_REQUEST: {
      processRTValue(messageObj[RTConst.VALUE]);
      break;
    }
    case RTRemoteMessageType.METHOD_CALL_REQUEST: {
      const args = messageObj[RTConst.FUNCTION_ARGS] || [];
      args.forEach(arg => processRTValue(arg));
      break;
    }
    case RTRemoteMessageType.METHOD_CALL_RESPONSE: {
      processRTValue(messageObj[RTConst.FUNCTION_RETURN_VALUE]);
      break;
    }
    default: {
      break;
    }
  }

  return messageObj;
}

module.exports = {
  toBuffer, fromBuffer,
};
