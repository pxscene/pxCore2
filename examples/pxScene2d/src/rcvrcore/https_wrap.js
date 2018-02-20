'use strict';

var https = require('https');

function HttpsWrap(accessControl) {
  HttpsWrap.prototype.globalAgent = https.globalAgent;

  // Server functionality needs to be disabled.
  //HttpsWrap.prototype.Server = https.Server;
  //HttpsWrap.prototype.createServer = https.createServer;

  HttpsWrap.prototype.request = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb, true) : arguments;
    return newArgs ? https.request.apply(this, newArgs) : null;
  };
  HttpsWrap.prototype.get = function (options, cb) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, cb, true) : arguments;
    return newArgs ? https.get.apply(this, newArgs) : null;
  };
  /**
   * @return {null}
   */
  HttpsWrap.prototype.Agent = function (options) {
    var newArgs = accessControl ? accessControl.wrapArgs(options, null, true) : arguments;
    return newArgs ? https.Agent.apply(this, newArgs) : null;
  };
}

module.exports = HttpsWrap;
