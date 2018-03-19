/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * This is the exception of this rt remote.
 *
 * @author      TCSCODER
 * @version     1.0
 */

/**
 * the RTException class
 */
class RTException extends Error {
  /**
   * create new RTException
   * @param {string} message the error message
   */
  constructor(message) {
    super();

    /**
     * the error message
     * @type {string}
     */
    this.message = message;
  }
}

module.exports = RTException;
