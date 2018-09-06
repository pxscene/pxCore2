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

var http2 = require('http2');
var http = require('http');
var url = require('url');
var WrapObj = require('rcvrcore/utils/WrapObj');
var Logger = require('rcvrcore/Logger').Logger;

var LogLevel = Object.freeze({
  info: 2, // Log always
  debug: 4, // Debugging
  trace: 100 // Log everything
});

/**
 * Exported class. Wraps scene, exposes access control functionality.
 * @param scene
 * @constructor
 */
function AccessControl(scene) {
  this.log = new Logger('AccessControl');
  this.log.message(LogLevel.debug, "create");
  this.scene = scene;
  this.wraps = [];
}

AccessControl.prototype.destroy = function () {
  this.log.message(LogLevel.debug, "destroy");
  this.scene = null;
  this.wraps.forEach(function (t) {
    t.removeAllListeners();
  });
  this.wraps = null;
};

AccessControl.prototype.origin = function () {
  if (this.scene) {
    var cors = this.scene.cors;
    if (cors) {
      return cors.origin;
    }
  }
  return null;
};

AccessControl.prototype.allows = function (url) {
  if (this.scene) {
    var permissions = this.scene.permissions;
    if (permissions) {
      return permissions.allows(url);
    }
  }
  return true;
};

AccessControl.prototype.passesAccessControlCheck = function (rawHeaders, withCredentials, toOrigin) {
  if (this.scene) {
    var cors = this.scene.cors;
    if (cors) {
      return cors.passesAccessControlCheck(rawHeaders, withCredentials, toOrigin);
    }
  }
  return true;
};

AccessControl.prototype.isCORSRequestHeader = function (headerName) {
  if (this.scene) {
    var cors = this.scene.cors;
    if (cors) {
      return cors.isCORSRequestHeader(headerName);
    }
  }
  return false;
};

AccessControl.prototype.isCredentialsRequestHeader = function (headerName) {
  if (this.scene) {
    var cors = this.scene.cors;
    if (cors) {
      return cors.isCredentialsRequestHeader(headerName);
    }
  }
  return false;
};

module.exports.AccessControl = AccessControl;

/**
 * Exported function. Returns an http request object, similar to what http2.request or http.request return.
 * @param accessControl
 * @param options
 * @param callback
 * @param defaultToHttp1
 * @returns {*}
 */
module.exports.request = function (accessControl, options, callback, defaultToHttp1) {
  return new _RequestWrapper(accessControl, options, defaultToHttp1).request(callback);
};

/**
 * A context object which creates a request.
 * @param accessControl
 * @param options
 * @param defaultToHttp1
 * @constructor
 * @private
 */
function _RequestWrapper(accessControl, options, defaultToHttp1) {
  this.log = new Logger('RequestWrapper');
  this.accessControl = accessControl;
  this.defaultToHttp1 = defaultToHttp1;
  this.fromOrigin = this.accessControl ? this.accessControl.origin() : null;
  this.options = this.normalizeOptions(options);
  this.toOrigin = Utils._getRequestOrigin(this.options, this.defaultToHttp1);
  this.scheme = Utils._getRequestScheme(this.options, this.defaultToHttp1);
  this.withCredentials = this.isWithCredentials();
  this.block = this.accessControl ? !this.accessControl.allows(this.toOrigin) : false;
  var message = "created. block: " + this.block;
  message += ", to origin: '" + this.toOrigin + "', from origin '" + this.fromOrigin + "'";
  message += ", withCredentials: " + this.withCredentials;
  message += ", defaultToHttp1: " + this.defaultToHttp1;
  message += ", scheme: " + this.scheme;
  this.log.message(LogLevel.debug, message);
}

/**
 * Determines whether or not request options contain credentials.
 * @returns {boolean}
 */
