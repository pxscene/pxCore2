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

px.import({
    scene: 'px:scene.1.js'
  })
  .then(function importsReady(imports) {
    var scene = imports.scene;
    var root = scene.root;

    let rect_one = scene.create({
      parent: scene.root,
      t: 'rect',
      id: 'one',
      x: 50,
      y: 50,
      w: 100,
      h: 100
    });

    function listener(event) {
      console.log("Event received");
    }

    scene.on("addEventsProper", function() { rect_one.on('onListener', listener); scene.on('onPendingEvents', listener); console.log("Received add events proper **************");  });
    scene.on("addPendingEvents", function() { scene.on('onPendingEvents', listener); console.log("Received add pending events proper **************");  });
    scene.on("removeEventsProper", function() { rect_one.delListener("onListener", listener); });
    scene.on("removeEventsImProper", function() { try { rect_one.delListener("onListener", null); } catch(e) { console.log("Received exception ");} });
    scene.on("syncEvent", function() { console.log("Sync event received"); rect_one.on("syncEventReceived", function() { console.log("syncEventReceived registered"); }); });
    scene.on("asyncEvent", function() { console.log("Async event received"); rect_one.on("asyncEventReceived", function() { console.log("asyncEventReceived registered"); }); });
  });
