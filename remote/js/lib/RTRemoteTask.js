/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the rtRemote task, used to store the messages(include protocol) into task queue
 *
 * @author      TCSCODER
 * @version     1.0
 */

/**
 * the rt remote task class
 */
class RTRemoteTask {
  /**
   * create new remote task
   * @param {RTRemoteProtocol} protocol the protocol instance
   * @param {object} message the message entity
   */
  constructor(protocol, message) {
    this.protocol = protocol;
    this.message = message;
  }
}


module.exports = RTRemoteTask;
