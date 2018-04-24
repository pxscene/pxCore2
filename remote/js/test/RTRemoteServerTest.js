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
