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

var isV8=(typeof _isV8 !== "undefined");

var http2 = isV8?null:require('http2');
var https = require('https');
var http = require('http');
var url = require('url');
var EventEmitter = require('events');

var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('http_wrap');

function Module(moduleName, appSceneContext) {
  this.request = function (options, callback) {
    return new Request(moduleName, appSceneContext, options, callback);
  };
  this.get = function (options, callback) {
    return new Request(moduleName, appSceneContext, options, callback).end();
  };
}

module.exports = Module;

function Request(moduleName, appSceneContext, options, callback) {
  EventEmitter.call(this);

  if (callback) {
    this.once('response', callback);
  }

  var self = this;
  options = Utils._normalizeOptions(options);
  var defaultProtocol = moduleName === 'http' ? 'http:' : 'https:';
  var toOrigin = Utils._getRequestOrigin(options, defaultProtocol);
  var fromOrigin = null;
  var withCredentials = false;
  var isBlocked = false;
  var httpRequest = null;
  Utils._assert(!!toOrigin, "no destination origin");

  if (appSceneContext &&
    appSceneContext.innerscene &&
    appSceneContext.innerscene.cors) {
    fromOrigin = appSceneContext.innerscene.cors.origin;
  }

  if (appSceneContext &&
    appSceneContext.innerscene &&
    appSceneContext.innerscene.permissions &&
    !appSceneContext.innerscene.permissions.allows(toOrigin)) {
    this.blocked = isBlocked = true;
    var message = "Permissions block for request to '" + toOrigin + "' from '" + fromOrigin + "'";
    log.warn(message);
    setTimeout(function () {
      log.message(4, "emit 'blocked' delayed after request creation");
      self.emit('blocked', new Error(message));

      // clean up
      self.removeAllListeners();
    });
  }

  if (appSceneContext &&
    appSceneContext.innerscene &&
    appSceneContext.innerscene.cors &&
    !isBlocked &&
    fromOrigin) {
    var h = options.headers ? options.headers : (options.headers = {});
    var keys = Object.keys(h);
    for (var i = 0; i < keys.length; i++) {
      var k = keys[i];
      if (appSceneContext.innerscene.cors.isCORSRequestHeader(k)) {
        log.warn("removing header: '" + k + "'=" + h[k]);
        delete h[k];
      } else if (appSceneContext.innerscene.cors.isCredentialsRequestHeader(k)) {
        withCredentials = true;
      }
    }
    log.message(4, "set header: 'Origin'=" + fromOrigin + "'");
    h.Origin = fromOrigin;
  }

  if (!isBlocked) {
    if (isV8 && !options.protocol) {
      options.protocol = defaultProtocol;
    }
    var module = moduleName === 'http' ? http : (moduleName === 'https' ? https : (isV8 ? https : http2));
    httpRequest = module.request.call(null, options);

    httpRequest.once('response', function (httpResponse) {
      var response = new Response(httpResponse, appSceneContext, fromOrigin, toOrigin, withCredentials);
      if (response.blocked) {
        self.blocked = true;
        self.abort();
        log.message(4, "emit 'blocked'");
        self.emit('blocked', new Error("CORS block for: '" + toOrigin + "' from '" + fromOrigin + "'"));
      } else {
        self.emit('response', response);
      }

      // clean up
      self.removeAllListeners();
      httpRequest.removeAllListeners();
    });
    httpRequest.once('error', function (e) {
      self.emit('error', e);

      // clean up
      self.removeAllListeners();
      httpRequest.removeAllListeners();
    });
    httpRequest.once('timeout', function () {
      self.emit('timeout');

      // clean up
      self.removeAllListeners();
      httpRequest.removeAllListeners();
    });
  }

  this.abort = function () {
    if (!isBlocked) {
      if (isV8) {
        httpRequest.abort();
      } else {
        httpRequest.abort.apply(httpRequest, arguments);
      }
    }
  };
  this.getHeader = function () {
    if (!isBlocked) {
      if (isV8) {
        return httpRequest.getHeader(arguments[0]);
      } else {
        return httpRequest.getHeader.apply(httpRequest, arguments);
      }
    }
  };
  this.setNoDelay = function () {
    if (!isBlocked) {
      if (isV8) {
        log.warn("setNoDelay not implemented for v8");
      } else {
        httpRequest.setNoDelay.apply(httpRequest, arguments);
      }
    }
  };
  this.setSocketKeepAlive = function () {
    if (!isBlocked) {
      if (isV8) {
        log.warn("setSocketKeepAlive not implemented for v8");
      } else {
        httpRequest.setSocketKeepAlive.apply(httpRequest, arguments);
      }
    }
  };
  this.setDefaultEncoding = function () {
    if (!isBlocked) {
      if (isV8) {
        log.warn("setSocketKeepAlive not implemented for v8");
      } else {
        httpRequest.setDefaultEncoding.apply(httpRequest, arguments);
      }
    }
    return self;
  };
  this.setTimeout = function () {
    if (!isBlocked) {
      if (isV8) {
        httpRequest.setTimeout(arguments[0], arguments[1]);
      } else {
        httpRequest.setTimeout.apply(httpRequest, arguments);
      }
    }
    return self;
  };
  this.end = function () {
    if (!isBlocked) {
      if (isV8) {
        httpRequest.end();
      } else {
        httpRequest.end.apply(httpRequest, arguments);
      }
    }
    return self;
  };
  this.write = function () {
    if (!isBlocked) {
      if (isV8) {
        return httpRequest.write(arguments[0]);
      } else {
        return httpRequest.write.apply(httpRequest, arguments);
      }
    }
  };
}

