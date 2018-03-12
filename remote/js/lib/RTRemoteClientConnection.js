/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The rt remote client connection
 *
 * @author      TCSCODER
 * @version     1.0
 */

const RTRemoteTCPTransport = require('./RTRemoteTCPTransport');
const RTRemoteObject = require('./RTRemoteObject');
const RTRemoteProtocol = require('./RTRemoteProtocol');

/**
 * the rt remote client connection class
 */
class RTRemoteClientConnection {
  /**
   * create new connection with protocol
   * @param {RTRemoteProtocol} protocol
   */
  constructor(protocol) {
    this.protocol = protocol;
  }

  /**
   * get rt remote object by object id
   * @param {string} objectId the object id
   * @return {RTRemoteObject} the returned rtRemote object
   */
  getProxyObject(objectId) {
    return new RTRemoteObject(this.protocol, objectId);
  }
}

/**
 * create tcp connection
 * @param {string} host the host name
 * @param {number/int} port the host port
 * @return {Promise<RTRemoteClientConnection>} the promise with connection
 */
function createTCPClientConnection(host, port) {
  const transport = new RTRemoteTCPTransport(host, port);
  return RTRemoteProtocol.create(transport, false).then(protocol => new RTRemoteClientConnection(protocol));
}

module.exports = {
  createTCPClientConnection,
};
