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
        const v = {};
        v[RTConst.FUNCTION_KEY] = functionName;
        v[RTConst.OBJECT_ID_KEY] = RTConst.FUNCTION_GLOBAL_SCOPE;
        v[RTConst.VALUE] = value;
        rtValue[RTConst.VALUE] = v;
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
