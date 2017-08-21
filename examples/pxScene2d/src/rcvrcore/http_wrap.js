'use strict';

var http = require('http');

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
  return false;
}

function HttpWrap()
{
  this.localApp = false;
}

HttpWrap.prototype.IncomingMessage = http.IncomingMessage;
HttpWrap.prototype.METHODS = http.METHODS;
HttpWrap.prototype.OutgoingMessage = http.OutgoingMessage;

HttpWrap.prototype.setLocalApp = function(isLocalApp) {
  this.localApp = isLocalApp;
};

HttpWrap.prototype.getLocalApp = function() {
  return this.localApp;
};

HttpWrap.prototype.request = function(options, cb) {
  if (true == isLocalAccess(options))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return http.request(options, cb);
};

HttpWrap.prototype.ClientRequest = function(options, cb) {
  if (true == isLocalAccess(options))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return http.ClientRequest(options, cb);
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

HttpWrap.prototype.Agent = function(options) {
  if (true == isLocalAccess(options))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return http.Agent(options);
};

HttpWrap.prototype.globalAgent = function(options) {
  if (true == isLocalAccess(options))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return http.globalAgent(options);
};

HttpWrap.prototype.Client = function(port, host) {
  if ((host === "localhost") || (host === "127.0.0.1"))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return http.Client(port,host);
};

HttpWrap.prototype.createClient = function(port, host) {
  if ((host === "localhost") || (host === "127.0.0.1"))
  {
    if (false == this.localApp)
    {
      console.log("localhost urls cannot be accessed by remote applications");
      return;
    }
  }
  return http.createClient(port,host);
};

module.exports = HttpWrap;
