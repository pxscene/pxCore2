'use strict';

var http = require('../http.js');

function isLocalAccess(reqOptions)
{
  if (((reqOptions.hostname) && ((reqOptions.hostname === "localhost") || (reqOptions.hostname === "127.0.0.1"))) || ((reqOptions.host) && ((reqOptions.host === "localhost") || (reqOptions.host === "127.0.0.1"))))
  {
    return true;
  }
  else if (((reqOptions.hostname) && ((reqOptions.hostname === "[::1]") || (reqOptions.hostname === "[0:0:0:0:0:0:0:1]"))) || ((reqOptions.host) && ((reqOptions.host === "[::1]") || (reqOptions.host === "[0:0:0:0:0:0:0:1]"))))
  {
    return true;
  }
  else if (((reqOptions.hostname) && ((reqOptions.hostname === "::1") || (reqOptions.hostname === "0:0:0:0:0:0:0:1"))) || ((reqOptions.host) && ((reqOptions.host === "::1") || (reqOptions.host === "0:0:0:0:0:0:0:1"))))
  {
    return true;
  }
  return false;
}

function HttpWrap()
{
  this.localApp = false;
}

HttpWrap.prototype.setLocalApp = function(isLocalApp) {
  this.localApp = isLocalApp;
};

HttpWrap.prototype.getLocalApp = function() {
  return this.localApp;
};

HttpWrap.prototype.get = function(options, cb) {
  if (true == isLocalAccess(options))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return http.get(options, cb);
};

HttpWrap.prototype.request = HttpWrap.prototype.get;

module.exports = HttpWrap;
