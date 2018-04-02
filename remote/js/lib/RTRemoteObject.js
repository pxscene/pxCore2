/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The rt remote object
 *
 * @author      TCSCODER
 * @version     1.0
 */


class RTRemoteObject {
  /**
   * create new RTRemoteObject
   * @param {RTRemoteProtocol} protocol the remote protocol
   * @param {string } objectId the object id
   */
  constructor(protocol, objectId) {
    this.protocol = protocol;
    this.id = objectId;
  }

  /**
   * set property by name or index
   * @param {string|number} name the property name or index
   * @param {object} value the rtValue
   * @param {number|int} index the array index for property
   * @return {promise<object> | Promise<{}>} the promise with object/rtValue
   */
  set(name, value, index = -1) {
    if (index >= 0) {
      return this.protocol.sendSetByIndex(this.id, name, index, value);
    }
    return this.protocol.sendSetByName(this.id, name, value);
  }

  /**
   * get property by name or index
   * @param {string|number} name the property name or index
   * @param {number|int} index the array index for property
   * @return {Promise<object>} the promise with object/rtValue
   */
  get(name, index = -1) {
    if (index >= 0) {
      return this.protocol.sendGetByIndex(this.id, name, index);
    }
    return this.protocol.sendGetByName(this.id, name);
  }

  /**
   * send call request and void value
   * @param {string} name the method name
   * @param {array} args the arguments used to invoke remote function
   * @return {Promise<void>} the promise with void rtvalue
   */
  send(name, ...args) {
    return this.protocol.sendCallByName(this.id, name, ...args);
  }

  /**
   * send call request and return rt value, in fact, this method same as *send*
   * @param {string} name the method name
   * @param {array} args the arguments used to invoke remote function
   * @return {Promise<object>} the promise with object/rtValue
   */
  sendReturns(name, ...args) {
    return this.protocol.sendCallByName(this.id, name, ...args);
  }
}

module.exports = RTRemoteObject;
