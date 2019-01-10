"use strict";
var remote = "/home/root/pxComponents/";//https://cfry002.github.io/pxComponents/"
//var localhost = "http://localhost:8090/"
//console.log("Time is "+Date.now());
var startClock = Date.now();

px.configImport({"components:":remote});

px.import({
    scene                   : "px:scene.1.js",
    uiImageEffects          : 'components:image/imageEffects.js',
    uiImageRenderer         : 'components:image/imageRenderer.js',
    uiAnimateEffects        : 'components:animate/animateEffects.js',
    uiImage                 : 'components:image/image.js',
    uiAnimate               : 'components:animate/animate.js',
    uiWebSocketDataProvider : 'components:dataProvider/webSocketDataProvider.js'}).then( function importsAreReady(imports) {

    var scene     = imports.scene,
        uiImage   = imports.uiImage,
        uiImageEffects      = imports.uiImageEffects,
        uiAnimate           = imports.uiAnimate(scene),
        uiAnimateEffects    = imports.uiAnimateEffects,
        root      = scene.root,
        uiWebSocketDataProvider = imports.uiWebSocketDataProvider,
        uiImageRenderer = imports.uiImageRenderer(scene);


  var message = scene.create({t:"text", parent:root, x:250, y:250, text:"DONE: \n"+startClock+" START TIME  \n"+Date.now()+" FINISH TIME"});

 
  }).catch( function importFailed(err){
  console.error("Import for test_import_times_import.js failed: " + err)
});
