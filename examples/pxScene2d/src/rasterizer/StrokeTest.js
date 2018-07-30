/*
   pxCore Copyright 2005-2017 John Robinson

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
}).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var root  = imports.scene.root;

  var base    = px.getPackageBaseFilePath();
  var fontRes = scene.create({ t: "fontResource", url: "FreeSans.ttf" });

  var bg = scene.create({t:"rect", parent: root, x: 0, y: 0, w: 100, h: 100, fillColor: "#aaa", id: "Background"});

  var title = scene.create({t:"textBox", text: "", parent: bg, pixelSize: 15, w: 800, h: 80, x: 0, y: 0,
                alignHorizontal: scene.alignHorizontal.LEFT, interactive: false,
                alignVertical:     scene.alignVertical.CENTER, textColor: 0x000000AF, a: 1.0});

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    drawTest
  //
  function drawTest()
  {

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

      var style = "'fill:#707; stroke:#fff; stroke-width:3'";

      var c1 = scene.create( { t: "path", parent: root, 
                               d: "<svg width='200' height='200'>"+
                                  "<circle cx='100' cy='100' r='80' style="+style+"/>"+
                                  "</svg>" } );

      c1.ready.then( function(o)
      {
        c1.x = (bg.w - c1.w) /2;
        c1.y = (bg.h - c1.h) /2;

        c1.x = 10;
        c1.y = 200;

        title.text = "Path SVG  .vs.  Loaded SVG";
      });

      var url = base + "/circle.svg";
      var c2 = scene.create({t:"image", url:url, parent:root, id:"circle" });

      c2.ready.then( function(o)
      {
        c2.x = 200;
        c2.y = 200;
      });


      var cim = scene.create({t:"imageResource", url:url, parent:root, id:"circle22res", w: 200, h: 200 });
      var c3 = scene.create({t:"image", resource: cim, parent:root, id:"circle22"});

      c3.ready.then( function(o)
      {
        console.log("\n\n\n\n ############ READY \n\n\n");

        c3.x = 400;
        c3.y = 200;
      });
     //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

      var rr = 15;

      var r1 = scene.create( { t: "path",  parent: bg,
                               d: "<svg width='200' height='200'>"+
                                  "<rect x='10' y='10' width='150' height='80' rx='" +rr+ "' ry='" +rr+ "' style="+style+"/>"+
                                  "</svg>"} );

      r1.ready.then( function(o)
      {
        r1.x = 10;
        r1.y = 60;
      });

      var url = base + "/rrect.svg";
      var r2 = scene.create({t:"image", url:url, parent:root, id:"rrect" });

      r2.ready.then( function(o)
      {
        r2.x = 200;
        r2.y = 60;
      });
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function updateSize(w,h)
  {
     bg.w = w;
     bg.h = h;

     title.x = 10;
     title.y = bg.h - title.h;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  scene.on("onResize", function(e) { updateSize(e.w, e.h); });

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  Promise.all([ bg.ready, title.ready ])
      .catch( (err) =>
      {
          console.log("SVG >> Loading Assets ... err = " + err);
      })
      .then( (success, failure) =>
      {
          updateSize(scene.w, scene.h);

          drawTest();  // <<<<<< RUN !!!

          bg.focus = true;
      });

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}).catch( function importFailed(err){
  console.error("SVG >> Import failed: " + err);
});
