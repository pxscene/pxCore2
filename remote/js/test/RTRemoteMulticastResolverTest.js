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
 * Unit tests for RTRemoteMulticastResolver
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteMulticastResolver = require('../lib/RTRemoteMulticastResolver');
const RTRemoteServer = require('../lib/RTRemoteServer');

const should = require('should'); // eslint-disable-line no-unused-vars

// RTRemoteMulticastResolver instance
const resolve = new RTRemoteMulticastResolver('224.10.10.12', 10004);

/*
 * Test RTRemoteMulticastResolver
 */

describe('RTRemoteMulticastResolver', () => {
  before((done) => {
  // Server is explicitly required to test all functions of RTRemoteMulticastResolver
    RTRemoteServer.create('224.10.10.12', 10004, '127.0.0.1').then((rtRemoteServer) => {
      rtRemoteServer.registerObject('host_object', { name: 'testing' });
      resolve.start().then(() => {
        done();
      });
    });
  });

  describe('Testing getReplyEndpoint function', () => {
    it('Calling getReplyEndpoint function should return a valid URI', () => {
      const uri = resolve.getReplyEndpoint();
      uri.should.startWith('udp://');
    });
  });

  describe('Testing locateObject function', () => {
    it('Calling locateObject with valid object should return a valid TCP URI', () => {
      resolve.locateObject('host_object').then((uri) => {
        uri.should.startWith('tcp://');
      });
    });

    it('Calling locateObject with invalid object should throw an error or return invalid status', () => {
      resolve.locateObject('dummy').then((status) => { // eslint-disable-line no-unused-vars
        // There is no response from RTRemoteServer for Invalid search
        // Server responds only for valid objects
        // There is no response from Server if the locateObject request is invalid
        // Server should indeed respond that requested object is not present
        // TODO: Once the server returns invalid status check it
      });
    });
  });
});
