'use strict';

var https = require('https');
var AccessControl = require('rcvrcore/utils/AccessControl');

function HttpsWrap(innerscene) {
  // do not expose these props through 'this.'
  var _accessControl = new AccessControl(innerscene, https.globalAgent);

  HttpsWrap.prototype.globalAgent = https.globalAgent;

  // Server functionality needs to be disabled.
  //HttpsWrap.prototype.Server = https.Server;
  //HttpsWrap.prototype.createServer = https.createServer;

  HttpsWrap.prototype.request = function (options, cb) {
    return _accessControl.wrapRequestPermissions(options, cb, function (options1, cb1) {
      return _accessControl.wrapRequestCORS(options1, cb1, function (options2, cb2) {
        return https.request(options2, cb2);
      });
    });
  };

  HttpsWrap.prototype.get = function (options, cb) {
    return _accessControl.wrapRequestPermissions(options, cb, function (options1, cb1) {
      return _accessControl.wrapRequestCORS(options1, cb1, function (options2, cb2) {
        return https.get(options2, cb2);
      });
    });
  };

  HttpsWrap.prototype.Agent = function (options) {
    return _accessControl.wrapRequestPermissions(options, null, function (options1) {
      return https.Agent(options1);
    });
  };
}

module.exports = HttpsWrap;
