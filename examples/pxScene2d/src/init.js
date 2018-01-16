var isDuk = (typeof Duktape != "undefined")?true:false;

if (isDuk) {
global.console = require('console');
global.timers = require('timers');
global.Promise = require('bluebird');

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

