'use strict';

var WebSocket = require('ws/lib/WebSocket');

function WebSocketWrap()
{
  this.connections = new Array();
}

WebSocketWrap.prototype.clearConnections = function() {
  var connectionLength = this.connections.length;
  for (var i = 0; i < connectionLength; i++) {
    this.connections[i].close();
  };
  this.connections.splice(0, this.connections.length);
  delete this.connections;
  this.conections = null;
}

WebSocketWrap.prototype.connect = WebSocketWrap.prototype.createConnection = function(address, fn) {
  var client = new WebSocket(address);
  if (typeof fn === 'function') {
    client.on('open', fn);
  }
  this.connections.push(client);
  return client;
};

WebSocketWrap.prototype.WebSocket = function (address, protocols, options) {
  var client = new WebSocket(address, protocols, options);
  if (typeof fn === 'function') {
    client.on('open', fn);
  }
  this.connections.push(client);
  return client;
};

//export to client
var WebSocketExport = module.exports;
WebSocketExport.WebSocket = WebSocketWrap;
module.exports = WebSocketExport;
