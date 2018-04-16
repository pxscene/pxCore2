/**
 * Unit tests for RTValueHelper
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTValueHelper = require('../lib/RTValueHelper');
const RTValueType = require('../lib/RTValueType');

// Required for testing with Big number object
const BigNumber = require('bignumber.js');
const should = require('should');

/*
 * Test RTValueHelper
 */

describe('RTValueHelper', () => {
  describe('Testing create function', () => {
    // Creating RTValue which is not covered in any of the switch case
    it('Creating RTValue of type INT8 should succeed', () => {
      const result = RTValueHelper.create(64, RTValueType.INT8);
      result.should.have.keys('value', 'type');
      result.value.should.be.eql(64);
      result.type.should.be.eql(RTValueType.INT8);
    });

    it('Creating RTValue of type FUNCTION should return a function with identifer', () => {
      const result = RTValueHelper.create(() => {
        const someNum = 10;
        return someNum;
      }, RTValueType.FUNCTION);
      result.should.have.keys('value', 'type', 'function.name', 'object.id');
      result.value.should.be.type('function');
      result.type.should.be.eql(RTValueType.FUNCTION);
      result['function.name'].should.startWith('func://');
      result['object.id'].should.eql('global');
    });

    it('Creating RTValue of type FUNCTION with null should not return a function identifier', () => {
      const result = RTValueHelper.create(null, RTValueType.FUNCTION);
      result.should.not.have.keys('function.name', 'object.id');
      should.not.exist(result.value);
      result.type.should.be.eql(RTValueType.FUNCTION);
    });

    it('Creating RTValue of type OBJECT should return an object identifier', () => {
      const result = RTValueHelper.create({ test: 'value' }, RTValueType.OBJECT);
      result.should.have.keys('value', 'type');
      result.type.should.be.eql(RTValueType.OBJECT);
      result.value['object.id'].should.startWith('obj://');
    });

    it('Creating RTValue of type OBJECT with null should not return a object identifier', () => {
      const result = RTValueHelper.create(null, RTValueType.OBJECT);
      result.should.not.have.keys('object.id');
      should.not.exist(result.value);
      result.type.should.be.eql(RTValueType.OBJECT);
    });

    it('Creating RTValue of type INT64 with BigNumber should succeed', () => {
      const result = RTValueHelper.create(new BigNumber('9223372036854775807'), RTValueType.INT64);
      result.should.have.keys('value', 'type');
      result.value.toString().should.be.eql('9223372036854775807');
      result.type.should.be.eql(RTValueType.INT64);
    });

    it('Creating RTValue of type INT64 with normal integer should fail', () => {
      const fnCall = () => { RTValueHelper.create(1234, RTValueType.INT64); };
      fnCall.should.throw('INT64/UINT64 cannot initialize with type number, only can use BigNumber initialize');
    });
  });
});
