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
 * the rt environment
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTConst = require('./RTConst');

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

/**
 * this app run mode
 * @type {string}
 */
let runMode = RTConst.CLIENT_MODE;

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

  /**
   * get the app run mode
   * @return {string} the app run mode
   */
  getRunMode: () => runMode,

  /**
   * set the app run mode
   * @param {string} mode
   */
  setRunMode: (mode) => {
    runMode = mode;
  },

  /**
   * check app is run as server or not
   * @return {boolean} the result
   */
  isServerMode: () => runMode === RTConst.SERVER_MODE,
};
