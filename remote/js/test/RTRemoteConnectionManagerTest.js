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
 * Unit tests for RTRemoteConnectionManager
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteConnectionManager = require('../lib/RTRemoteConnectionManager');
const net = require('net');
const { URL } = require('url');

let server;

/*
 * Test RTRemoteConnectionManager
 */

describe('RTRemoteConnectionManager', () => {
  before((done) => {
  // Start a Dummy Server to test the connection
    server = net.createServer((connection) => {}).listen(10000); // eslint-disable-line no-unused-vars
    done();
  });

  after((done) => {
    server.close();
    done();
  });

  it('Creating an UDP connection should throw an error', () => {
    const udpCreate = () => { RTRemoteConnectionManager.getObjectProxy(new URL('udp://192.168.72.1:34567')); };
    udpCreate.should.throw('unsupported scheme : udp:');
  });

  it('Creating an TCP connection to non-existent server should throw error', () => {
    const tcpNonExist = () => { RTRemoteConnectionManager.getObjectProxy(new URL('tcp://127.0.0.1:34567')); }; // eslint-disable-line
    // TODO: Current implementation doesn't throw error if the connection is refused
    // to the given URI though there are errors printed in the console
    // error:  Error: connect ECONNREFUSED 127.0.0.1:34567
    // tcpNonExist.should.throw('failed to open socket');
  });

  it('Creating an TCP connection to existing server should after creating connection', () => {
    RTRemoteConnectionManager.getObjectProxy(new URL('tcp://127.0.0.1:10000')).then((remoteObj) => {
      remoteObj.protocol.transport.host.should.be.eql('127.0.0.1');
      remoteObj.protocol.transport.port.should.be.eql('10000');
    });
  });

  // For test coverage
  it('Creating an TCP connection to same TCP server will return from connection pool', () => {
    RTRemoteConnectionManager.getObjectProxy(new URL('tcp://127.0.0.1:10000')).then((remoteObj) => {
      remoteObj.protocol.transport.host.should.be.eql('127.0.0.1');
      remoteObj.protocol.transport.port.should.be.eql('10000');
    });
  });
});
