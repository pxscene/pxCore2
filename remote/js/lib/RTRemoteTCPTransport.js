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
 * The rt remote tcp tranport, used create tcp connection and read/send data
 *
 * @author      TCSCODER
 * @version     1.0
 */

const logger = require('./common/logger');
const net = require('net');
const RTConst = require('./RTConst');
const RTException = require('./RTException');
/**
 * the rt remote tcp transport class
 */
class RTRemoteTCPTransport {
  /**
   * create new RTRemoteTCPTransport
   * @param {string} host the host address
   * @param {number|int|null} port the host port
   */
  constructor(host, port = null) {
    /**
     * the tcp host name
     * @type {string}
     */
    this.host = null;

    /**
     * the host port
     * @type {number/int}
     */
    this.port = null;

    /**
     * the tcp socket
     * @type {socket}
     */
    this.socket = null;

    /**
     * represents transport is running or not
     * @type {boolean}
     */
    this.mRunning = true;

    if (typeof host === 'string') {
      this.host = host;
      this.port = port;
    } else {
      this.socket = host;
    }
  }

  /**
   * create tcp connection and open it
   * @return {Promise<RTRemoteTCPTransport>} the promise with RTRemoteTCPTransport
   */
  open() {
    return new Promise((resolve, reject) => {
      const transport = this;
      this.socket = net.connect(transport.port, transport.host, () => {
        logger.info(`new tcp connection to ${transport.host}:${transport.port}`);
        resolve(transport);
      });
      this.socket.on('error', (err) => {
        logger.error(err);
        if (transport.mRunning) { // should close socket
          transport.socket.destroy();
        }
        reject(new RTException(err.message));
      });
      this.socket.on('close', () => {
        this.mRunning = false;
        logger.info('a connection closed');
      });
    });
  }

  /**
   * send buffer to dest host
   * @param {Buffer} buffer the send buffer
   */
  send(buffer) {
    if (this.mRunning) {
      const sendBuff = Buffer.alloc(RTConst.PROTOCOL_HEADER_LEN + buffer.length);
      sendBuff.writeUInt32BE(buffer.length, 0); // write buffer header
      sendBuff.fill(buffer, RTConst.PROTOCOL_HEADER_LEN); // copy content buffer
      this.socket.write(sendBuff, RTConst.DEFAULT_CHARSET); // send buffer
    } else {
      throw new RTException('cannot send because of transport mRunning = false');
    }
  }
}

module.exports = RTRemoteTCPTransport;
