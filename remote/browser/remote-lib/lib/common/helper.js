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

module.exports = {
  getRandomUUID,
  getStatusStringByCode,
  getTypeStringByType,
  isBigNumber,
};
