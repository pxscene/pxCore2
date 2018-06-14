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

AccessControl.prototype.checkAccessControlHeaders = function (url, rawHeaders) {
  if (this.scene) {
    return this.scene.checkAccessControlHeaders(url, rawHeaders);
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
  // Do not expose AccessControlClientRequest's prototype... wrap everything
  // Note: set properties in constructor or prototype so that they don't get lost here
  // Note: socket is not exposed
  var wrap = WrapObj(req, {socket: undefined, connection: undefined});
  return wrap;
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

  http.ClientRequest.call(this, options, callback);

  var _this = this;
  var requestOrigin = AccessControl._getRequestOrigin(options, protocol);
  if (!accessControl.allows(requestOrigin)) {
    var message = "Permissions block for request to origin: '" + requestOrigin + "' from origin '" + accessControl.origin() + "'";
    log.warn(message);
    this.blocked = true;
    this.abort();
    setTimeout(function () {
      _this.emit('blocked', new Error(message));
    });
  }

  if (accessControl.origin()) {
    log.message(2, "for request to origin: '" + requestOrigin + "' set origin '" + accessControl.origin() + "'");
    http.ClientRequest.prototype.setHeader.call(this, "Origin", accessControl.origin());
  }

  function checkResponseHeaders(res) {
    var rawHeaders = "";
    for (var key in res.headers) {
      if (res.headers.hasOwnProperty(key)) {
        rawHeaders += (rawHeaders ? "\r\n" : "") + key + ": " + res.headers[key];
      }
    }
    log.message(4, "check for request to origin: '" + requestOrigin + "' from origin '" + accessControl.origin() + "' headers: " + rawHeaders);
    if (!accessControl.checkAccessControlHeaders(requestOrigin, rawHeaders)) {
      var message = "CORS block for request to origin: '" + requestOrigin + "' from origin '" + accessControl.origin() + "'";
      log.warn(message);
      _this.blocked = true;
      res.destroy(new Error(message));
      _this.emit('blocked', new Error(message));
      return false;
    }
    return true;
  }

  function createEmitWrapper(_emit) {
    return function (type, arg2, arg3, arg4) {
      if (type === 'socket') {
        // Note: socket is not exposed
        return _emit.call(_this, type, null);
      } else if (type === 'connect' || type === 'upgrade') {
        if (checkResponseHeaders(arg2) === false) {
          return false;
        }
        // Note: socket is not exposed
        return _emit.call(_this, type, WrapObj(arg2, {socket: undefined}), null, arg4);
      } else if (type === 'information' || type === 'response') {
        if (checkResponseHeaders(arg2) === false) {
          return false;
        }
        // Note: socket is not exposed
        return _emit.call(_this, type, WrapObj(arg2, {socket: undefined}));
      }
      return _emit.apply(_this, arguments);
    };
  }

  _this.emit = createEmitWrapper(_this.emit);
  _this.$emit = createEmitWrapper(_this.$emit);

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

  log.message(4, "created a request to origin: '" + requestOrigin + "'");
}

AccessControlClientRequest.prototype = Object.create(http.ClientRequest.prototype);
AccessControlClientRequest.prototype.constructor = AccessControlClientRequest;

AccessControlClientRequest.prototype.blocked = undefined;

module.exports = AccessControl;
