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
 * The rt value types
 *
 * @author      TCSCODER
 * @version     1.0
 */

module.exports = {
  VOID: 0, // \0
  VALUE: 118, // v
  BOOLEAN: 98, // b
  INT8: 49, // 1
  UINT8: 50, // 2
  INT32: 52, // 4
  UINT32: 53, // 5
  INT64: 54, // 6
  UINT64: 55, // 7
  FLOAT: 101, // e
  DOUBLE: 100, // d
  STRING: 115, // s
  OBJECT: 111, // o
  FUNCTION: 102, // f
  VOIDPTR: 122, // z
};
