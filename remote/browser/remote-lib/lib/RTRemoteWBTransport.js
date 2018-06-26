/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The rt remote websocket tranport, used create websocket connection and read/send data
 *
 * @author      TCSCODER
 * @version     1.0
 */

const logger = require('./common/logger');
const RTException = require('./RTException');

/**
 * the rt remote websocket transport class
 */
class RTRemoteWBTransport {
  /**
   * create new RTRemoteWBTransport
   * @param {string} uri the webscoket uri
   */
  constructor(uri) {
    /**
     * the websocket server uri
     * @type {string}
     */
    this.uri = null;

    /**
     * the websocket instance
     * @type {socket}
     */
    this.socket = null;

    /**
     * represents transport is running or not
     * @type {boolean}
     */
    this.mRunning = true;

    /**
     * the close callback envets
     * @type {Array}
     */
    this.closeCallBack = [];

    if (typeof uri === 'string') {
      this.uri = uri;
    } else {
      this.socket = uri;
    }
  }

  /**
   * create websocket connection and open it
   * @return {Promise<RTRemoteWBTransport>} the promise with RTRemoteWBTransport
   */
  open() {
    return new Promise((resolve, reject) => {
      const transport = this;
      this.socket = new WebSocket(this.uri);  // eslint-disable-line

      this.socket.onopen = () => {
        logger.info('new web socket connection to uri');
        resolve(transport);
      };
      this.socket.onclose = (evt) => {
        this.mRunning = false;
        for (let i = 0; i < this.closeCallBack.length; i += 1) {
          if (this.closeCallBack[i]) {
            this.closeCallBack[i](evt);
          }
        }
        logger.info('a connection closed');
      };
      this.socket.onerror = (err) => {
        logger.error(err);
        if (transport.mRunning) { // should close socket
          transport.socket.close();
        }
        reject(new RTException(err));
      };
    });
  }

  /**
   * send string to dest host
   * @param {String} objectString the send string
   */
  send(objectString) {
    if (this.mRunning) {
      this.socket.send(objectString);
    } else {
      throw new RTException('cannot send because of transport mRunning = false');
    }
  }

  /**
   * add events
   * @param event the event name
   * @param cb the callback
   */
  on(event, cb) {
    if (event === 'close') {
      this.closeCallBack.push(cb);
    }
  }
}

module.exports = RTRemoteWBTransport;
