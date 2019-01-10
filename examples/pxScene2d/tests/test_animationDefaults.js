"use strict";
px.import({scene:"px:scene.1.js",assert:"../test-run/assert.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;

var basePackageUri = px.getPackageBaseFilePath();

    var urls = [
      basePackageUri+"/images/ball.png",
      basePackageUri+"/../../../images/tiles/001.jpg",
      basePackageUri+"/../../../images/tiles/002.jpg",
      basePackageUri+"/../../../images/tiles/003.jpg",
      basePackageUri+"/../../../images/dolphin.jpg",
    ];
    
var parent = scene.create({t:'object', parent:root, y: 50, clip:false});

var rect1 = scene.create({t:'rect', x:298, y:298, w:304, h:304,  parent:parent, lineWidth:2});

var ball = scene.create({t:'image', w:300, h:300, stretchX:scene.stretch.STRETCH, stretchY:scene.stretch.STRETCH, parent:parent,url:urls[0]});


ball.ready.then(obj=>{
  console.log("ball is ready");
  
  console.log("anim1");
  ball.animateTo({x:100, y:100}, 1.0).then(obj=>{
    console.log("anim1 done");
  
    console.log("anim2");
    ball.animateTo({x:ball.x+100, y:ball.y+100}, 1.0).then(obj=>{
      console.log("anim2 done");
    
      console.log("anim3");
      ball.animateTo({x:ball.x+100, y:ball.y+100}, 1.0).then(obj=>{
        console.log("anim3 done");
      });
     });
  });
});


}).catch( function importFailed(err){
  console.error("Import for test_fastForward.js failed: " + err)
});