_RequestWrapper.prototype.isWithCredentials = function () {
  var result = false;
  if (this.options && this.options.headers && this.accessControl) {
    var h = this.options.headers;
    var _this = this;
    Object.keys(h).forEach(function (k) {
      if (_this.accessControl.isCredentialsRequestHeader(k)) {
        _this.log.message(LogLevel.info, "is credentials header: '" + k + "'=" + h[k]);
        result = true;
      }
    });
  }
  return result;
};

/**
 * Creates an options object from input, takes care of required headers (for example CORS).
 * @param options
 * @returns {*}
 */
_RequestWrapper.prototype.normalizeOptions = function (options) {
  if (typeof options === 'string') {
    this.log.message(LogLevel.trace, "options is URL: " + options);
    options = url.parse(options);
  } else {
    this.log.message(LogLevel.trace, "options is object");
    options = Utils._extend({}, options);
  }
  if (this.fromOrigin && this.accessControl) {
    var h = options.headers;
    if (!h) {
      h = options.headers = {};
    }
    var _this = this;
    Object.keys(h).forEach(function (k) {
      if (_this.accessControl.isCORSRequestHeader(k)) {
        _this.log.message(LogLevel.info, "removing header: '" + k + "'=" + h[k]);
        delete h[k];
      }
    });
    this.log.message(LogLevel.info, "set header: 'Origin'=" + this.fromOrigin + "'");
    h.Origin = this.fromOrigin;
  }
  return options;
};

/**
 * Creates a request. Depending on scheme request can be based either on http2 or http.
 * @param callback
 */
_RequestWrapper.prototype.request = function (callback) {
  var ret;
  if (this.block) {
    this.log.message(LogLevel.trace, "create blocked request");
    this.req = new http2.ClientRequest();
    this.block_Http2_OutgoingRequest(this.req);
    ret = this.wrap(this.req);
  } else if (this.scheme === 'http') {
    if (this.accessControl) {
      this.log.message(LogLevel.trace, "create wrapped http request");
      this.req = http.request(this.options);
      ret = this.wrap(this.req);
      if (callback) {
        ret.on('response', callback);
      }
    } else {
      this.log.message(LogLevel.trace, "create http request");
      this.req = http.request(this.options, callback);
      ret = this.req;
    }
  } else {
    if (this.accessControl) {
      this.log.message(LogLevel.trace, "create wrapped http2 request");
      this.req = http2.request(this.options);
      ret = this.wrap(this.req);
      if (callback) {
        ret.on('response', callback);
      }
    } else {
      this.log.message(LogLevel.trace, "create http2 request");
      this.req = http2.request(this.options, callback);
      ret = this.req;
    }
  }
  return ret;
};

/**
 * Destroys http2.OutgoingRequest so that it cannot be used afterwards.
 */
_RequestWrapper.prototype.block_Http2_OutgoingRequest = function (request) {
  Utils._assert(request instanceof http2.OutgoingRequest, "wrong class");
  var message = "Permissions block for request";
  message += " to origin: '" + this.toOrigin + "' from origin '" + this.fromOrigin + "'";
  this.log.warn(message);
  request.blocked = true;
  this.log.message(LogLevel.trace, "about to abort request");
  request.abort();
  var self = this;
  setTimeout(function () {
    self.log.message(LogLevel.trace, "about to emit 'blocked'");
    request.emit('blocked', new Error(message));
  });
};

/**
 * Updates http2.IncomingResponse according to AccessControl.
 * @param response
 */
_RequestWrapper.prototype.update_Http2_IncomingResponse = function (response) {
  Utils._assert(response instanceof http2.IncomingResponse, "wrong class");
  var rawHeaders = Utils._packHeaders(response.headers);
  this.log.message(LogLevel.debug, "response headers: " + rawHeaders);
  if (this.accessControl) {
    if (!this.accessControl.passesAccessControlCheck(rawHeaders, this.withCredentials, this.toOrigin)) {
      this.block_Http2_IncomingResponse(response);
    } else {
      this.log.message(LogLevel.trace, "CORS passed: '" + this.toOrigin + "' from '" + this.fromOrigin);
    }
  }
};

