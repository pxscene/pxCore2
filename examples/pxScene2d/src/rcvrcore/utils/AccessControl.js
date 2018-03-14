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

var url = require('url');

function AccessControl(scene) {
  this.scene = scene;
}

AccessControl.prototype.destroy = function () {
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
    return this.scene.allows(url);
  }
  return true;
};

AccessControl.prototype.checkAccessControlHeaders = function (url, rawHeaders) {
  if (this.scene) {
    return this.scene.checkAccessControlHeaders(url, rawHeaders);
  }
  return true;
};

AccessControl.prototype.wrapArgs = function (options, cb, secure) {
  var self = this;
  if (typeof options === 'string') {
    options = url.parse(options);
    if (!options.hostname) {
      throw new Error('Unable to determine the domain name');
    }
  } else {
    options = util._extend({}, options);
  }

  // 1. add origin header (CORS)
  var sceneOrigin = self.origin();
  if (sceneOrigin) {
    if (!options.headers) {
      options.headers = {};
    }
    options.headers["Origin"] = sceneOrigin;
  }

  // 2. check if host is permitted (permissions)
  var protocol = options.protocol || (secure ? "https:" : "http:");
  var port = options.port ? ":" + options.port : "";
  var host = options.host || options.hostname || 'localhost';
  if (host && port && host.indexOf(port, host.length - port.length) !== -1) {
    port = "";
  }
  var reqOrigin = protocol + (protocol.indexOf("//") > 0 ? "" : "//") + host + port;
  if (!self.allows(reqOrigin)) {
    return null;
  }

  // 3. check response headers (CORS)
  if (cb) {
    var _originalCb = cb;
    cb = function (response) {
      var rawHeaders = "";
      for (var key in response.headers) {
        if (response.headers.hasOwnProperty(key)) {
          rawHeaders += (rawHeaders ? "\r\n" : "") + key + ": " + response.headers[key];
        }
      }
      if (!self.checkAccessControlHeaders(reqOrigin, rawHeaders)) {
        response.destroy("CORS block");
      } else if (_originalCb) {
        _originalCb(response);
      }
    };
  }

  // 4. return modified args
  return cb ? [options, cb] : [options];
};

module.exports = AccessControl;
