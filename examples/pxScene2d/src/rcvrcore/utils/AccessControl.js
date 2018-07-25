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

var http = require('http');
var url = require('url');
var events = require('events');
var WrapObj = require('rcvrcore/utils/WrapObj');
var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('AccessControl');

/**
 * Public wrapper around scene's access control functionality.
 *
 * @param scene
 * @constructor
 */
function AccessControl(scene) {
  log.message(4, "create");
  this.scene = scene;
}

AccessControl.prototype.destroy = function () {
  log.message(4, "destroy");
  this.scene = null;
};

AccessControl.prototype.origin = function () {
  if (this.scene) {
    return this.scene.origin;
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

AccessControl.prototype.passesAccessControlCheck = function (rawHeaders, withCredentials, origin) {
  if (this.scene) {
    var cors = this.scene.cors;
    if (cors) {
      return cors.passesAccessControlCheck(rawHeaders, withCredentials, origin);
    }
  }
  return true;
};

AccessControl.isCORSRequestHeader = function (name) {
  if (name) {
    if (name.match(/^(Origin|Access-Control-Request-Method|Access-Control-Request-Headers)$/ig)) {
      return true;
    }
  }
  return false;
};

AccessControl.prototype.createClientRequest = function (options, callback, requestOrigin) {
  var req = AccessControlClientRequest(options, callback, this, requestOrigin);

  // In order to prevent hacks do not expose the AccessControlClientRequest.
  // Exposed object is an external EventEmitter + selected APIs
  return WrapObj(req, WrapObj(req._externalEvents), true, [
    'blocked','abort','aborted','end','getHeader','maxHeadersCount','removeHeader','setHeader','setNoDelay','setSocketKeepAlive','setTimeout','write'
  ]);
};

AccessControl._extend = function (target, source) {
  if (source === null || typeof source !== 'object') return target;
  var keys = Object.keys(source);
  var i = keys.length;
  while (i--) {
    target[keys[i]] = source[keys[i]];
  }
  return target;
};

AccessControl._getRequestOrigin = function (options, protocol) {
  var optionsCopy = AccessControl._extend({}, typeof options === 'string' ? url.parse(options) : options);
  delete optionsCopy.headers;
  var testReq = new http.ClientRequest(optionsCopy);
  var host = testReq.getHeader('host');
  testReq.abort();
  return host ? protocol + host : null;
};

/**
 * Private class AccessControlClientRequest, inherits from http.ClientRequest.
 * Applies CORS and permissions.
 *
 * @param options
 * @param callback
 * @param accessControl
 * @param protocol
 * @constructor
 */
function AccessControlClientRequest(options, callback, accessControl, protocol) {
  if (!(this instanceof AccessControlClientRequest))
    return new AccessControlClientRequest(options, callback, accessControl, protocol);

  this._externalEvents = new events();
  this._externalEvents.once('response', callback);

  http.ClientRequest.call(this, options);

  // Internal listener. If no 'error' handler is added, then 'uncaught exception' is thrown.
  this.on('error', function (e) {
    log.message(4, "internal error handler fired: " + e);
  });

  // Internal listener. If no 'response' handler is added, then the response will be entirely discarded.
  this.on('response', function (r) {
    log.message(4, "internal response handler fired: HTTP " + r.statusCode);
  });

  var _this = this;
  var appOrigin = accessControl.origin();
  var requestOrigin = AccessControl._getRequestOrigin(options, protocol);
  if (!accessControl.allows(requestOrigin)) {
    var message = "Permissions block for request to origin: '" + requestOrigin + "' from origin '" + appOrigin + "'";
    log.warn(message);
    this.blocked = true;
    this.abort();
    setTimeout(function () {
      log.message(4, "about to emit error/blocked");
      _this.emit('error', new Error(message));
      _this.emit('blocked', new Error(message));
    });
  }

  if (appOrigin) {
    log.message(2, "for request to origin: '" + requestOrigin + "' set origin '" + appOrigin + "'");
    http.ClientRequest.prototype.setHeader.call(this, "Origin", appOrigin);
  }

  function checkResponseHeaders(res) {
    var rawHeaders = "";
    for (var key in res.headers) {
      if (res.headers.hasOwnProperty(key)) {
        rawHeaders += (rawHeaders ? "\r\n" : "") + key + ": " + res.headers[key];
      }
    }
    log.message(4, "check for request to origin: '" + requestOrigin + "' from origin '" + appOrigin + "' headers: " + rawHeaders);
    if (!accessControl.passesAccessControlCheck(rawHeaders, false, requestOrigin)) {
      var message = "CORS block for request to origin: '" + requestOrigin + "' from origin '" + appOrigin + "'";
      log.warn(message);
      _this.blocked = true;
      res.destroy(new Error(message));
      _this.emit('blocked', new Error(message));
      return false;
    }
    return true;
  }

  // Events handled by _externalEvents event handler hide socket from argument lists
  function wrapExternalArgs(type) {
    var args = Array.prototype.slice.call(arguments);
    if (type === 'socket') {
      args[1] = null; // socket obj
    } else if (type === 'connect' || type === 'upgrade') {
      args[1] = WrapObj(args[1], {socket: undefined}, true); // response obj
      args[2] = null; // socket obj
    } else if (type === 'information' || type === 'response') {
      args[1] = WrapObj(args[1], {socket: undefined}, true); // response obj
    }
    return args;
  }

  function createEmitWrapper(_emit) {
    return function (type, arg2) {
      log.message(4, "event is about to emit: "+type);
      if (typeof type === 'string' && type.match(/^(connect|upgrade|information|response)$/ig)) {
        if (checkResponseHeaders(arg2) === false) {
          return false;
        }
      }
      log.message(4, type+": external listeners count: "+_this._externalEvents.listenerCount(type));
      var externalArgs = wrapExternalArgs.apply(null, arguments);
      _this._externalEvents.emit.apply(_this._externalEvents, externalArgs);
      log.message(4, type+": internal listeners count: "+_this.listenerCount(type));
      return _emit.apply(_this, arguments);
    };
  }

  var _emit = this.emit;
  var _emit2 = this.$emit;
  this.emit = createEmitWrapper(_emit);
  this.$emit = createEmitWrapper(_emit2);

  log.message(4, "created a request to origin: '" + requestOrigin + "'");
}

AccessControlClientRequest.prototype = Object.create(http.ClientRequest.prototype);
AccessControlClientRequest.prototype.constructor = AccessControlClientRequest;

AccessControlClientRequest.prototype.blocked = undefined;
AccessControlClientRequest.prototype._externalEvents = undefined;

AccessControlClientRequest.prototype.setHeader = function (name) {
  if (AccessControl.isCORSRequestHeader(name)) {
    var message = "not allowed to set header '" + name + "'";
    log.warn(message);
    throw new Error(message);
  }
  return http.ClientRequest.prototype.setHeader.apply(this, arguments);
};

AccessControlClientRequest.prototype.removeHeader = function (name) {
  if (AccessControl.isCORSRequestHeader(name)) {
    var message = "not allowed to remove header '" + name + "'";
    log.warn(message);
    throw new Error(message);
  }
  return http.ClientRequest.prototype.removeHeader.apply(this, arguments);
};

module.exports = AccessControl;
