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
 * @param {number|int} port the host port
 * @return {Promise<RTRemoteClientConnection>} the promise with connection
 */
function createTCPClientConnection(host, port) {
  const transport = new RTRemoteTCPTransport(host, port);

  // 1. create protocol, the second param is false mean protocol will open transport socket
  // connection and bind relate input/output events
  // 2. then use initialized protocol create connection
  return RTRemoteProtocol.create(transport, false).then(protocol => new RTRemoteClientConnection(protocol));
}

module.exports = {
  createTCPClientConnection,
};
