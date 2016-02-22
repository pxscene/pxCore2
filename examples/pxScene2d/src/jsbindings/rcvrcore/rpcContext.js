/**
 * Created by tjcarroll2
 * on 2/6/16.
 */

function RPCContext(theScene) {
  var scene = theScene;
  var rpcController = null;
  var sourceTargetName;
  var appNameTargetRegistered = false;
  var functions = {};
  var onRPCCall;

  this.registerApp = function(targetName) {
    if( rpcController !== null && !appNameTargetRegistered ) {
      appNameTargetRegistered = rpcController.registerApp(targetName, scene);
      sourceTargetName = targetName;
    } else {
      console.error("RPC registerApp: No rpcController exists or target app is already registered for " + targetName + ", rpcController=" + rpcController);
    }

    return appNameTargetRegistered;
  };

  this.onRPCCall = function(callback) {
    onRPCCall = callback;
  };

  this.registerFunction = function(functionName, domainFiltersArray) {
    if( rpcController !== null || !appNameTargetRegistered ) {
      functions[functionName] = domainFiltersArray;
      return true;
    } else {
      console.error("No rpcController exists or target  for registering function " + appNameTargetRegistered);
    }

    return false;
  };

  this.execute = function(remoteTargetName, functionName, args, callback) {
    if( rpcController !== null ) {
      rpcController.execute(sourceTargetName, remoteTargetName, functionName, args, callback);
    } else {
      console.error("No rpcController for executing RPC function " + remoteTargetName + ":" + functionName);
    }
  };

  this._execute = function(sourceTarget, remoteTargetName, functionName, args) {
    if( !functions.hasOwnProperty(functionName) ) {
      console.error("RPC execute: function name<" + functionName + "> doesn't exist in target <" + remoteTargetName + ">");
      return false;
    }

    if( !isSourceDomainAllowed(sourceTarget, functions[functionName]) ) {
      console.error("RPC execute: function name<" + functionName + "> doesn't allow calls from sourceTarget domain <" + sourceTarget + ">");
      return false;
    }

    return onRPCCall(functionName, args);

  };

  function isSourceDomainAllowed(sourceTarget, domainFiltersArray) {
    if( domainFiltersArray === undefined || domainFiltersArray === null || domainFiltersArray.length == 0
      || (domainFiltersArray.length === 1 && domainFiltersArray[0][1] === "*") ) {
      return true;
    }

    var lastDotIndex = sourceTarget.lastIndexOf('.');
    if( lastDotIndex === -1 ) {
      // default packages aren't allowed
      return false;
    }

    // include the dot
    var baseDomain = sourceTarget.substring(0, lastDotIndex+1);

    for (var index = 0; index < domainFiltersArray.length; ++index) {
      var filter = domainFiltersArray[index][1] + ".";
      if( filter.length <= baseDomain.length  ) {
        var sourceDomainStart = baseDomain.substring(0, filter.length);
        if( filter === sourceDomainStart ) {
          return true;
        }
      }
    }

    return false;

  }

  this._setRPCController = function(rpcCtlr) {
    rpcController = rpcCtlr;
  }

}

module.exports = RPCContext;