/**
 * Destroys http2.IncomingResponse so that it cannot be used afterwards.
 * @param response
 */
_RequestWrapper.prototype.block_Http2_IncomingResponse = function (response) {
  Utils._assert(response instanceof http2.IncomingResponse, "wrong class");
  var message = "CORS block for request";
  message += " to origin: '" + this.toOrigin + "' from origin '" + this.fromOrigin + "'";
  this.log.warn(message);
  this.req.blocked = true;
  this.log.message(LogLevel.trace, "about to end response");
  response.end();
  this.log.message(LogLevel.trace, "about to abort request");
  this.req.abort();
  var self = this;
  setTimeout(function () {
    self.log.message(LogLevel.trace, "about to emit 'blocked'");
    self.req.emit('blocked', new Error(message));
  });
};

/**
 * Updates http.IncomingMessage according to AccessControl.
 * @param response
 */
_RequestWrapper.prototype.update_Http_IncomingMessage = function (response) {
  Utils._assert(response instanceof http.IncomingMessage, "wrong class");
  var rawHeaders = Utils._packHeaders(response.headers);
  this.log.message(LogLevel.debug, "response headers: " + rawHeaders);
  if (this.accessControl) {
    if (!this.accessControl.passesAccessControlCheck(rawHeaders, this.withCredentials, this.toOrigin)) {
      this.block_Http_IncomingMessage(response);
    } else {
      this.log.message(LogLevel.trace, "CORS passed: '" + this.toOrigin + "' from '" + this.fromOrigin);
    }
  }
};

/**
 * Destroys http.IncomingMessage so that it cannot be used afterwards.
 * @param response
 */
_RequestWrapper.prototype.block_Http_IncomingMessage = function (response) {
  Utils._assert(response instanceof http.IncomingMessage, "wrong class");
  var message = "CORS block for request";
  message += " to origin: '" + this.toOrigin + "' from origin '" + this.fromOrigin + "'";
  this.log.warn(message);
  this.req.blocked = true;
  this.log.message(LogLevel.trace, "about to destroy response");
  response.destroy(new Error(message));
  this.log.message(LogLevel.trace, "about to abort request");
  this.req.abort();
  var self = this;
  setTimeout(function () {
    self.log.message(LogLevel.trace, "about to emit 'blocked'");
    self.req.emit('blocked', new Error(message));
  });
};

/**
 * Creates a wrapper around an object which is expected to be exposed to the app.
 * @param o - object to wrap
 * @returns {*} - wrapped {@see o}
 */
_RequestWrapper.prototype.wrap = function (o) {
  var cl = Utils._getClassName(o);
  if (o instanceof http2.IncomingResponse) {
    this.log.message(LogLevel.trace, "wrap '" + cl + "'");
    o = this.wrap_http2_IncomingResponse(o);
    this.accessControl.wraps.push(o);
  } else if (cl === 'IncomingPromise') {
    this.log.message(LogLevel.trace, "wrap '" + cl + "'");
    o = this.wrap_http2_IncomingPromise(o);
    this.accessControl.wraps.push(o);
  } else if (o instanceof http2.OutgoingRequest) {
    this.log.message(LogLevel.trace, "wrap '" + cl + "'");
    o = this.wrap_http2_OutgoingRequest(o);
    this.accessControl.wraps.push(o);
  } else if (cl === 'Stream') {
    this.log.message(LogLevel.trace, "wrap '" + cl + "'");
    o = null;
  } else if (o instanceof http.ClientRequest) {
    this.log.message(LogLevel.trace, "wrap '" + cl + "'");
    o = this.wrap_http_ClientRequest(o);
    this.accessControl.wraps.push(o);
  } else if (o instanceof http.IncomingMessage) {
    this.log.message(LogLevel.trace, "wrap '" + cl + "'");
    o = this.wrap_http_IncomingMessage(o);
    this.accessControl.wraps.push(o);
  } else {
    this.log.message(LogLevel.trace, "skip wrap for '" + cl + "'");
  }
  return o;
};

