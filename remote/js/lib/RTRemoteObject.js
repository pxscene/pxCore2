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
   * @return {Promise<object>} the promise with object/rtValue
   */
  set(name, value) {
    const propertyType = typeof name;
    if (propertyType === 'string') {
      return this.protocol.sendSetByName(this.id, name, value);
    } else if (propertyType === 'number') {
      return this.protocol.sendSetById(this.id, name, value);
    }
    return Promise.reject(new Error(`unsupport set type = ${propertyType}`));
  }

  /**
   * get property by name or index
   * @param {string|number} name the property name or index
   * @return {Promise<object>} the promise with object/rtValue
   */
  get(name) {
    const propertyType = typeof name;
    if (propertyType === 'string') {
      return this.protocol.sendGetByName(this.id, name);
    } else if (propertyType === 'number') {
      return this.protocol.sendGetById(this.id, name);
    }
    return Promise.reject(new Error(`unsupport get type = ${propertyType}`));
  }

  /**
   * send call request with void returns
   * @param {string} name the method name
   * @param {array} args the call function args
   * @return {Promise<void>} the promise with void rtvalue
   */
  send(name, ...args) {
    return this.protocol.sendCallByName(this.id, name, ...args);
  }

  /**
   * send call request with returns, in fact, this method same as *send*
   * @param {string} name the method name
   * @param {array} args the call function args
   * @return {Promise<object>} the promise with object/rtValue
   */
  sendReturns(name, ...args) {
    return this.protocol.sendCallByName(this.id, name, ...args);
  }
}

module.exports = RTRemoteObject;
