/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * This module contains the winston logger configuration.
 *
 * @author      TCSCODER
 * @version     1.0
 */


const config = require('../config');

/**
 * get output function
 * @param type the log type
 * @return {*} the output function
 */
const getOutput = (type) => {
  if (config.DISABLE_LOGGING) {
    return () => null;
  }
  return console[ type ]; // eslint-disable-line
};

/**
 * create new logger
 */
const logger = {
  warn: getOutput('warn'),
  info: getOutput('info'),
  debug: getOutput('debug'),
  error: getOutput('error'),
};

module.exports = logger;
