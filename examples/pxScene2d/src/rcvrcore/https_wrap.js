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

var https = require('https');

function HttpsWrap(accessControl) {
  HttpsWrap.prototype.globalAgent = https.globalAgent;

  // Server functionality needs to be disabled.
  //HttpsWrap.prototype.Server = https.Server;
  //HttpsWrap.prototype.createServer = https.createServer;

  HttpsWrap.prototype.request = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb, true) : arguments;
    return newArgs ? https.request.apply(this, newArgs) : null;
  };
  HttpsWrap.prototype.get = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb, true) : arguments;
    return newArgs ? https.get.apply(this, newArgs) : null;
  };
  /**
   * @return {null}
   */
  HttpsWrap.prototype.Agent = function (options) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, null, true) : arguments;
    return newArgs ? https.Agent.apply(this, newArgs) : null;
  };
}

module.exports = HttpsWrap;
