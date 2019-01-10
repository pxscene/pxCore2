"use strict";
px.import({scene:"px:scene.1.js",
            keys:"px:tools.keys.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var keys = imports.keys;


var cycleTimer;
var cycleIndex; 
var cycleSize; 

var basePackageUri = px.getPackageBaseFilePath();

    var urls = [
      basePackageUri+"/images/ball.png",
      basePackageUri+"/../../../images/tiles/001.jpg",
      basePackageUri+"/../../../images/tiles/002.jpg",
      basePackageUri+"/../../../images/tiles/003.jpg",
      basePackageUri+"/../../../images/dolphin.jpg",
    ];
    
var parent = scene.create({t:'object', parent:root, y: 50, clip:false});

var widgets = [];
widgets.push(scene.create({t:'image', parent:parent,url:urls[0]}));
widgets.push(scene.create({t:'image', parent:parent,url:urls[1]}));
widgets.push(scene.create({t:'image', parent:parent,url:urls[2]}));
widgets.push(scene.create({t:'image', parent:parent,url:urls[3]}));
widgets.push(scene.create({t:'image', parent:parent,url:urls[4]}));


cycleSize = widgets.length;
cycleIndex = 0;

var cyclePictures = function() {

  widgets[cycleIndex++].moveToFront();

  if( cycleIndex >= cycleSize) {
    cycleIndex = 0;
  }

}


  scene.root.on("onKeyDown", function (e) {
    var code  = e.keyCode;
    var flags = e.flags;
    console.log(">>>>> onKeyDown! "+code);
    
    if( keys.is_CTRL(flags)) {
      
      switch(code) { 
        case keys.ZERO: { // A 
          console.log("SetInterval"); 
          cycleTimer = setInterval(cyclePictures, 15000);
          break;
        }
        case keys.ONE: { // B 
          console.log("Clear Interval!"); 
          clearInterval(cycleTimer);
          cycleTimer = null;
          break;
        }
        
        default:
        {
          console.log("Key not handled");
          break;
        }
      }
    }
    console.log("DONE WITH KEY PRESS");
  }
  
);

}).catch( function importFailed(err){
  console.error("Import for test_intervals.js failed: " + err)
});
