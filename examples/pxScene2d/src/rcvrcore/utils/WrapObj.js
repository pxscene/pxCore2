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
 * Note: 'this.' properties must be declared in constructor or prototype otherwise the wrap won't have them.
 * @param orig - object
 * @param override - object or null
 * @param useOriginalThis - true/false
 * @param onlySelectedProps - array
 * @returns {{}}
 */
module.exports = function (orig, override, useOriginalThis, onlySelectedProps) {
  var ret = override ? override : {};
  var keys = onlySelectedProps ? onlySelectedProps : [];
  if (!onlySelectedProps) {
    for (var prop in orig) {
      keys.push(prop);
    }
  }
  keys.forEach(function(prop) {
    if (!ret.hasOwnProperty(prop)) {
      (function (prop) {
        if (typeof(orig[prop]) === 'function') {
          ret[prop] = function () {
            return orig[prop].apply(useOriginalThis ? orig : this, arguments);
          };
        } else {
          Object.defineProperty(ret, prop, {
            'get': function () {
              if (orig[prop] === orig) {
                return ret;
              } else {
                return orig[prop];
              }
            },
            'set': function (value) {
              orig[prop] = value;
            }
          });
        }
      })(prop);
    }
  });
  return ret;
};
