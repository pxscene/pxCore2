"use strict";

px.import("px:scene.1.js").then( function ready(scene) {

  var root = scene.root;
  var scene1;
  var scene2;
  var scene3;

  console.log("Creating scene1");
  scene1 = scene.create({t:"scene", parent:root, url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/test_rect_lines.js"});
  scene1.ready.then(function(s1) {
      console.log("Promise for scene1");
      
      console.log("Creating scene2");
      scene2 = scene.create({t:"scene", parent:root, url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/test_xre2-41.js"});
      scene2.ready.then(function(s2) {
          console.log("Promise for scene2"); 
          scene1.remove();
          console.log("Setting scene1 = null");
          scene1 = null;
          
          console.log("Creating scene3");
          scene3 = scene.create({t:"scene", parent:root, url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/test_xre2-47.js"});
          scene3.ready.then(function(s3) {
              console.log("Promise for scene3");
              scene2.remove();
              console.log("Setting scene2 = null");
              scene2 = null;
              
              scene3.remove();
              console.log("Setting scene3 = null");
              scene3 = null;
            });
        });
      });
  

 
  }).catch( function importFailed(err){
  console.error("Import for test_scene_create.js failed: " + err)
});
