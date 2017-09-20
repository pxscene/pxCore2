'use strict';

var url = require('url');

function AccessControl(packageUrl) {
  this.origin = AccessControl.parseOrigin(packageUrl);
  this.localApp = AccessControl.isLocalAccess(packageUrl);
  //console.log("AccessControl : origin="+this.origin+" localApp="+this.localApp+" FOR packageUrl="+packageUrl);
}

AccessControl.prototype.isLocalAccessFromRemote = function(options) {
  return false === this.localApp && true === AccessControl.isLocalAccess(options);
};

AccessControl.prototype.wrapHttpRequestOptions = function(options) {
  if (!this.origin) {
    return options;
  }
  // options <string>
  if (typeof options === 'string') {
    options = url.parse(options);
    // options <URL>
    if (!options.hostname) {
      throw new Error('Unable to determine the domain name');
    }
  }
  // options <Object> | <URL>
  options = Object.assign({}, options);
  // options <Object>
  if (!options.headers) {
    options.headers = {};
  }
  options.headers[AccessControl.HTTP_HEADER_ORIGIN] = this.origin;
  return options;
};

AccessControl.prototype.wrapHttpResponseCallback = function(cb) {
  if (!this.origin) {
    return cb;
  }
  var _origin = this.origin;
  return function (response) {
    var allowOrigin = response.headers ?
      response.headers[AccessControl.HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN.toLowerCase()] :
      null;
    if (!allowOrigin) {
      console.error("CORS ERROR : origin="+_origin+" allow_origin=N/A");
      response.destroy(AccessControl.ERROR_NO_ACCESS_CONTROL_ALLOW_ORIGIN(_origin));
    } else if (allowOrigin !== AccessControl.ANONYMOUS_ACCESS_CONTROL_ALLOW_ORIGIN &&
      allowOrigin.toLowerCase() !== _origin) {
      console.error("CORS ERROR : origin="+_origin+" allow_origin="+allowOrigin);
      response.destroy(AccessControl.ERROR_DENY_ACCESS_CONTROL_ALLOW_ORIGIN(_origin, allowOrigin));
    } else {
      cb(response);
    }
  };
};

AccessControl.HTTP_HEADER_ORIGIN = "Origin";
AccessControl.HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN = "Access-Control-Allow-Origin";
AccessControl.ANONYMOUS_ACCESS_CONTROL_ALLOW_ORIGIN = "*";
AccessControl.LOCALHOST_PATTERN = /^(localhost|127\.0\.0\.1|\[::1\]|\[0:0:0:0:0:0:0:1\]|::1|0:0:0:0:0:0:0:1)$/;

AccessControl.ERROR_NO_ACCESS_CONTROL_ALLOW_ORIGIN = function (origin) {
  return new Error(
    "No '"+AccessControl.HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN+
    "' header is present on the requested resource. " +
    "Origin '"+origin+"' is therefore not allowed access"
  );
};

AccessControl.ERROR_DENY_ACCESS_CONTROL_ALLOW_ORIGIN = function (origin, allowOrigin) {
  return new Error(
    "The '"+AccessControl.HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN+
    "' header has a value '"+allowOrigin+"' that is not equal to the supplied origin. " +
    "Origin '"+origin+"' is therefore not allowed access"
  );
};

AccessControl.isLocalAccess = function(options) {
  // options <string>
  if (typeof options === 'string') {
    options = url.parse(options);
    // options <URL>
  }
  // options <Object> | <URL>
  if (options.host || options.hostname) {
    return AccessControl.LOCALHOST_PATTERN.test(options.host) ||
      AccessControl.LOCALHOST_PATTERN.test(options.hostname);
  }
  // has no host means file system
  return true;
};

AccessControl.parseOrigin = function(packageUrl) {
  if (typeof packageUrl === 'string') {
    var urlObject = url.parse(packageUrl);
    if (urlObject.host && urlObject.protocol) {
      return urlObject.protocol + (urlObject.slashes ? "//" : "") + urlObject.host;
    }
  }
  // has no host means file system
  return null;
};

module.exports = AccessControl;
