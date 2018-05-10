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
 * Unit tests for RTRemoteTCPTransport
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteTCPTransport = require('../lib/RTRemoteTCPTransport');

const net = require('net');
const should = require('should'); // eslint-disable-line no-unused-vars

let server; // eslint-disable-line no-unused-vars
let transport; // Remote TCP transport

/*
 * Test RTRemoteTCPTransport
 */

describe('RTRemoteTCPTransport', () => {
  before((done) => {
  // Start a Dummy Server to test the connection
    server = net.createServer((connection) => {}).listen(10022); // eslint-disable-line no-unused-vars
    transport = new RTRemoteTCPTransport('127.0.0.1', 10022);
    done();
  });

  it('Opening connection to valid TCP transport should succeed', () => {
    transport.open().then((trans) => {
      trans.host.should.be.eql('127.0.0.1');
      trans.port.should.be.eql(10022);
      trans.mRunning.should.be.eql(true);
    });
  });

  it('Sending message to open transport should succeed', () => {
    transport.open().then((trans) => {
      const sendCall = () => { trans.send('Testing'); };
      sendCall.should.not.throw();
    });
  });

  it('Sending message to closed transport should throw error', (done) => {
    transport.open().then((trans) => { // eslint-disable-line no-unused-vars
      transport.socket.end();
      setTimeout(() => {
        const closedCall = () => { transport.send('Testing'); };
        closedCall.should.throw('cannot send because of transport mRunning = false');
        done();
      }, 1000);
    });
  }).timeout(10000);

  // Mainly for test coverage
  it('Emitting an error on socket will close the connection', (done) => {
    transport.open().then((trans) => { // eslint-disable-line no-unused-vars
      transport.socket.emit('error', new Error('ECONNRESET'));
      transport.mRunning.should.be.eql(false);
      done();
    });
  });
});
