/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The rt remote server class.
 *
 * @author      TCSCODER
 * @version     1.0
 */
const dgram = require('dgram');
const net = require('net');
const { URL } = require('url');
const RTEnvironment = require('./RTEnvironment');
const RTRemoteSerializer = require('./RTRemoteSerializer');
const RTRemoteTCPTransport = require('./RTRemoteTCPTransport');
const RTRemoteProtocol = require('./RTRemoteProtocol');
const RTRemoteMessageType = require('./RTRemoteMessageType');
const RTConst = require('./RTConst');
const RTMessageHelper = require('./RTMessageHelper');
const RTValueType = require('./RTValueType');
const RTException = require('./RTException');
const helper = require('./common/helper');
const logger = require('./common/logger');

/**
 * the registered object map
 * @type {object}
 */
const registeredObjectMap = {};

class RTRemoteServer {
  /**
   * create rtRemote server
   */
  constructor() {
    this.rpcSocketServer = null;
    this.rpcPort = null;
    this.rpcHost = null;
    this.serverName = `remote-server-${helper.getRandomUUID()}`;
  }

  /**
   * init rt remote server
   * 1. create udp multicast server, receive udp locate object packet and return message
   * 2. create rpc tcp socket server, receive rpc messages and return messages
   * @param {string} udpHost the multicast udp address
   * @param {number|int} udpPort the multicast udp port
   * @param {string} rpcHost the tcp bind ip address
   * @param {number|int} rpcPort the tcp bind port
   */
  init(udpHost, udpPort, rpcHost, rpcPort) {
    RTEnvironment.setRunMode(RTConst.SERVER_MODE);
    return this.openMulticastServer(udpHost, udpPort)
      .then(() => this.openRpcSocketServer(rpcHost, rpcPort))
      .then(() => this);
  }

  /**
   * open multicast server to receive udp locate object packet and return message
   * @param {string} udpHost the multicast udp address
   * @param {number|int} udpPort the multicast udp port
   * @return {Promise<dgram>} the promise with dgram
   */
  openMulticastServer(udpHost, udpPort) {
    const udpSocketIn = dgram.createSocket({
      type: 'udp4',
      reuseAddr: true,
    });

    const udpSocketOut = dgram.createSocket('udp4');
    udpSocketOut.bind(() => { // create channel to send located message
      udpSocketOut.setBroadcast(true);
      udpSocketOut.addMembership(udpHost);
    });

    return new Promise((resolve, reject) => {
      udpSocketIn.on('error', (err) => {
        logger.error(`multicast socket in error:\n${err.stack}`);
        reject(new RTException(err.message));
        udpSocketIn.close();
      });

      udpSocketIn.on('message', (buffer) => {
        const msg = RTRemoteSerializer.fromBuffer(buffer);
        const replyToURL = new URL(msg[RTConst.REPLY_TO]);
        const objectId = msg[RTConst.OBJECT_ID_KEY];

        if (registeredObjectMap[objectId]) {
          const locateMsg = RTMessageHelper.newLocateResponse(
            `tcp://${this.rpcHost}:${this.rpcPort}`,
            objectId, msg[RTConst.SENDER_ID], msg[RTConst.CORRELATION_KEY],
          );
          udpSocketOut.send(
            RTRemoteSerializer.toBuffer(locateMsg),
            replyToURL.port, replyToURL.hostname, (err) => {
              if (err) { // send failed, only log this error, no need do other actions
                logger.error(`send locate response to ${replyToURL.hostname}:${replyToURL.port} failed`);
                logger.error(err.stack);
              }
            },
          );
        } else {
          logger.warn(`client want search object ${objectId}, but not found in server ${this.serverName}.`);
        }
      });

      udpSocketIn.on('listening', () => {
        const address = udpSocketIn.address();
        logger.debug(`server bind multicast socket succeed, ${address.address}:${address.port}`);
        resolve(udpSocketIn);
      });
      udpSocketIn.bind({ port: udpPort }, () => {
        udpSocketIn.setBroadcast(true);
        udpSocketIn.addMembership(udpHost); // join to multicast channel
      });
    });
  }

