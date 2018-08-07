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
