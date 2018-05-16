/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The remote connection manager
 *
 * @author      TCSCODER
 * @version     1.0
 */
const logger = require('./common/logger');
const RTRemoteClientConnection = require('./RTRemoteClientConnection');
const RTException = require('./RTException');

/**
 * the connections map
 * @type {object} the connections map
 */
const connections = {};

/**
 * get rt remote object by websocket uri and object name
 * @param {string} uri the object uri
 * @param {string} objectName object name
 * @return {Promise<RTRemoteObject>} the promise with rt remote object
 */
function getObjectProxy(uri, objectName) {
  const connectionSpec = uri;
  const getRemoteObject = (conn, name) => conn.getProxyObject(name);
  if (connections[connectionSpec] && connections[connectionSpec].protocol.transport.mRunning) {
    return Promise.resolve(getRemoteObject(connections[connectionSpec], objectName));
  }
  return createConnectionFromSpec(uri).then((connection) => {
    connections[connectionSpec] = connection;
    return Promise.resolve(getRemoteObject(connection, objectName));
  });
}

/**
 * create connection from url
 * @param {string} uri the websocket server uri
 * @return {Promise<RTRemoteClientConnection>} the promise with connection
 */
function createConnectionFromSpec(uri) {
  let connection = null;
  logger.info(`start connection ${uri}`);
  const schema = uri.split('://')[0];
  switch (schema) {
    case 'ws':
      connection = RTRemoteClientConnection.createWBClientConnection(uri);
      break;
    default:
      throw new RTException(`unsupported scheme : ${schema}`);
  }
  return connection;
}

module.exports = {
  getObjectProxy,
};
