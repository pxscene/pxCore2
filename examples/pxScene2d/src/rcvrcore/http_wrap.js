'use strict';

var http = require('http');

function HttpWrap(accessControl) {
  HttpWrap.prototype.IncomingMessage = http.IncomingMessage;
  HttpWrap.prototype.METHODS = http.METHODS;
  HttpWrap.prototype.OutgoingMessage = http.OutgoingMessage;
  HttpWrap.prototype.globalAgent = http.globalAgent;

  // Server functionality needs to be disabled.
  //HttpWrap.prototype.ServerResponse = http.ServerResponse;
  //HttpWrap.prototype.STATUS_CODES = http.STATUS_CODES;
  //HttpWrap.prototype.Server = http.Server;
  //HttpWrap.prototype.createServer = http.createServer;

  HttpWrap.prototype.request = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb) : arguments;
    return newArgs ? http.request.apply(this, newArgs) : null;
  };
  /**
   * @return {null}
   */
  HttpWrap.prototype.ClientRequest = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb) : arguments;
    return newArgs ? http.ClientRequest.apply(this, newArgs) : null;
  };
  HttpWrap.prototype.get = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb) : arguments;
    return newArgs ? http.get.apply(this, newArgs) : null;
  };
  /**
   * @return {null}
   */
  HttpWrap.prototype.Agent = function (options) {
    var newArgs = accessControl ? accessControl.wrapArgs(options) : arguments;
    return newArgs ? http.Agent.apply(this, newArgs) : null;
  };

  // deprecated
  //HttpWrap.prototype.Client = http.Client;
  //HttpWrap.prototype.createClient = http.createClient;
}

module.exports = HttpWrap;
