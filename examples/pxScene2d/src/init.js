global.console = require('rcvrcore/console.js');
global.timers = require('rcvrcore/timers.js');
global.Promise = require('bluebird.js');

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
        obj.setResolve(resolve);
        obj.setReject(reject);
    });
}

var AppSceneContext = require('rcvrcore/AppSceneContext.js');
var RPCController = require('rcvrcore/rpcController.js');

global.loadUrl = function loadUrl(url) {

  var ctx = new AppSceneContext({        scene: getScene("scene.1"),
                                     makeReady: this.makeReady,
                                  getContextID: this.getContextID,
                                    packageUrl: url,
                                 rpcController: new RPCController() } );

  // console.log("JS >>>> loadURL()  ctx: " + getContextID() );

  ctx.loadScene();
}