  /**
   * open rpc tcp socket server receive rpc messages and return messages
   * @param {string} rpcHost the tcp bind ip address
   * @param {number|int} rpcPort the tcp bind port
   * @return {Promise<RTRemoteServer>} the remote server
   */
  openRpcSocketServer(rpcHost, rpcPort) {
    const that = this;
    return new Promise((resolve, reject) => {
      that.rpcSocketServer = net.createServer((socket) => {
        const transport = new RTRemoteTCPTransport(socket);

        // socket already created, no exceptions need catch for create protocol
        RTRemoteProtocol.create(transport, true).then((protocol) => {
          protocol.rtRemoteServer = that;
        });

        socket.on('error', (err) => {
          // only log error and close this socket, server still need continue run
          logger.error(err);
          socket.destroy();
        });
        socket.on('close', () => {
          logger.info('a client connection closed');
        });
      });

      this.rpcSocketServer.on('error', (err) => {
        logger.error(`unexpected rpc server error:\n${err.stack}`);
        reject(new RTException(err.message));
      });

      this.rpcSocketServer.listen(rpcPort, rpcHost, () => {
        that.rpcHost = that.rpcSocketServer.address().address;
        that.rpcPort = that.rpcSocketServer.address().port;
        logger.info(`rpc socket server bind succeed, ${that.rpcHost}:${that.rpcPort}`);
        resolve(that);
      });
    });
  }

  /**
   * get object by objectName/objectId
   *
   * @param {string} objName the object name
   */
  getObjectByName(objName) {
    const obj = registeredObjectMap[objName];
    if (!obj) {
      // here should not be throw null exception, it should return null, so that
      //    set property by name reponse can return OBJECT_NOT_FOUND
      logger.error(`getObjectByName object with name = ${objName} is null in server ${this.serverName}`);
    }
    return obj;
  }

  /**
   * hander client message
   * @param {RTRemoteTask} task the remote protocol
   * @return {Promise<any>} the promise with the result
   */
  handlerMessage(task) {
    switch (task.message[RTConst.MESSAGE_TYPE]) {
      case RTRemoteMessageType.GET_PROPERTY_BYNAME_REQUEST:
        return this.handlerGetPropertyByNameRequest(task);
      case RTRemoteMessageType.GET_PROPERTY_BYINDEX_REQUEST:
        return this.handlerGetPropertyByIndexRequest(task);
      case RTRemoteMessageType.SET_PROPERTY_BYNAME_REQUEST:
        return this.handlerSetPropertyByNameRequest(task);
      case RTRemoteMessageType.SET_PROPERTY_BYINDEX_REQUEST:
        return this.handlerSetPropertyByIndexRequest(task);
      case RTRemoteMessageType.METHOD_CALL_REQUEST:
        return this.handlerCallRequest(task);
      default:
        return Promise.reject(new RTException(`handlerMessage -> don't support message type ${
          task.message[RTConst.MESSAGE_TYPE]}`));
    }
  }

  /**
   * process set property by name request
   * @param {RTRemoteTask} task the remote task
   */
  handlerSetPropertyByNameRequest(task) {
    const { message } = task;
    if (message[RTConst.VALUE][RTConst.TYPE] === RTValueType.FUNCTION) {
      const rtFunction = message[RTConst.VALUE];
      message[RTConst.VALUE] = helper.updateListenerForRTFuction(task.protocol, rtFunction);
    }
    const response = helper.setProperty(this.getObjectByName(message[RTConst.OBJECT_ID_KEY]), message);
    return task.protocol.transport.send(RTRemoteSerializer.toBuffer(response));
  }