_RequestWrapper.prototype.wrap_http2_IncomingResponse = function (o) {
  Utils._assert(o instanceof http2.IncomingResponse, "wrong class");
  // http2.IncomingResponse
  // - props: statusCode
  // - functions: _onHeaders
  // - events: 'ready'
  // http2.IncomingMessage
  // - props: socket, stream, _log, httpVersion, httpVersionMajor, httpVersionMinor, headers, trailers, _lastHeadersSeen
  // - functions: _onHeaders, _onEnd, setTimeout, _checkSpecialHeader, _validateHeaders 
  // stream.PassThrough
  // stream.Transform
  // - functions: destroy
  // - events: 'error', 'close'
  // stream.Duplex
  // stream.Writable
  // - props: writableHighWaterMark, writableLength
  // - functions: cork, destroy, end, setDefaultEncoding, uncork, write
  // - events: 'close', 'drain', 'error', 'finish', 'pipe(stream.Readable)', 'unpipe(stream.Readable)'
  // stream.Duplex
  // stream.Readable
  // - props: readableHighWaterMark, readableLength
  // - functions: destroy, isPaused, pause, pipe, read, resume, setEncoding, unpipe, unshift, wrap
  // - events: 'close', 'data', 'end', 'error', 'readable'
  this.update_Http2_IncomingResponse(o);
  return WrapObj(
    o, null,
    o, [
      'statusCode',
      'httpVersion', 'httpVersionMajor', 'httpVersionMinor', 'headers', 'trailers', 'setTimeout',
      'destroy',
      'writableHighWaterMark', 'writableLength', 'cork', 'end', 'setDefaultEncoding', 'uncork', 'write',
      'readableHighWaterMark', 'readableLength', 'isPaused', 'pause', 'read', 'resume', 'setEncoding', 'unshift'
    ], [
      'ready',
      'error', 'close',
      'drain', 'finish',
      'data', 'end'
    ],
    this.wrap.bind(this)
  );
};

_RequestWrapper.prototype.wrap_http2_IncomingPromise = function (o) {
  Utils._assert(o instanceof http2.IncomingPromise, "wrong class");
  // http2.IncomingPromise
  // - props: _responseStream, stream
  // - functions: _onHeaders, cancel, setPriority, _onPromise
  // - events: 'response(http2.IncomingResponse)'
  // http2.IncomingRequest
  // - props: method, scheme, host, url
  // - functions: _onHeaders
  // - events: 'ready'
  // http2.IncomingMessage
  // ...
  return WrapObj(
    o, null,
    o, [
      'cancel', 'setPriority',
      'method', 'scheme', 'host', 'url',
      'httpVersion', 'httpVersionMajor', 'httpVersionMinor', 'headers', 'trailers', 'setTimeout',
      'destroy',
      'writableHighWaterMark', 'writableLength', 'cork', 'end', 'setDefaultEncoding', 'uncork', 'write',
      'readableHighWaterMark', 'readableLength', 'isPaused', 'pause', 'read', 'resume', 'setEncoding', 'unshift'
    ], [
      'response',
      'ready',
      'error', 'close',
      'drain', 'finish',
      'data', 'end'
    ],
    this.wrap.bind(this)
  );
};

