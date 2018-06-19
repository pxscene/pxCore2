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

var http = require('http');

function HttpWrap(accessControl) {
  HttpWrap.prototype.request = function (options, callback) {
    if (accessControl) {
      return accessControl.createClientRequest(options, callback, "http://");
    }
    return http.request.apply(null, arguments);
  };

  HttpWrap.prototype.get = function (options, callback) {
    var req = this.request.apply(this, arguments);
    req.end();
    return req;
  };
}

// No not expose sockets.
//HttpWrap.prototype.globalAgent = http.globalAgent;
//HttpWrap.prototype.Agent = http.Agent;
//HttpWrap.prototype.IncomingMessage = http.IncomingMessage;
//HttpWrap.prototype.OutgoingMessage = http.OutgoingMessage;

// Use 'request' instead
//HttpWrap.prototype.ClientRequest

// Server functionality needs to be disabled.
//HttpWrap.prototype.ServerResponse = http.ServerResponse;
//HttpWrap.prototype.STATUS_CODES = http.STATUS_CODES;
//HttpWrap.prototype.Server = http.Server;
//HttpWrap.prototype.createServer = http.createServer;

//HttpWrap.prototype.Client = http.Client;
//HttpWrap.prototype.createClient = http.createClient;

module.exports = HttpWrap;
