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

'use strict';

px.import({scene:"px:scene.1.js"})
.then(function(imports)
{
  var scene = imports.scene;
  var root  = imports.scene.root;

  var txtStart = scene.create({t:"text", parent: root, x: 15, y:30, textColor: "#FFF", text:"Start" });
  var txtEnd   = scene.create({t:"text", parent: root, x:205, y:30, textColor: "#FFF", text:"End" });

  var dy = 60;

  // Count = INFINITE
  var txt0 = scene.create({t:"text", parent: root, x:280, y: dy + 15, textColor: "#000", text:" OSCILLATE  Count= Inf" });
  var box0 = scene.create({t:"rect", id:"box0", parent: root, x: 10, y: dy,      fillColor: "#0F0", w: 50, h: 50 });
  box0.ready.then(function(o)
  {
    o.animateTo({ x: 200  }, 1.0, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, scene.animation.COUNT_FOREVER);
  });

  dy += 60;

  // Count = 1
  var txt1 = scene.create({t:"text", parent: root, x:280, y: dy + 15, textColor: "#000", text:" OSCILLATE  Count= 1" });
  var box1 = scene.create({t:"rect", id:"box1", parent: root, x: 10, y: dy,      fillColor: "#F00", w: 50, h: 50 });
  box1.ready.then(function(o)
  {
    o.animateTo({ x: 200  }, 1.0, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, 1);
  });

  dy += 60;

  // Count = 2
  var txt2 = scene.create({t:"text", parent: root, x:280, y: dy + 15, textColor: "#000", text:" OSCILLATE  Count= 2" });
  var box2 = scene.create({t:"rect", id:"box2", parent: root, x: 10, y: dy,      fillColor: "#F00", w: 50, h: 50 });
  box2.ready.then(function(o)
  {
    o.animateTo({ x: 200  }, 1.0, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, 2);
  });

  dy += 60;

  // Count = 3
  var txt3 = scene.create({t:"text", parent: root, x:280, y: dy + 15, textColor: "#000", text:" OSCILLATE  Count= 3" });
  var box3 = scene.create({t:"rect", id:"box3", parent: root, x: 10, y: dy,      fillColor: "#F00", w: 50, h: 50 });
  box3.ready.then(function(o)
  {
    o.animateTo({ x: 200  }, 1.0, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, 3);
  });

})
.catch(function(err){
  console.error("test_OSCILLATE.js import failed: " + err)
});
