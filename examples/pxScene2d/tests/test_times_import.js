"use strict";
//var remote = "/home/cfry002/pxComponents/";//https://cfry002.github.io/pxComponents/"
//var localhost = "http://localhost:8090/"
//console.log("Time is "+Date.now());
var startClock = Date.now();
px.import("px:scene.1.js").then( function ready(scene) {

  var root = scene.root;

  var message = scene.create({t:"text", parent:root, x:250, y:250, text:"DONE: \n"+startClock+" START TIME  \n"+Date.now()+" FINISH TIME"});

 
  }).catch( function importFailed(err){
  console.error("Import for test_import_times.js failed: " + err)
});
