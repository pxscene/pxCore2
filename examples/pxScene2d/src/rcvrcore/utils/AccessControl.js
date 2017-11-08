'use strict';

var url = require('url');

function AccessControl(innerscene, agent) {
  this.innerscene = innerscene;
  this.agent = agent;
}

AccessControl.prototype.wrapRequestCORS = function(options, cb, originalFunction) {
  var _origin = this.innerscene.origin;
  if (!_origin) {
    return originalFunction(options, cb);
  }

  // 1. add CORS headers
  options = this.convertOptionsToObject(options);
  if (!options.headers) {
    options.headers = {};
  }
  options.headers["Origin"] = _origin;

  // extract url to check if it is same-origin request...
  var url = this.convertOptionsToUrlString(options);
  if (_origin === this.innerscene.getUrlOrigin(url)) {
    return originalFunction(options, cb);
  }

  // 2. check response headers
  var _originalCb = cb;
  var _this = this;
  cb = function (response) {
    var rawHeaders = "";
    for (var key in response.headers) {
      if (response.headers.hasOwnProperty(key)) {
        rawHeaders += (rawHeaders ? "\r\n" : "") + key + ": " + response.headers[key];
      }
    }
    if (!_this.innerscene.checkAccessControlHeaders(rawHeaders)) {
      response.destroy("CORS block");
    } else if (_originalCb) {
      _originalCb(response);
    }
  };

  return originalFunction(options, cb);
};

AccessControl.prototype.wrapRequestPermissions = function(options, cb, originalFunction) {
  options = this.convertOptionsToObject(options);
  var url = this.convertOptionsToUrlString(options);
  return this.innerscene.allows(url) ? originalFunction(options, cb) : null;
};

AccessControl.prototype.convertOptionsToObject = function (options) {
  if (typeof options === 'string') {
    options = url.parse(options);
    if (!options.hostname) {
      throw new Error('Unable to determine the domain name');
    }
  } else {
    options = util._extend({}, options);
  }
  return options;
};

AccessControl.prototype.convertOptionsToUrlString = function (options) {
  var protocol = options.protocol || this.agent.protocol;
  var port = options.port;
  var host = options.hostname || options.host || 'localhost';
  var path = options.path || "";
  return protocol + (protocol.indexOf("//") > 0 ? "" : "//") + host + (port ? ":" + port : "") + path;
};

module.exports = AccessControl;
