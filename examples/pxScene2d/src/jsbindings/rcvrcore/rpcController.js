/**
 * Created by tjcarroll2
 * on 2/6/16.
 */

function RPCController() {
  var registeredApps = {};

  this.registerApp = function(targetName, scene) {
    if( registeredApps.hasOwnProperty(targetName) ) {
      console.error("RPC registerApp:  App name RPC target already exists for  " + targetName);
      return false;
    }
    registeredApps[targetName] = scene;
  };

  this.execute = function(sourceTarget, remoteTargetName, functionName, args, callback) {
    if( !registeredApps.hasOwnProperty(remoteTargetName) ) {
      console.error("RPC execute: App name RPC target doesn't exist for  " + remoteTargetName);
      return false;
    }

    var rtnValue = registeredApps[remoteTargetName].getRPCContext()._execute(sourceTarget, remoteTargetName, functionName, args);
    if( callback !== undefined && callback !== null ) {
      callback(rtnValue);
    }
  }
}

module.exports = RPCController;