_RequestWrapper.prototype.wrap_http2_OutgoingRequest = function (o) {
  Utils._assert(o instanceof http2.OutgoingRequest, "wrong class");
  // http2.OutgoingRequest
  // - props: _log, stream, options, headersSent
  // - functions: _start, _fallback, setPriority, on, setNoDelay, setSocketKeepAlive, setTimeout, abort, _onPromise
  // - events: 'socket(http2.Stream)', 'response(http2.IncomingResponse)', 'push(http2.IncomingPromise)'
  // http2.OutgoingMessage
  // - props: _headers, _trailers, headersSent, finished
  // - functions: _write, _finish, setHeader, removeHeader, getHeader, addTrailers, setTimeout, _checkSpecialHeader
  // - events: 'error'
  // stream.Writable
  // ...
  return WrapObj(
    o, null,
    o, [
      'blocked',
      'headersSent', 'setPriority', 'setNoDelay', 'setSocketKeepAlive', 'setTimeout', 'abort',
      'finished', 'getHeader', 'addTrailers',
      'writableHighWaterMark', 'writableLength',
      'cork', 'destroy', 'end', 'setDefaultEncoding', 'uncork', 'write'
    ], [
      'blocked',
      'response', 'push',
      'error',
      'close', 'drain', 'finish'
    ],
    this.wrap.bind(this)
  );
};

_RequestWrapper.prototype.wrap_http_ClientRequest = function (o) {
  Utils._assert(o instanceof http.ClientRequest, "wrong class");
  // http.ClientRequest
  // - props: aborted, connection, maxHeadersCount, socket
  // - functions: abort, end, flushHeaders, getHeader, removeHeader, setHeader, setNoDelay, setSocketKeepAlive, setTimeout, write
  // - events: 'abort', 'connect', 'continue', 'information', 'response', 'socket', 'timeout', 'upgrade'
  // stream.Writable
  // ...
  return WrapObj(
    o, null,
    o, [
      'blocked',
      'aborted', 'maxHeadersCount', 'abort', 'getHeader', 'setNoDelay', 'setSocketKeepAlive', 'setTimeout',
      'writableHighWaterMark', 'writableLength',
      'cork', 'destroy', 'end', 'setDefaultEncoding', 'uncork', 'write'
    ], [
      'blocked',
      'abort', 'response', 'timeout',
      'error',
      'close', 'drain', 'finish'
    ],
    this.wrap.bind(this)
  );
};

_RequestWrapper.prototype.wrap_http_IncomingMessage = function (o) {
  Utils._assert(o instanceof http.IncomingMessage, "wrong class");
  // http.IncomingMessage
  // - props: aborted, headers, httpVersion, method, rawHeaders, rawTrailers, socket, statusCode, statusMessage, trailers, url
  // - functions: destroy, setTimeout
  // - events: 'aborted', 'close'
  // stream.Readable
  // ...
  this.update_Http_IncomingMessage(o);
  return WrapObj(
    o, null,
    o, [
      'aborted', 'headers', 'httpVersion', 'method', 'rawHeaders', 'rawTrailers', 'statusCode', 'statusMessage', 'trailers', 'url',
      'destroy', 'setTimeout',
      'readableHighWaterMark', 'readableLength', 'isPaused', 'pause', 'read', 'resume', 'setEncoding', 'unshift'
    ], [
      'aborted', 'close',
      'close', 'data', 'end', 'error'
    ],
    this.wrap.bind(this)
  );
};

function Utils() {
}

Utils._packHeaders = function (headers) {
  var rawHeaders = "";
  Object.keys(headers).forEach(function (t) {
    rawHeaders += "\r\n" + t + ": " + headers[t];
  });
  return rawHeaders.slice(2);
};

Utils._getRequestOrigin = function (options, defaultToHttp1) {
  var protocol = Utils._getRequestScheme(options, defaultToHttp1);
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

Utils._getRequestScheme = function (options, defaultToHttp1) {
  var scheme = options.protocol;
  if (!scheme) {
    scheme = defaultToHttp1 ? 'http' : 'https';
  }
  var pos = scheme.indexOf(':');
  if (pos !== -1) {
    scheme = scheme.substring(0, scheme.indexOf(':'));
  }
  return scheme.toLowerCase();
};

Utils._getClassName = function (o) {
  return o && o.constructor ? o.constructor.name : typeof o;
};
