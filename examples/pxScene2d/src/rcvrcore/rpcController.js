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
    return true;
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
  };
}

module.exports = RPCController;

