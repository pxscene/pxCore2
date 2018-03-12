'use strict';

var http = require('http');
var AccessControl = require('rcvrcore/utils/AccessControl');

function HttpWrap(innerscene) {
  // do not expose these props through 'this.'
  var _accessControl = new AccessControl(innerscene, http.globalAgent);

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
    return _accessControl.wrapRequestPermissions(options, cb, function (options1, cb1) {
      return _accessControl.wrapRequestCORS(options1, cb1, function (options2, cb2) {
        return http.request(options2, cb2);
      });
    });
  };

  // http.request == new http.ClientRequest
  HttpWrap.prototype.ClientRequest = function (options, cb) {
    return _accessControl.wrapRequestPermissions(options, cb, function (options1, cb1) {
      return _accessControl.wrapRequestCORS(options1, cb1, function (options2, cb2) {
        return http.ClientRequest(options2, cb2);
      });
    });
  };

  // http.get == http.request (+end)
  HttpWrap.prototype.get = function (options, cb) {
    return _accessControl.wrapRequestPermissions(options, cb, function (options1, cb1) {
      return _accessControl.wrapRequestCORS(options1, cb1, function (options2, cb2) {
        return http.get(options2, cb2);
      });
    });
  };

  HttpWrap.prototype.Agent = function (options) {
    return _accessControl.wrapRequestPermissions(options, null, function (options1) {
      return http.Agent(options1);
    });
  };

  // deprecated
  //HttpWrap.prototype.Client = http.Client;
  //HttpWrap.prototype.createClient = http.createClient;
}

module.exports = HttpWrap;
