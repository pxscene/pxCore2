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
  PROPERTY_INDEX: 'property.index',
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
