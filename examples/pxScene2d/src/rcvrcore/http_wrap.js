'use strict';
var http = require('http');

module.exports.IncomingMessage = http.IncomingMessage;
module.exports.Agent = http.Agent;
module.exports.METHODS = http.METHODS;
module.exports.OutgoingMessage = http.OutgoingMessage;
module.exports.globalAgent = http.globalAgent;
module.exports.ClientRequest = http.ClientRequest;
module.exports.request = http.request;
module.exports.get = http.get;
module.exports.Client = http.Client;
module.exports.createClient = http.createClient;