Request.prototype = Object.create(EventEmitter.prototype);
Request.prototype.constructor = Request;
Request.prototype.blocked = false;

function Response(httpResponse, appSceneContext, fromOrigin, toOrigin, withCredentials) {
  EventEmitter.call(this);

  var isBlocked = false;
  var rawHeaders = Utils._packHeaders(httpResponse.headers);
  log.message(4, "response headers: " + rawHeaders);
  if (appSceneContext &&
    appSceneContext.innerscene &&
    appSceneContext.innerscene.cors &&
    !appSceneContext.innerscene.cors.passesAccessControlCheck(rawHeaders, withCredentials, toOrigin)) {
    var message = "CORS block for: '" + toOrigin + "' from '" + fromOrigin + "'";
    log.warn(message);
    this.blocked = isBlocked = true;

    // destroy the response
    if (typeof httpResponse.end === 'function') {
      httpResponse.end();
    } else if (typeof httpResponse.destroy === 'function') {
      httpResponse.destroy(new Error(message));
    }
  } else {
    log.message(4, "CORS passed for: '" + toOrigin + "' from '" + fromOrigin + "'");
  }

  if (!isBlocked) {
    var self = this;
    httpResponse.on('data', function (data) {
      self.emit('data', data);
    });
    httpResponse.once('error', function (e) {
      self.emit('error', e);

      // clean up
      self.removeAllListeners();
      httpResponse.removeAllListeners();
    });
    httpResponse.once('end', function () {
      self.emit('end');

      // clean up
      self.removeAllListeners();
      httpResponse.removeAllListeners();
    });
  }

  this.headers = httpResponse.headers;
  this.httpVersion = httpResponse.httpVersion;
  this.method = httpResponse.method;
  this.rawHeaders = httpResponse.rawHeaders;
  this.rawTrailers = httpResponse.rawTrailers;
  this.statusCode = httpResponse.statusCode;
  this.statusMessage = httpResponse.statusMessage;
  this.trailers = httpResponse.trailers;
  this.url = httpResponse.url;

  this.setEncoding = function () {
    if (isV8) {
      log.warn("setEncoding not implemented for v8");
    } else {
      httpResponse.setEncoding.apply(httpResponse, arguments);
    }
  };
}

Response.prototype = Object.create(EventEmitter.prototype);
Response.prototype.constructor = Response;
Response.prototype.blocked = false;

function Utils() {
}

Utils._normalizeOptions = function (options) {
  if (typeof options === 'string') {
    options = url.parse(options);
  } else {
    options = Utils._extend({}, options);
  }
  return options;
};

Utils._packHeaders = function (headers) {
  var rawHeaders = "";
  Object.keys(headers).forEach(function (t) {
    rawHeaders += "\r\n" + t + ": " + headers[t];
  });
  return rawHeaders.slice(2);
};

Utils._getRequestOrigin = function (options, defaultProtocol) {
  var protocol = Utils._getRequestScheme(options, defaultProtocol);
  var host = options.host || options.hostname || 'localhost';
  var result = protocol + "://" + host;
  if (options.port) {
    var portPart = ':' + options.port;
    if (result.substr(-portPart.length) !== portPart) {
      result += portPart;
    }
  }
  return result;
};

Utils._extend = function (target, source) {
  if (source === null || typeof source !== 'object') return target;
  var keys = Object.keys(source);
  var i = keys.length;
  while (i--) {
    target[keys[i]] = source[keys[i]];
  }
  return target;
};

Utils._assert = function (condition, message) {
  if (!condition) {
    log.warn(message);
    throw new Error(message);
  }
};

Utils._getRequestScheme = function (options, defaultProtocol) {
  var scheme = options.protocol ? options.protocol : defaultProtocol;
  var pos = scheme.indexOf(':');
  if (pos !== -1) {
    scheme = scheme.substring(0, scheme.indexOf(':'));
  }
  return scheme.toLowerCase();
};
