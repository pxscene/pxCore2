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
    this.connections[i].close();
    delete this.connections[i];
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
