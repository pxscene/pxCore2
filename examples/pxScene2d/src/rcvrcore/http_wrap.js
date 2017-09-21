'use strict';

var http = require('http');

function HttpWrap(accessControl) {
  // do not expose accessControl through 'this.accessControl'
  var _accessControl = accessControl;

  HttpWrap.prototype.IncomingMessage = http.IncomingMessage;
  HttpWrap.prototype.METHODS = http.METHODS;
  HttpWrap.prototype.OutgoingMessage = http.OutgoingMessage;
  HttpWrap.prototype.ServerResponse = http.ServerResponse;
  HttpWrap.prototype.STATUS_CODES = http.STATUS_CODES;
  HttpWrap.prototype.Server = http.Server;
  HttpWrap.prototype.createServer = http.createServer;
  HttpWrap.prototype.globalAgent = http.globalAgent;

  HttpWrap.prototype.request = function (options, cb) {
    if (_accessControl) {
      if (_accessControl.isLocalAccessFromRemote(options)) {
        console.log("localhost urls cannot be accessed by remote applications");
        return;
      }
      options = _accessControl.wrapHttpRequestOptions(options);
      cb = _accessControl.wrapHttpResponseCallback(cb);
    }
    return http.request(options, cb);
  };

  // http.request == new http.ClientRequest
  HttpWrap.prototype.ClientRequest = function (options, cb) {
    if (_accessControl) {
      if (_accessControl.isLocalAccessFromRemote(options)) {
        console.log("localhost urls cannot be accessed by remote applications");
        return;
      }
      options = _accessControl.wrapHttpRequestOptions(options);
      cb = _accessControl.wrapHttpResponseCallback(cb);
    }
    return http.ClientRequest(options, cb);
  };

  // http.get == http.request (+end)
  HttpWrap.prototype.get = function (options, cb) {
    if (_accessControl) {
      if (_accessControl.isLocalAccessFromRemote(options)) {
        console.log("localhost urls cannot be accessed by remote applications");
        return;
      }
      options = _accessControl.wrapHttpRequestOptions(options);
      cb = _accessControl.wrapHttpResponseCallback(cb);
    }
    return http.get(options, cb);
  };

  HttpWrap.prototype.Agent = function (options) {
    if (_accessControl) {
      if (_accessControl.isLocalAccessFromRemote(options)) {
        console.log("localhost urls cannot be accessed by remote applications");
        return;
      }
    }
    return http.Agent(options);
  };

  // TODO CORS?
  // deprecated
  HttpWrap.prototype.Client = function (port, host) {
    if (_accessControl) {
      if (_accessControl.isLocalAccessFromRemote(host)) {
        console.log("localhost urls cannot be accessed by remote applications");
        return;
      }
    }
    return http.Client(port, host);
  };

  // TODO CORS?
  // deprecated
  HttpWrap.prototype.createClient = function (port, host) {
    if (_accessControl) {
      if (_accessControl.isLocalAccessFromRemote(host)) {
        console.log("localhost urls cannot be accessed by remote applications");
        return;
      }
    }
    return http.createClient(port, host);
  };
}

module.exports = HttpWrap;
