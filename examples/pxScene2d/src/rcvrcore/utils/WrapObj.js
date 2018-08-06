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

'use strict';

var Events = require('events');

/**
 * Creates a wrapper around {@see from} and based on {@see to}. Result has props/functions of both.
 * @param from
 * @param to
 * @param thisArg - replaces 'this' in result's functions
 * @param props - a set of props from {@see from}, otherwise all props
 * @param events - a set of events from {@see from}, otherwise all events
 * @param eventArgWrapper - if {@see events} set, use this to modify event params
 * @returns {{}}
 */
function wrap(from, to, thisArg, props, events, eventArgWrapper) {
  var keys = props ? props : [];
  if (!props) {
    for (var prop in from) {
      keys.push(prop);
    }
  }
  var ret = {};
  if (events) {
    var e = new Events();
    events.forEach(function (k) {
      from.on(k, function () {
        var args = Array.prototype.slice.call(arguments);
        if (eventArgWrapper) {
          args = args.map(eventArgWrapper);
        }
        args.unshift(k);
        e.emit.apply(e, args);
      });
    });
    if (to) {
      ret = wrap(to, e);
    } else {
      ret = e;
    }
  } else if (to) {
    ret = to;
  }
  keys.forEach(function(prop) {
    if (!ret.hasOwnProperty(prop)) {
      (function (prop) {
        if (typeof(from[prop]) === 'function') {
          ret[prop] = function () {
            return from[prop].apply(thisArg ? thisArg : this, arguments);
          };
        } else {
          Object.defineProperty(ret, prop, {
            'get': function () {
              if (from[prop] === from) {
                return ret;
              } else {
                return from[prop];
              }
            },
            'set': function (value) {
              from[prop] = value;
            }
          });
        }
      })(prop);
    }
  });
  return ret;
}

module.exports = wrap;
