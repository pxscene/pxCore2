'use strict';
var WS = require('ws/lib/WebSocket');

function socketClosed()
{
  arguments[0].clearConnection(this);
  this.removeListener('close',socketClosed);
}

//class exposed to client
//this manages the list of connections opened
function WebSocketManager()
{
  this.connections = new Array();
}

WebSocketManager.prototype.clearConnection = function(client) {
  if ((this.connections != undefined) && (this.connections.length > 0))
  {
    var index = this.connections.indexOf(client);
    if (index != -1)
    {
      delete this.connections[index];
      this.connections.splice(index,1);
    }    
  }
}

WebSocketManager.prototype.clearConnections = function() {
  var connectionLength = this.connections.length;
  for (var i = 0; i < connectionLength; i++) {
    if ((null != this.connections[i]) && (undefined != this.connections[i])) {
      // make sure we close the socket connection and don't wait for process exit to determine it
      this.connections[i].close();
      // make sure we call listeners of close now, else it is taking some seconds to get landed
      this.connections[i].closeimmediate();
      // make sure we remove all listeners registered for websocket
      // this is holding javascript variables reference, causing leaks
      // having check here, don't know who is setting this value to undefined
      if ((null != this.connections[i]) && (undefined != this.connections[i])) {
        this.connections[i].removeAllListeners('open');
        this.connections[i].removeAllListeners('error');
        this.connections[i].removeAllListeners('message');
        this.connections[i].removeAllListeners('close');
        delete this.connections[i];
      }
    }
  };
  this.connections.splice(0, this.connections.length);
  delete this.connections;
  this.connections = null;
}

WebSocketManager.prototype.WebSocket = function (address, protocols, options) {
  var client = new WS(address, protocols, options);
  if (typeof fn === 'function') {
    client.on('open', fn);
  }
  this.connections.push(client);
  client.on('close', socketClosed.bind(client,this));
  return client;
};

module.exports = WebSocketManager;
