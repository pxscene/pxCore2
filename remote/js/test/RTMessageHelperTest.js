/**
 * Unit tests for RTMessageHelper
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTMessageHelper = require('../lib/RTMessageHelper');
const RTRemoteMessageType = require('../lib/RTRemoteMessageType');

const should = require('should'); // eslint-disable-line no-unused-vars

/*
 * Test RTMessageHelper
 */

describe('RTMessageHelper', () => {
  it('newLocateRequest should return valid LocateRequest', () => {
    const result = RTMessageHelper.newLocateRequest('1234', 'somePerson');
    result['message.type'].should.be.eql(RTRemoteMessageType.SEARCH_OBJECT);
    result['object.id'].should.be.eql('1234');
    result['reply-to'].should.be.eql('somePerson');
  });

  it('newLocateResponse should return valid location response', () => {
    const result = RTMessageHelper.newLocateResponse('tcp://127.0.0.1:1423', '1234', '2345', 'someKey');
    result['message.type'].should.be.eql(RTRemoteMessageType.LOCATE_OBJECT);
    result.endpoint.should.be.eql('tcp://127.0.0.1:1423');
    result['correlation.key'].should.be.eql('someKey');
    result['sender.id'].should.be.eql('2345');
    result['object.id'].should.be.eql('1234');
  });

  it('newSetRequest should return valid set request', () => {
    const result = RTMessageHelper.newSetRequest('1234', 'someProp', 'someValue');
    result['message.type'].should.be.eql(RTRemoteMessageType.SET_PROPERTY_BYNAME_REQUEST);
    result['property.name'].should.be.eql('someProp');
    result.value.should.be.eql('someValue');
    result['object.id'].should.be.eql('1234');
  });

  it('newGetRequest should return valid get response', () => {
    const result = RTMessageHelper.newGetRequest('1234', 'someProp');
    result['message.type'].should.be.eql(RTRemoteMessageType.GET_PROPERTY_BYNAME_REQUEST);
    result['property.name'].should.be.eql('someProp');
    result['object.id'].should.be.eql('1234');
  });

  it('newCallMethodRequest should return valid callMethod request', () => {
    const result = RTMessageHelper.newCallMethodRequest('1234', 'someFunc', 'someParam');
    result['message.type'].should.be.eql(RTRemoteMessageType.METHOD_CALL_REQUEST);
    result['object.id'].should.be.eql('1234');
    result['function.name'].should.be.eql('someFunc');
    result['function.args'].should.be.eql(['someParam']);
  });

  it('newCallResponse should return valid callResponse object', () => {
    const result = RTMessageHelper.newCallResponse('1234', 'someRetValue', 200);
    result['message.type'].should.be.eql(RTRemoteMessageType.METHOD_CALL_RESPONSE);
    result['correlation.key'].should.be.eql('1234');
    result['function.return_value'].should.be.eql('someRetValue');
    result['status.code'].should.be.eql(200);
  });

  it('newCallResponse with null return value should return valid callResponse object', () => {
    const result = RTMessageHelper.newCallResponse('1234', null, 200);
    result['message.type'].should.be.eql(RTRemoteMessageType.METHOD_CALL_RESPONSE);
    result['correlation.key'].should.be.eql('1234');
    result['function.return_value'].should.be.null; // eslint-disable-line
    result['status.code'].should.be.eql(200);
  });

  it('newKeepAliveResponse should return valid KeepAlive Response', () => {
    const result = RTMessageHelper.newKeepAliveResponse('1234', 200);
    result['message.type'].should.be.eql(RTRemoteMessageType.KEEP_ALIVE_RESPONSE);
    result['correlation.key'].should.be.eql('1234');
    result['status.code'].should.be.eql(200);
  });

  it('newSetPropertyResponse should return valid SetProperty Response', () => {
    const result = RTMessageHelper.newSetPropertyResponse('1234', 200, '2345');
    result['message.type'].should.be.eql(RTRemoteMessageType.SET_PROPERTY_BYNAME_RESPONSE);
    result['correlation.key'].should.be.eql('1234');
    result['status.code'].should.be.eql(200);
    result['object.id'].should.be.eql('2345');
  });

  it('newGetPropertyResponse should return valid GetProperty Response', () => {
    const result = RTMessageHelper.newGetPropertyResponse('1234', 200, '2345', 'someValue');
    result['message.type'].should.be.eql(RTRemoteMessageType.GET_PROPERTY_BYNAME_RESPONSE);
    result['correlation.key'].should.be.eql('1234');
    result['status.code'].should.be.eql(200);
    result['object.id'].should.be.eql('2345');
    result.value.should.be.eql('someValue');
  });
});
