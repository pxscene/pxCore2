/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
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
    this.methodValue = 0;
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
    if (rtFunction && rtFunction.value) {
      rtFunction.value([in1, this.methodValue]).then(() => {
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
