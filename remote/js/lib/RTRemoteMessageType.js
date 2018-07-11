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
 * The remote message type
 *
 * @author      TCSCODER
 * @version     1.0
 */

module.exports = {
  SESSION_OPEN_REQUEST: 'session.open.request',
  SESSION_OPEN_RESPIONSE: 'session.open.response',
  GET_PROPERTY_BYNAME_REQUEST: 'get.byname.request',
  GET_PROPERTY_BYNAME_RESPONSE: 'get.byname.response',
  SET_PROPERTY_BYNAME_REQUEST: 'set.byname.request',
  SET_PROPERTY_BYNAME_RESPONSE: 'set.byname.response',
  GET_PROPERTY_BYINDEX_REQUEST: 'get.byindex.request',
  GET_PROPERTY_BYINDEX_RESPONSE: 'get.byindex.response',
  SET_PROPERTY_BYINDEX_REQUEST: 'set.byindex.request',
  SET_PROPERTY_BYINDEX_RESPONSE: 'set.byindex.response',
  KEEP_ALIVE_REQUEST: 'keep_alive.request',
  KEEP_ALIVE_RESPONSE: 'keep_alive.response',
  METHOD_CALL_RESPONSE: 'method.call.response',
  METHOD_CALL_REQUEST: 'method.call.request',
  SEARCH_OBJECT: 'search',
  LOCATE_OBJECT: 'locate',
};
