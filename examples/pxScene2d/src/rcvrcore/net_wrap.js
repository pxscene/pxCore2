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
var net = require('net');
module.exports.connect = net.connect;
module.exports.createConnection = net.createConnection;
module.exports.Socket = net.Socket;
module.exports.isIP = net.isIP;
module.exports.isIPv4 = net.isIPv4;
module.exports.isIPv6 = net.isIPv6;
