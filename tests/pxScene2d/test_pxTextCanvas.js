/*

 pxCore Copyright 2005-2019 John Robinson

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

"use strict";
px.configImport({"px.local:": px.getPackageBaseFilePath()+"/"});

px.import({
    scene:  "px:scene.1.js"
    , assert: "./test-run/assert.js"
    , manual: "./test-run/tools_manualTests.js"
}).then( function ready(imports) {

    let sparkscene = imports.scene;
    let assert = imports.assert.assert;
    let manual = imports.manual;

    let manualTest = manual.getManualTestValue();
    let parent = sparkscene.root;
    let promises = [];

    // Can be replaced with the standard logger, consider including 'px:tools.../Logger.js'
    let Logger = function(enabled = false) {
        this.enabled = enabled;
        this.enable = () => { this.enabled = true; };
        this.diable = () => { this.enabled = false; };
        this.message = function () {if (this.enabled) { console.log.apply(this, arguments); }};
    };
    let logger = new Logger(true);

    //let fontUrlBase = "http://sparkui.org/examples/fonts/";
    // local:
    let fontUrlBase = "/Users/silver/developer/spark/simple-lightning-spark-apps/v0.simpleImage/static/fonts/";
    let IndieFlower = "IndieFlower.ttf";
    let DejaVu = "DejaVuSans.ttf";

    let fontUrl = fontUrlBase + IndieFlower;
    let fontUrl2 = fontUrlBase + DejaVu;
    let fontResource = sparkscene.create({t:"fontResource", url: fontUrl});
    let fontResource2 = sparkscene.create({t:"fontResource", url: fontUrl2});

    let w = 300;
    let h = 300;
    // let x = 100;
    // let y = 0;
    let canvas;

    promises.push(fontResource.ready);
    promises.push(fontResource2.ready);

    // beforeStart will verify that we have correct resolutions for the fontResources that were preloaded
    // as well as for the canvas scene we're going to test
    let beforeStart = function () {
        return new Promise(function (resolve, reject) {
            let results = [];
            let message;
            Promise.all(promises).then(function () {
                message = 'promise resolved for the fonts';
                results.push(assert(true, message));
            }, function () {
                message = 'Not expected rejection received for the fonts';
                results.push(assert(false, message));
                reject(results);
            }).then(function () {
                canvas = sparkscene.create({
                    t: "textCanvas"
                    , w: w
                    , h: h
                    , parent: parent
                    , text: "Bitter sweet"
                    , font: fontResource
                });
                return canvas.ready.then(
                    () => {
                        message = 'promise resolved for the textCanvas scene';
                        results.push(assert(true, message));
                    },
                    () => {
                        message = 'Not expected rejection received the textCanvas scene';
                        results.push(assert(false, message));
                        reject(results);
                    });
            }).then(function () {
                resolve(results);
            });
        });
    };

    ///////////////////////
    // Test functions begin

    let fillStyle = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.fillStyle = 0xFF6600FF;
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let fillText = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.fillText('Hello, world', 0, 0);
            //canvas.clear();
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let textBaseline = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.textBaseline = 'hanging';
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let globalAlpha = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.globalAlpha = 1.0;
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let shadowColor = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.shadowColor = 0xFF6600FF;
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let shadowBlur = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.shadowBlur = 5;
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let shadowOffsetX = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.shadowOffsetX = 1.5;
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let shadowOffsetY = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.shadowOffsetY = 1.6;
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let measureText = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            let text = 'I need to measure this';
            let measure = canvas.measureText(text);
            logger.message('Width of "' + text + '" is ' + measure.width);
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let clear = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            logger.message("Canvas", canvas);
            canvas.fillStyle = 0xFFFFFFFF;
            canvas.clear();
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let fillRect = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.fillRect(0, 0, 10, 10);
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    let translate = function(params) {
        return new Promise(function(resolve, reject) {
            var results = [];
            logger.message("Executing", params.name, params.description);
            canvas.translate(10, 10);
            canvas.ready.then(function() {
                results.push(assert(true, params.name + ": expected and received resolution"));
                resolve(results);
            }, function() {
                results.push(assert(false, params.name + ": expected resolution but received rejection"));
                reject(results);
            }).then(function() {
                resolve(results);
            });
        });
    };

    // Test functions end
    //////////////////////////

    // Function executor with timeout
    let timeout = 1000; //msecs
    let exec = function (params) {
        return new Promise(function (resolve, reject) {
            let results = [];
            let message;

            let timer = setTimeout(function () {
                message = params.name + ' never got promise!';
                logger.message('debug', message);
                results.push(assert(false, message));
                reject(results);
            }, timeout);

            params.func(params).then(function (res) {
                results.push(res);
                clearTimeout(timer);
                resolve(results);
            }, function () {
                clearTimeout(timer);
                reject(results);
            });
        }, function () {
            logger.message('debug', 'exec() got a rejection promise');
        });
    };

    // We execute the functions through exec() by supplying them as a params object
    // params object: {name: 'function name', func: functionName, description: 'some optional text'};

    let beforeStartEx = function() {
        return exec({name: 'beforeStart', func: beforeStart});
    };

    let tests = {
        0: () => {return exec({name: 'fillStyle', func: fillStyle, description: 'promise test'})}
        , 1: () => {return exec({name: 'fillText', func: fillText, description: 'promise test'})}
        , 2: () => {return exec({name: 'textBaseline', func: textBaseline, description: 'promise test'})}
        , 3: () => {return exec({name: 'shadowColor', func: shadowColor, description: 'promise test'})}
        , 4: () => {return exec({name: 'shadowBlur', func: shadowBlur, description: 'promise test'})}
        , 5: () => {return exec({name: 'shadowOffsetX', func: shadowOffsetX, description: 'promise test'})}
        , 6: () => {return exec({name: 'shadowOffsetY', func: shadowOffsetY, description: 'promise test'})}
        , 7: () => {return exec({name: 'measureText', func: measureText, description: 'promise test'})}
        , 8: () => {return exec({name: 'globalAlpha', func: globalAlpha, description: 'promise test'})}
        , 9: () => {return exec({name: 'fillRect', func: fillRect, description: 'promise test'})}
        , 10: () => {return exec({name: 'clear', func: clear, description: 'promise test'})}
        , 11: () => {return exec({name: 'translate', func: translate, description: 'promise test'})}
    };

    if(manualTest === true) {
        logger.message('Manual mode.');
        manual.runTestsManually(tests, beforeStartEx);
    }

    logger.message('done.');
}).catch(function importFailed(err) {
    console.error("Import failed for test-textcanvas.js: " + err);
});
