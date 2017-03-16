'use strict';
var net = require('net');
module.exports.connect = net.connect;
module.exports.createConnection = net.createConnection;
module.exports.Socket = net.Socket;
module.exports.isIP = net.isIP;
module.exports.isIPv4 = net.isIPv4;
module.exports.isIPv6 = net.isIPv6;
