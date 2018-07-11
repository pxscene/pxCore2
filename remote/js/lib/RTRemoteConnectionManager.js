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
 * The remote connection manager
 *
 * @author      TCSCODER
 * @version     1.0
 */
const { URL } = require('url');
const logger = require('./common/logger');
const RTRemoteClientConnection = require('./RTRemoteClientConnection');
const RTException = require('./RTException');

/**
 * the connections map
 * @type {object} the connections map
 */
const connections = {};

/**
 * get rt remote object by uri
 * @param {string} uri the object uri
 * @return {Promise<RTRemoteObject>} the promise with rt remote object
 */
function getObjectProxy(uri) {
  const url = new URL(uri);
  const connectionSpec = `${url.protocol}//${url.hostname}:${url.port}`;
  const getRemoteObject = (conn, pathname) => conn.getProxyObject(pathname.substr(1, pathname.length));
  if (connections[connectionSpec]) {
    return Promise.resolve(getRemoteObject(connections[connectionSpec], url.pathname));
  }
  return createConnectionFromSpec(url).then((connection) => {
    connections[connectionSpec] = connection;
    return Promise.resolve(getRemoteObject(connection, url.pathname));
  });
}

/**
 * create connection from url
 * @param {URL} url the dest url the url object
 * @return {Promise<RTRemoteClientConnection>} the promise with connection
 */
function createConnectionFromSpec(url) {
  let connection = null;
  const schema = url.protocol.substr(0, url.protocol.length - 1);
  logger.info(`start connection ${url}`);
  switch (schema) {
    case 'tcp':
      connection = RTRemoteClientConnection.createTCPClientConnection(url.hostname, url.port);
      break;
    default:
      throw new RTException(`unsupported scheme : ${url.protocol}`);
  }
  return connection;
}

module.exports = {
  getObjectProxy,
};
