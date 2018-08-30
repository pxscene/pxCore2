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

var request = require('rcvrcore/utils/AccessControl').request;

function Http2Wrap(accessControl, defaultToHttp1) {
  this.request = function (options, callback) {
    return request(accessControl, options, callback, defaultToHttp1);
  };
  this.get = function (options, callback) {
    var req = this.request.apply(this, arguments);
    req.end();
    return req;
  };
}

module.exports = Http2Wrap;
