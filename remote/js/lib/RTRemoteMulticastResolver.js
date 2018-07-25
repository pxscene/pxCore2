/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The rt remote multicast resolver
 *
 * @author      TCSCODER
 * @version     1.0
 */

const logger = require('./common/logger');
const dgram = require('dgram');
const ip = require('ip');
const RTMessageHelper = require('./RTMessageHelper');
const RTRemoteSerializer = require('./RTRemoteSerializer');
const RTConst = require('./RTConst');
const RTException = require('./RTException');
/**
 * return rt remote object cache map
 * @type {object} the object map
 */
const objectMap = {};

/**
 * the remote multicast resolver class
 */
class RTRemoteMulticastResolver {
  /**
   * create new RTRemoteMulticastResolver
   * @param {string} address the udp address
   * @param {number/int} port the udp port
   */
  constructor(address, port) {
    this.address = address;
    this.port = port;

    /**
     * the udp scoket in, used to read locate response message
     * @type {dgram} udp socket v4
     */
    this.udpSocketIn = null;

    /**
     * the udp socket out, used to send locate object message
     * @type {dgram} udp socket v4
     */
    this.udpSocketOut = null;
  }

  /**
   * create udp server to read search response and create udp socket to send search response
   * @return {Promise} the promise resolve when socket done
   */
  start() {
    // create udp socket to send search response
    this.udpSocketOut = dgram.createSocket({
      type: 'udp4',
    });
    this.udpSocketOut.bind(() => {
      this.udpSocketOut.setBroadcast(true);
      this.udpSocketOut.addMembership(this.address);
    });

    // create udp server to read search response
    this.udpSocketIn = dgram.createSocket('udp4');
    return new Promise((resolve, reject) => {
      this.udpSocketIn.on('error', (err) => {
        logger.error(`server error:\n${err.stack}`);
        reject(new RTException(err.message));
        this.udpSocketIn.close();
      });

      this.udpSocketIn.on('message', (msg) => {
        const locatedObj = RTRemoteSerializer.fromBuffer(msg);
        const objectId = locatedObj[RTConst.OBJECT_ID_KEY];
        objectMap[objectId] = `${locatedObj[RTConst.ENDPOINT]}/${objectId}`;
      });

      this.udpSocketIn.on('listening', () => {
        const address = this.udpSocketIn.address();
        logger.debug(`search udp socket in listening ${address.address}:${address.port}`);
        resolve();
      });
      this.udpSocketIn.bind({ address: ip.address(), port: 0 });
    });
  }

  /**
   * get local udp uri to recv packet
   * @return {string} the udp uri
   */
  getReplyEndpoint() {
    const address = this.udpSocketIn.address();
    return `udp://${address.address}:${address.port}`;
  }

  /**
   * search remote object, this method will be block thread
   * 1. send udp packet
   * 2. check remote object found or not
   * 3. if not found wait 2*previous time, then send udp packet again
   * @param {string} objectId the object name
   * @return {Promise} the search promise
   */
  locateObject(objectId) {
    const locateObj = RTMessageHelper.newLocateRequest(objectId, this.getReplyEndpoint());
    const locateBuffer = RTRemoteSerializer.toBuffer(locateObj);
    let intervalId = null;

    return new Promise((resolve, reject) => {
      let preCheckTime = Date.now();
      let diff = 0;
      let now = 0;
      let seachTimeMultiple = 1;
      let totalCostTime = 0;

      const sendSearchMessage = () => {
        now = Date.now();
        diff = now - preCheckTime;

        if (objectMap[objectId]) {
          const uri = objectMap[objectId];
          objectMap[objectId] = null;
          clearInterval(intervalId);
          resolve(uri);
          return;
        }

        const currentTime = RTConst.FIRST_FIND_OBJECT_TIME * seachTimeMultiple;
        if (diff >= currentTime) {
          totalCostTime += currentTime;
          logger.debug(`searching object ${objectId}, cost = ${totalCostTime / 1000.0}s`);
          seachTimeMultiple *= 2;
          preCheckTime = now;

          // do next search
          this.udpSocketOut.send(locateBuffer, this.port, this.address, (err) => {
            if (err) {
              clearInterval(intervalId);
              reject(err);
            } else {
              preCheckTime = Date.now();
            }
          });
        }
      };
      intervalId = setInterval(sendSearchMessage, 10); // mock threads
    });
  }
}

module.exports = RTRemoteMulticastResolver;
