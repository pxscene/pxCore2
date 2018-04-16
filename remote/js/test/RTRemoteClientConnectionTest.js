/**
 * Unit tests for RTRemoteClientConnection
 *
 * @author      TCSCODER
 * @version     1.0
 */


const RTRemoteClientConnection = require('../lib/RTRemoteClientConnection');

const net = require('net');
const should = require('should'); // eslint-disable-line no-unused-vars

let server; // eslint-disable-line no-unused-vars

/*
 * Test RTRemoteClientConnection
 */

describe('RTRemoteClientConnection', () => {
  before((done) => {
  // Start a Dummy Server to test the connection
    server = net.createServer((connection) => {}).listen(10023); // eslint-disable-line no-unused-vars
    done();
  });

  it('createTCPClientConnection with invalid TCP connection details should throw error', () => {
    RTRemoteClientConnection.createTCPClientConnection('localhost', 10024)
      .should.be.rejectedWith('connect ECONNREFUSED 127.0.0.1:10024');
  });

  it('createTCPClientConnection with valid TCP connection details should succeed', () => {
    RTRemoteClientConnection.createTCPClientConnection('localhost', 10023).then((conn) => {
      conn.protocol.transport.host.should.be.eql('localhost');
      conn.protocol.transport.port.should.be.eql(10023);
    });
  });
});
