/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * The RT remote const values
 *
 * @author      TCSCODER
 * @version     1.0
 */

/**
 * the rt const values
 */
module.exports = {
  FUNCTION_KEY: 'function.name',
  FUNCTION_GLOBAL_SCOPE: 'global',
  CORRELATION_KEY: 'correlation.key',
  OBJECT_ID_KEY: 'object.id',
  PROPERTY_NAME: 'property.name',
  FUNCTION_ARGS: 'function.args',
  STATUS_MESSAGE: 'status.message',
  MESSAGE_TYPE: 'message.type',
  KEEP_ALIVE_IDS: 'keep_alive.ids',
  FUNCTION_RETURN_VALUE: 'function.return_value',
  SERVER_MODE: 'SERVER_MODE',
  CLIENT_MODE: 'CLIENT_MODE',
  TYPE: 'type',
  SENDER_ID: 'sender.id',
  REPLY_TO: 'reply-to',
  ENDPOINT: 'endpoint',
  STATUS_CODE: 'status.code',
  VALUE: 'value',
  UNKNOWN_CODE: 'UNKNOWN CODE',
  UNKNOWN_TYPE: 'UNKNOWN TYPE',
  UNKNOWN_MESSAGE_TYPE: 'UNKNOWN MESSAGE TYPE',
  /**
   * the first time to find object, then exponential backoff, the unit is ms
   */
  FIRST_FIND_OBJECT_TIME: 10,

  /**
   * the default charset
   */
  DEFAULT_CHARSET: 'utf8',

  /**
   * protocol use int as packet header, so the length is 4
   */
  PROTOCOL_HEADER_LEN: 4,

  /**
   * the request timeout, unit is second
   */
  REQUEST_TIMEOUT: 10 * 1000,
};
