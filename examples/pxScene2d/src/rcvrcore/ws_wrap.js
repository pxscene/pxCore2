'use strict';

var WebSocket = require('ws/lib/WebSocket');

function socketClosed()
{
  arguments[0].clearConnection(this);
  this.removeListener('close',socketClosed);
}

//class exposed to client
function WebSocketWrap()
{
  this.connections = new Array();
}

WebSocketWrap.prototype.clearConnection = function(client) {
  if ((this.connections != undefined) && (this.connections.length > 0))
  {
    var index = this.connections.indexOf(client);
    if (index != -1)
    {
      this.connections.splice(index,1);
    }    
  }
}

WebSocketWrap.prototype.clearConnections = function() {
  var connectionLength = this.connections.length;
  for (var i = 0; i < connectionLength; i++) {
    this.connections[i].close();
  };
  this.connections.splice(0, this.connections.length);
  delete this.connections;
  this.connections = null;
}

WebSocketWrap.prototype.connect = WebSocketWrap.prototype.createConnection = function(address, fn) {
  var client = new WebSocket(address);
  if (typeof fn === 'function') {
    client.on('open', fn);
  }
  this.connections.push(client);
  client.on('close', socketClosed.bind(client,this));
  return client;
};

WebSocketWrap.prototype.WebSocket = function (address, protocols, options) {
  var client = new WebSocket(address, protocols, options);
  if (typeof fn === 'function') {
    client.on('open', fn);
  }
  this.connections.push(client);
  client.on('close', socketClosed.bind(client,this));
  return client;
};

//export to client
var WebSocketExport = module.exports;
WebSocketExport.WebSocket = WebSocketWrap;
module.exports = WebSocketExport;
