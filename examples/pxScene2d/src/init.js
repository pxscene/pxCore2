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

var isDuk = (typeof Duktape != "undefined")?true:false;
var isV8 = (typeof _isV8 != "undefined")?true:false;

if (isDuk) {
global.console = require('console');
global.timers = require('timers');
global.Promise = require('bluebird');
global.websocket = require("ws.js");

global.setTimeout = timers.setTimeout;
global.clearTimeout = timers.clearTimeout;
global.setInterval = timers.setInterval;
global.clearInterval = timers.clearInterval;

Promise.setScheduler(function (fn) {
    var timer = uv.new_timer.call({});
    uv.timer_start(timer, 0, 0, fn);
});

global.constructPromise = function (obj) {
    return new Promise(function (resolve, reject) {
        // TODO Don't use the then method... reentrant recursion... 
        obj.then2(resolve, reject);
    });
}
}
else if (isV8) {
console = require('console');
timers = require('timers');

setTimeout = timers.setTimeout;
clearTimeout = timers.clearTimeout;
setInterval = timers.setInterval;
clearInterval = timers.clearInterval;
}

var AppSceneContext = require('rcvrcore/AppSceneContext');
var RPCController = require('rcvrcore/rpcController');

if (isDuk) {
global.loadUrl = function loadUrl(url) {

  var ctx = new AppSceneContext({        scene: getScene("scene.1"),
                                     makeReady: this.makeReady,
                                  getContextID: this.getContextID,
                                    packageUrl: url,
                                 rpcController: new RPCController() } );

  // console.log("JS >>>> loadURL()  ctx: " + getContextID() );

  ctx.loadScene();
}
}
else {
    function loadUrl(url) {

        var ctx = new AppSceneContext({        scene: getScene("scene.1"),
                                           makeReady: this.makeReady,
                                        getContextID: this.getContextID,
                                          packageUrl: url,
                                       rpcController: new RPCController() } );
      
        // console.log("JS >>>> loadURL()  ctx: " + getContextID() );
      
        ctx.loadScene();
      }
}

