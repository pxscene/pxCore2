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

var http = require('http');

function HttpWrap(accessControl) {
  HttpWrap.prototype.IncomingMessage = http.IncomingMessage;
  HttpWrap.prototype.METHODS = http.METHODS;
  HttpWrap.prototype.OutgoingMessage = http.OutgoingMessage;
  HttpWrap.prototype.globalAgent = http.globalAgent;

  // Server functionality needs to be disabled.
  //HttpWrap.prototype.ServerResponse = http.ServerResponse;
  //HttpWrap.prototype.STATUS_CODES = http.STATUS_CODES;
  //HttpWrap.prototype.Server = http.Server;
  //HttpWrap.prototype.createServer = http.createServer;

  HttpWrap.prototype.request = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb) : arguments;
    return newArgs ? http.request.apply(this, newArgs) : null;
  };
  /**
   * @return {null}
   */
  HttpWrap.prototype.ClientRequest = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb) : arguments;
    return newArgs ? http.ClientRequest.apply(this, newArgs) : null;
  };
  HttpWrap.prototype.get = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb) : arguments;
    return newArgs ? http.get.apply(this, newArgs) : null;
  };
  /**
   * @return {null}
   */
  HttpWrap.prototype.Agent = function (options) {
    var newArgs = accessControl ? accessControl.wrapArgs(options) : arguments;
    return newArgs ? http.Agent.apply(this, newArgs) : null;
  };

  // deprecated
  //HttpWrap.prototype.Client = http.Client;
  //HttpWrap.prototype.createClient = http.createClient;
}

module.exports = HttpWrap;
