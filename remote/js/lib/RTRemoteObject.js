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
   * @param {string|number} prop the property name or index
   * @param {object} value the rtValue
   * @return {promise<object> | Promise<{}>} the promise with object/rtValue
   */
  set(prop, value) {
    if (typeof prop === 'number') {
      return this.protocol.sendSetByIndex(this.id, prop, value);
    }
    return this.protocol.sendSetByName(this.id, prop, value);
  }

  /**
   * get property by name or index
   * @param {string|number} prop the property name or index
   * @return {Promise<object>} the promise with object/rtValue
   */
  get(prop) {
    if (typeof prop === 'number') {
      return this.protocol.sendGetByIndex(this.id, prop);
    }
    return this.protocol.sendGetByName(this.id, prop);
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
