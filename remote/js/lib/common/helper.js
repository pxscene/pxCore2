/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * This module contains some common helper method
 *
 * @author      TCSCODER
 * @version     1.0
 */

const uuidV4 = require('uuid/v4');
const RTStatusCode = require('../RTStatusCode');
const RTValueType = require('../RTValueType');

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
  return 'UNKOWN CODE';
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
  return 'UNKOWN TYPE';
}

module.exports = {
  getRandomUUID, getStatusStringByCode, getTypeStringByType,
};
