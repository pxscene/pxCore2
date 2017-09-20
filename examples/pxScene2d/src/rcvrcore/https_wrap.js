'use strict';

var https = require('https');

function HttpsWrap(accessControl) {
  // do not expose accessControl through 'this.accessControl'
  var _accessControl = accessControl;

  HttpsWrap.prototype.globalAgent = https.globalAgent;
  HttpsWrap.prototype.Server = https.Server;
  HttpsWrap.prototype.createServer = https.createServer;

  HttpsWrap.prototype.request = function (options, cb) {
    if (_accessControl) {
      if (_accessControl.isLocalAccessFromRemote(options)) {
        console.log("localhost urls cannot be accessed by remote applications");
        return;
      }
      options = _accessControl.wrapHttpRequestOptions(options);
      cb = _accessControl.wrapHttpResponseCallback(cb);
    }
    return https.request(options, cb);
  };

  HttpsWrap.prototype.get = function (options, cb) {
    if (_accessControl) {
      if (_accessControl.isLocalAccessFromRemote(options)) {
        console.log("localhost urls cannot be accessed by remote applications");
        return;
      }
      options = _accessControl.wrapHttpRequestOptions(options);
      cb = _accessControl.wrapHttpResponseCallback(cb);
    }
    return https.get(options, cb);
  };

  HttpsWrap.prototype.Agent = function (options) {
    if (_accessControl) {
      if (_accessControl.isLocalAccessFromRemote(options)) {
        console.log("localhost urls cannot be accessed by remote applications");
        return;
      }
    }
    return https.Agent(options);
  };
}

module.exports = HttpsWrap;
