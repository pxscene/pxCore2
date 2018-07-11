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
 * the sample object class
 *
 * @author      TCSCODER
 * @version     1.0
 */


const RTValueHelper = require('../../lib/RTValueHelper');
const RTValueType = require('../../lib/RTValueType');
const logger = require('../../lib/common/logger');

/**
 * the sample object class
 */
class SampleObject {
  constructor() {
    this.name = RTValueHelper.create(false, RTValueType.STRING);
    this.uint64 = RTValueHelper.create(null, RTValueType.UINT64);
    this.ffloat = RTValueHelper.create(null, RTValueType.FLOAT);
    this.bbool = RTValueHelper.create(null, RTValueType.BOOLEAN);

    this.bbool = RTValueHelper.create(null, RTValueType.BOOLEAN);
    this.int8 = RTValueHelper.create(null, RTValueType.INT8);
    this.uint8 = RTValueHelper.create(null, RTValueType.UINT8);
    this.int32 = RTValueHelper.create(null, RTValueType.INT32);
    this.uint32 = RTValueHelper.create(null, RTValueType.UINT32);
    this.int64 = RTValueHelper.create(null, RTValueType.INT64);
    this.uint64 = RTValueHelper.create(null, RTValueType.UINT64);
    this.ddouble = RTValueHelper.create(null, RTValueType.DOUBLE);
    this.string = RTValueHelper.create(null, RTValueType.STRING);
    this.vptr = RTValueHelper.create(null, RTValueType.VOIDPTR);
    this.onTick = RTValueHelper.create(null, RTValueType.FUNCTION);
    this.objvar = RTValueHelper.create(null, RTValueType.OBJECT);
    this.methodValue = RTValueHelper.create(0, RTValueType.INT32);
    this.arr = [RTValueHelper.create(10, RTValueType.INT32),
      RTValueHelper.create(12.3, RTValueType.FLOAT),
      RTValueHelper.create('hello,world', RTValueType.STRING)];
  }

  method0AndReturn10() { // eslint-disable-line class-methods-use-this
    return RTValueHelper.create(10, RTValueType.INT32);
  }

  twoIntNumberSum(in1, in2) { // eslint-disable-line class-methods-use-this
    return RTValueHelper.create(in1.value + in2.value, RTValueType.INT32);
  }

  twoFloatNumberSum(in1, in2) { // eslint-disable-line class-methods-use-this
    return RTValueHelper.create(in1.value + in2.value, RTValueType.FLOAT);
  }

  method1IntAndNoReturn(in1) {
    this.methodValue = in1;
  }

  method2FunctionAndNoReturn(rtFunction, in1) {
    if (rtFunction && rtFunction.value && rtFunction.value.value) {
      rtFunction.value.value([in1, this.methodValue]).then(() => {
        logger.debug('method2FunctionAndNoReturn invoke rtFunction succeed');
      }).catch((err) => {
        logger.error(err.stack);
      });
    } else {
      logger.debug('rtFunction or listener is null');
    }
  }
}


module.exports = SampleObject;
