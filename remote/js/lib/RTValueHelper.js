/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The rt value helper, used to crate rt value
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTEnvironment = require('./RTEnvironment');
const RTValueType = require('./RTValueType');
const RTConst = require('./RTConst');
const helper = require('./common/helper');
const RTException = require('./RTException');

/**
 * create a new rtValue
 * @param {string|number|BigNumber|object|function} value the value
 * @param {RTValueType|number} type the rt value type value
 * @return {object} the rtValue
 */
function create(value, type) {
  const rtValue = { value, type };
  switch (type) {
    case RTValueType.FUNCTION: {
      if (value) { // create rtValue with type function
        const functionName = `func://${helper.getRandomUUID()}`;
        rtValue[RTConst.FUNCTION_KEY] = functionName;
        rtValue[RTConst.OBJECT_ID_KEY] = RTConst.FUNCTION_GLOBAL_SCOPE;
        RTEnvironment.getRtFunctionMap()[functionName] = value; // cache the callback
      }
      break;
    }
    case RTValueType.OBJECT: {
      if (value) { // create rtObject
        const objectId = `obj://${helper.getRandomUUID()}`;
        rtValue[RTConst.VALUE] = {};
        rtValue[RTConst.VALUE][RTConst.OBJECT_ID_KEY] = objectId;
        value[RTConst.OBJECT_ID_KEY] = objectId;
        RTEnvironment.getRtObjectMap()[objectId] = value; // cache the object
      }
      break;
    }
    case RTValueType.INT64:
    case RTValueType.UINT64: {
      if (value && typeof value !== 'object') { // none null/undefined value must be object/BigNumber type
        throw new RTException(`INT64/UINT64 cannot initialize with type ${typeof value}, only can use BigNumber initialize`);
      }
      break;
    }
    default:
      break;
  }
  return rtValue;
}

module.exports = {
  create,
};
