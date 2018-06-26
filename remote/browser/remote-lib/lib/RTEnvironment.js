/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the rt environment
 *
 * @author      TCSCODER
 * @version     1.0
 */

/**
 * the rt function map, used to cache local rt function
 * @type {object}
 */
const rtFunctionMap = {};

/**
 * the rt object map, used to cache local rt object
 * @type {object}
 */
const rtObjectMap = {};

module.exports = {

  /**
   * get rt function cache map
   * @return {object} the function map
   */
  getRtFunctionMap: () => rtFunctionMap,

  /**
   * get the rt object cache map
   * @return {object} the object map
   */
  getRtObjectMap: () => rtObjectMap,
};
