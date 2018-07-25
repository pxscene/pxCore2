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
 * Unit tests for RTRemoteServer
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteServer = require('../lib/RTRemoteServer');

const should = require('should');

let remoteServer;

/*
 * Test RTRemoteServer
 */
describe('RTRemoteServer', () => {
  before((done) => {
    RTRemoteServer.create('224.10.10.25', 10050, '127.0.0.1').then((rtRemoteServer) => {
      remoteServer = rtRemoteServer;
      Promise.resolve()
        .then(() => remoteServer.registerObject('test_object', { name: 'testing' }))
        .then(() => { remoteServer.registerObject('null_object', null); done(); });
    });
  });

  describe('Testing registerObject function ', () => {
    it('Register Object should succeed with valid object', () => {
      const regObj = () => { remoteServer.registerObject('check_object', { name: 'host' }); };
      regObj.should.not.throw();
    });

    it('Registering the same object again should fail', () => {
      const regAgain = () => { remoteServer.registerObject('check_object', { name: 'host' }); };
      regAgain.should.throw(`object with name = check_object already exists in server ${remoteServer.serverName}, please don't register again.`); // eslint-disable-line
    });
  });

  describe('Testing un-registerObject function ', () => {
    it('Deregistering non-existent object should fail', () => {
      const deRegInvalid = () => { remoteServer.unRegisterObject('invalid_obj'); };
      deRegInvalid.should.throw(`cannot found register object named invalid_obj in server ${remoteServer.serverName}`);
    });

    it('Deregistering existing object should succeed', () => {
      const deRegvalid = () => { remoteServer.unRegisterObject('check_object'); };
      deRegvalid.should.not.throw();
    });
  });

  describe('Testing getObjectByName function ', () => {
    it('getObjectByName with valid object should succeed', () => {
      const testObj = remoteServer.getObjectByName('test_object');
      testObj.name.should.be.eql('testing');
    });

    it('getObjectByName for NuLL object should be NULL', () => {
      const result = remoteServer.getObjectByName('null_object');
      should.not.exist(result);
    });
  });
});
