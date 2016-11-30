"use strict";

px.import("px:scene.1.js").then( function ready(scene) {
  console.log("Test ");

  }).catch( function importFailed(err){
  console.error("Import for test_scene_create.js failed: " + err)
});
