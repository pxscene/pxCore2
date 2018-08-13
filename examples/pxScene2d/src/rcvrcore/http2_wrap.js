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

var http2 = require('http2');

// NOTE: http2 functionality limited to client-only.

// Common server and client side code
// ==================================

module.exports.STATUS_CODES = http2.STATUS_CODES;
module.exports.IncomingMessage = http2.IncomingMessage;
module.exports.OutgoingMessage = http2.OutgoingMessage;
module.exports.protocol = http2.protocol;

// Bunyan serializers exported by submodules that are worth adding when creating a logger.
module.exports.serializers = http2.serializers;

// Server side
// ===========

//module.exports.Server = http2.Server;
//module.exports.IncomingRequest = http2.IncomingRequest;
//module.exports.OutgoingResponse = http2.OutgoingResponse;
//module.exports.ServerResponse = http2.OutgoingResponse; // for API compatibility

// Exposed main interfaces for HTTPS connections (the default)
module.exports.https = http2.https;
//module.exports.createServer = http2.createServer;
module.exports.request = http2.request;
module.exports.get = http2.get;

// Exposed main interfaces for raw TCP connections (not recommended)
//module.exports.raw = http2.raw;
//module.exports.http = http2.http;

// Client side
// ===========

module.exports.ClientRequest = http2.OutgoingRequest; // for API compatibility
module.exports.OutgoingRequest = http2.OutgoingRequest;
module.exports.IncomingResponse = http2.IncomingResponse;
module.exports.Agent = http2.Agent;
module.exports.globalAgent = http2.globalAgent;
