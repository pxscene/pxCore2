/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * type test examples
 *
 * @author      TCSCODER
 * @version     1.0
 */
var RTValueType = RTRemote.RTValueType;
var BigNumber = RTRemote.BigNumber;
var createTypeTest = common.createTypeTest;

window.createTypeTestExamples = function (uri, objectNames) {

  return RTRemote.RTRemoteConnectionManager.getObjectProxy(uri, objectNames[ 0 ]).then(rtObject => {
    var items = [];

    // function test
    items.push(createTypeTest(rtObject, RTValueType.FUNCTION, null, 'onTick', 'function'));
    // object test
    items.push(createTypeTest(rtObject, RTValueType.OBJECT, null, 'objvar', 'object'));

    // arr index
    items.push(createTypeTest(rtObject, RTValueType.INT32, 104, 0, 'index'));
    items.push(createTypeTest(rtObject, RTValueType.FLOAT, 123.2, 1, 'index'));
    items.push(createTypeTest(rtObject, RTValueType.STRING, 'Sample String', 2, 'index'));

    // in c++/java, float only had 7 valid digits
    items.push(createTypeTest(rtObject, RTValueType.FLOAT, 1.23456789, 'ffloat'));
    items.push(createTypeTest(rtObject, RTValueType.FLOAT, -1.234567, 'ffloat'));
    items.push(createTypeTest(rtObject, RTValueType.FLOAT, 12.34567, 'ffloat'));
    items.push(createTypeTest(rtObject, RTValueType.FLOAT, 123.4567, 'ffloat'));
    items.push(createTypeTest(rtObject, RTValueType.FLOAT, 1234.567, 'ffloat'));
    items.push(createTypeTest(rtObject, RTValueType.FLOAT, 123456.71, 'ffloat'));

    // test bool
    items.push(createTypeTest(rtObject, RTValueType.BOOLEAN, true, 'bbool'));
    items.push(createTypeTest(rtObject, RTValueType.BOOLEAN, false, 'bbool'));

    // int8 range [-128,127]
    items.push(createTypeTest(rtObject, RTValueType.INT8, -128, 'int8'));
    items.push(createTypeTest(rtObject, RTValueType.INT8, 0, 'int8'));
    items.push(createTypeTest(rtObject, RTValueType.INT8, 127, 'int8'));

    // test uint8, the data range is[0,255]
    items.push(createTypeTest(rtObject, RTValueType.UINT8, 0, 'uint8'));
    items.push(createTypeTest(rtObject, RTValueType.UINT8, 255, 'uint8'));

    // test int32, range is  [–2147483648 , 2147483647]
    items.push(createTypeTest(rtObject, RTValueType.INT32, -2147483648, 'int32'));
    items.push(createTypeTest(rtObject, RTValueType.INT32, 0, 'int32'));
    items.push(createTypeTest(rtObject, RTValueType.INT32, 123, 'int32'));
    items.push(createTypeTest(rtObject, RTValueType.INT32, 2147483647, 'int32'));

    // test uint32, range is [0 - 4,294,967,295]
    items.push(createTypeTest(rtObject, RTValueType.UINT32, 0, 'uint32'));
    items.push(createTypeTest(rtObject, RTValueType.UINT32, 4294967295, 'uint32'));
    items.push(createTypeTest(rtObject, RTValueType.UINT32, 123123, 'uint32'));

    // test int64, range is [–9223372036854775808  9223372036854775807]
    items.push(createTypeTest(rtObject, RTValueType.INT64, new BigNumber('-9223372036854775808'), 'int64'));
    items.push(createTypeTest(rtObject, RTValueType.INT64, new BigNumber('9223372036854775807'), 'int64'));
    items.push(createTypeTest(rtObject, RTValueType.INT64, new BigNumber(0), 'int64'));
    items.push(createTypeTest(rtObject, RTValueType.INT64, new BigNumber(123123), 'int64'));

    // test uint64, range is [0 - 18446744073709551615]
    items.push(createTypeTest(rtObject, RTValueType.UINT64, new BigNumber('18446744073709551615'), 'uint64'));
    items.push(createTypeTest(rtObject, RTValueType.UINT64, new BigNumber(0), 'uint64'));
    items.push(createTypeTest(rtObject, RTValueType.UINT64, new BigNumber(123123123), 'uint64'));
    items.push(createTypeTest(rtObject, RTValueType.UINT64, new BigNumber('123'), 'uint64'));

    // test double
    items.push(createTypeTest(rtObject, RTValueType.DOUBLE, 1231.12312312312, 'ddouble'));
    items.push(createTypeTest(rtObject, RTValueType.DOUBLE, -1231.12312312312, 'ddouble'));
    items.push(createTypeTest(rtObject, RTValueType.DOUBLE, -0.12, 'ddouble'));

    // test string
    items.push(createTypeTest(
      rtObject, RTValueType.STRING,
      'implemented in both /Library/Java/JavaVirtualMachines/jdk1.8.0_40.jdk/Conten', 'string',
    ));
    items.push(createTypeTest(
      rtObject, RTValueType.STRING,
      '{"jsonKey":"values"}', 'string',
    ));
    items.push(createTypeTest(rtObject, RTValueType.STRING, '1', 'string'));

    // void ptr is a uint32 or uint64
    items.push(createTypeTest(rtObject, RTValueType.VOIDPTR, new BigNumber('723123231'), 'vptr'));
    items.push(createTypeTest(rtObject, RTValueType.VOIDPTR, new BigNumber('789892349'), 'vptr'));
    return {
      items, rtObject
    };
  });
};