  /**
   * process set property by index request
   * @param {RTRemoteTask} task the remote task
   */
  handlerSetPropertyByIndexRequest(task) {
    const { message } = task;
    if (message[RTConst.VALUE][RTConst.TYPE] === RTValueType.FUNCTION) {
      const rtFunction = message[RTConst.VALUE];
      message[RTConst.VALUE] = helper.updateListenerForRTFuction(task.protocol, rtFunction);
    }
    const response = helper.setProperty(this.getObjectByName(message[RTConst.OBJECT_ID_KEY]), message);
    response[RTConst.MESSAGE_TYPE] = RTRemoteMessageType.SET_PROPERTY_BYINDEX_RESPONSE;
    return task.protocol.transport.send(RTRemoteSerializer.toBuffer(response));
  }

  /**
   * process set property by index request
   * @param {RTRemoteTask} task the remote task
   */
  handlerGetPropertyByIndexRequest(task) {
    const { message } = task;
    const response = helper.getProperty(this.getObjectByName(message[RTConst.OBJECT_ID_KEY]), message, this);
    response[RTConst.MESSAGE_TYPE] = RTRemoteMessageType.GET_PROPERTY_BYINDEX_RESPONSE;
    return task.protocol.transport.send(RTRemoteSerializer.toBuffer(response));
  }


  /**
   * process set property by name request
   * @param {RTRemoteTask} task the remote task
   */
  handlerGetPropertyByNameRequest(task) {
    const { message } = task;
    const response = helper.getProperty(this.getObjectByName(message[RTConst.OBJECT_ID_KEY]), message, this);
    return task.protocol.transport.send(RTRemoteSerializer.toBuffer(response));
  }

  /**
   * process call method request
   * @param {RTRemoteTask} task the remote task
   */
  handlerCallRequest(task) {
    const { message } = task;

    const args = message[RTConst.FUNCTION_ARGS] || [];
    if (args.length > 0) {
      args.forEach((arg, index) => {
        if (arg[RTConst.TYPE] === RTValueType.FUNCTION) {
          args[index] = helper.updateListenerForRTFuction(task.protocol, arg);
        }
      });
    }
    const response = helper.invokeMethod(this.getObjectByName(message[RTConst.OBJECT_ID_KEY]), message);
    return task.protocol.transport.send(RTRemoteSerializer.toBuffer(response));
  }

  /**
   * register a object with object name
   * @param {string} objectName the object name
   * @param {object} obj the object entity
   */
  registerObject(objectName, obj) {
    if (registeredObjectMap[objectName]) { // object exists
      throw new RTException(`object with name = ${objectName} already exists in server ${
        this.serverName}, please don't register again.`);
    }
    registeredObjectMap[objectName] = obj;
    logger.info(`object with name = ${objectName} register successfully in server ${this.serverName}`);
  }

  /**
   * check the objectName is register or not
   * @param objectName the object name
   */
  isRegister(objectName) { // eslint-disable-line
    return !!registeredObjectMap[objectName];
  }

  /**
   * unregister object from server
   *
   * @param {string} objectName the object name
   * @throws RTException throw errors if object not exist
   */
  unRegisterObject(objectName) {
    const object = registeredObjectMap[objectName];
    if (object == null) {
      throw new RTException(`cannot found register object named ${objectName} in server ${this.serverName}`);
    }
    registeredObjectMap[objectName] = null; // remove it
  }
}

/**
 * create rtRemote server with params
 * @param {string} udpHost the multicast udp address
 * @param {number|int} udpPort the multicast udp port
 * @param {string} rpcHost the tcp bind ip address
 * @param {undefined|number|int|null} rpcPort the tcp bind port, it will allocate by system if value is empty/null/undefined/0
 */
function create(udpHost, udpPort, rpcHost, rpcPort = null) {
  const server = new RTRemoteServer();
  return server.init(udpHost, udpPort, rpcHost, rpcPort || 0);
}

module.exports = {
  create,
};
