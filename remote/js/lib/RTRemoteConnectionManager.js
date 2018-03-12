/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
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
      throw new Error(`unsupported scheme : ${url.protocol}`);
  }
  return connection;
}

module.exports = {
  getObjectProxy,
};
