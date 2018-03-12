'use strict';
var WS = module.exports = require('ws/lib/WebSocket');
WS.Sender = require('ws/lib/Sender');
WS.Receiver = require('ws/lib/Receiver');
WS.connect = WS.createConnection = function connect(address, fn) {
  var client = new WS(address);
  if (typeof fn === 'function') {
    client.on('open', fn);
  }
  return client;
};
