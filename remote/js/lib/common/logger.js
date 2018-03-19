/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * This module contains the winston logger configuration.
 *
 * @author      TCSCODER
 * @version     1.0
 */

const winston = require('winston');
const config = require('../config');

/**
 * the logger tranports array
 * @type {Array}
 */
const transports = [];

/**
 * add transports
 */
if (!config.DISABLE_LOGGING) {
  transports.push(new (winston.transports.Console)({
    timestamp: true,
    colorize: true,
    level: config.LOG_LEVEL,
  }));
}

/**
 * create new logger
 */
const logger = new (winston.Logger)({
  transports,
});

module.exports = logger;
