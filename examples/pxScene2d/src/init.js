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
global = global || {};
global.console = console = require('console');
global.timers = timers = require('timers');
global.Buffer = Buffer = require('buffer').Buffer;
global.setTimeout = setTimeout = timers.setTimeout;
global.clearTimeout = clearTimeout = timers.clearTimeout;
global.setInterval = setInterval = timers.setInterval;
global.clearInterval = clearInterval = timers.clearInterval;
global.Promise = Promise = require('bluebird');
global.process = process = require('process');
global.pako = pako = require('pako');
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
    var baseViewerUrl = 'https://www.pxscene.org'
 
    function loadUrl(url) {
        var Url = require('url')
        var Path = require('path')

        var ext

        if (true) {
            var urlParts = Url.parse(url,true)
            ext = urlParts.query['_ext']
        }
        else {
            var urlParts = Url.parse(url)
        }

        if (!ext) {
            ext = Path.extname(urlParts.pathname)
        }

        //console.log('Original Url: ', url)
        if (ext=='.md' || ext=='.sd') {
            url = baseViewerUrl+'/mime/viewMarkdown.js?url='+encodeURIComponent(url)
        }
        else if (ext=='.png' || ext == '.jpg' || ext=='.svg') {
            url = baseViewerUrl+'/mime/viewImage.js?url='+encodeURIComponent(url)
        }
        else if (ext=='.txt' || ext=='.text') {
            url = baseViewerUrl+'/mime/viewText.js?url='+encodeURIComponent(url)
        }
        /*
        else if (ext=='.htm' || ext=='.html'){
            url = baseViewerUrl+'/mime/viewHTML.js?url='+encodeURIComponent(url)
        }
        */
        else if (ext=='.js' || ext=='.jar') {
            // Do nothing and let the url fall through
        }
        else {
            // TODO Do a HTTP head check to see if we can get a mimetype/contenttype for routing
        }
        
        //console.log('Rewritten Url: ', url)


        var ctx = new AppSceneContext({        scene: getScene("scene.1"),
                                            makeReady: this.makeReady,
                                        getContextID: this.getContextID,
                                            packageUrl: url,
                                        rpcController: new RPCController() } );

        // console.log("JS >>>> loadURL()  ctx: " + getContextID() );

        ctx.loadScene();
    }
}

