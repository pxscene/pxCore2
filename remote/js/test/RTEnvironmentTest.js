/**
 * Unit tests for RTEnvironment
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTEnvironment = require('../lib/RTEnvironment');
const RTConst = require('../lib/RTConst');

const should = require('should'); // eslint-disable-line no-unused-vars

/*
 * Test RTEnvironment
 */
describe('RTEnvironment', () => {
  it('Should be able to get default RunMode before setting it', () => {
    const result = RTEnvironment.getRunMode();
    result.should.be.eql(RTConst.CLIENT_MODE);
  });

  it('Should be able to set RunMode', () => {
    RTEnvironment.setRunMode(RTConst.SERVER_MODE);
    const result = RTEnvironment.getRunMode();
    result.should.be.eql(RTConst.SERVER_MODE);
  });

  it('Should be able to check if the Running mode is SERVER_MODE', () => {
    const result = RTEnvironment.isServerMode();
    result.should.be.eql(true);
  });

  // rtFunctionMap and rtObjectMap will be empty initially
});
