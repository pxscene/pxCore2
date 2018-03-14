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

"use strict";

px.import("px:scene.1.js").then( function ready(scene) {

  var root = scene.root;
  var scene1;

  console.log("Creating scene1");
  scene1 = scene.create({t:"scene", parent:root, url:"test.js"});
  scene1 = null; 
  }).catch( function importFailed(err){
  console.error("Import for test_scene_create.js failed: " + err)
});
