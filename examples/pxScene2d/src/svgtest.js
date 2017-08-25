

px.import({ scene:      'px:scene.1.js',
             keys:      'px:tools.keys.js',
}).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = imports.scene.root;

        
  var bg = scene.create({t:"rect",  parent: root, x:0, y:0, w: 100, h: 100, fillColor: 0x888888ff, a: 1.0 });

   scene.create( { t: "path", d:"M100 180 q50 10, 95 80",     strokeColor: 0x000000ff, strokeWidth: 6, parent: bg} );  // relative
   scene.create( { t: "path", d:"M100 180 Q150 190, 195 260", strokeColor: 0xFF0000ff, strokeWidth: 3, parent: bg} );  // absolute

//##################################################################################################################################
        
  bg.on("onMouseUp", function(e)
  {
    if(true)
    {
//##################################################################################################################################
//
//------ LINE/MOVE
//
//       scene.create( { t: "path", d:"M100 100 L200 200",                         fillColor: 0x00FF88ff, parent: bg} );
//    var shape = scene.create( { t: "path", d:"M100 100 L200 100 200 200 100 200 100 100", strokeColor: 0xFF000Fff, strokeWidth: 6, parent: bg} );
      var shape = scene.create( { id: "Path ("+e.x+","+e.y+")",  t: "path", d:"M0 0 l100 0 0 100 -100 0 0 -100", interactive: false,
                             x: e.x-50, y: e.y-50, w: 100, h: 100, fillColor: 0xFF000088, strokeColor: 0x000000ff, strokeWidth: 6, parent: bg} );
      
//      var shape = scene.create( { id: "Path ("+e.x+","+e.y+")",  t: "path", d:"M50 0 L100 100  0 100  50 0", interactive: false,
//                             x: e.x-50, y: e.y-50, w: 100, h: 100, fillColor: 0xFF000088, strokeColor: 0x000000ff, strokeWidth: 6, parent: bg} );
        
     shape.ready.then
     (
      function(o)
      {
        o.moveToFront(); // <<<< Dunno why I need this.
      
        var px = o.x + 100;
      
        console.log("\n SVG >> animateTo !!!!!! ");

        o.animateTo({ x: px }, 10.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1);
      }
    );
//
//##################################################################################################################################
//
//------ VERTICAL
//
//       scene.create( { t: "path", d:"M100 100 v-50",  strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // relative
//       scene.create( { t: "path", d:"V200", strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // absolute
             
//##################################################################################################################################
//
//------ HORIZONTAL
//
//       scene.create( { t: "path", d:"M100 100 h-50",  strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // relative
//       scene.create( { t: "path", d:"H200", strokeColor: 0x00FF88ff, strokeWidth: 6, parent: bg} );  // absolute
             
//##################################################################################################################################
//
//------ QUADRATIC CURVE
//
//         scene.create( { t: "path", d:"M10 80 Q 50 10, 95 80 T 180 80 270 110", strokeColor: 0x000000ff, strokeWidth: 3, parent: bg} );  // relative

//         These curves should overlap, Black Below, Red Above
//         scene.create( { t: "path", d:"M100 180 q50 10, 95 80",     strokeColor: 0x000000ff, strokeWidth: 6, parent: bg} );  // relative
//         scene.create( { t: "path", d:"M100 180 Q150 190, 195 260", strokeColor: 0xFF0000ff, strokeWidth: 3, parent: bg} );  // absolute
             
//##################################################################################################################################
//
//------ CUBIC CURVE
//
//             var curve = scene.create( { t: "path", d:"M100 180 c100,100 250,100 250,200", strokeColor: 0x00FF88ff, strokeWidth: 3, parent: bg} );  // relative
//             var curve = scene.create( { t: "path", d:"M0 0 c100,100 250,100 250,200", strokeColor: 0x00FF88ff, strokeWidth: 3, parent: bg, x: 200, y: 400, w:10, h: 10 } );  // relative
//         scene.create( { t: "path", d:"M100,200 C100,100 250,100 250,200", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );  // absolute
//         var curve = scene.create( { t: "path", d:"M100,200 C100,100 250,100 250,200 S400,300 400,200", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );  // absolute
 
//         curve.ready.then
//         (
//          function(o)
//          {
//             curve.w = 310; curve.h = 210;
//
//             curve.animateTo({ x: 400.0  }, 10.75, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1).then
//             (
//              function(o) {
//              
//              }
//              );
//         }
//        );
             
//             scene.create( { t: "path", d:"M0.330689 0 c -0.182634,0 -0.330689,0.148055 -0.330689,0.330689 0,0.182638 0.148055,0.330689 0.330689,0.330689 0.182634,0 0.330689,-0.148051 0.330689,-0.330689 0,-0.182634 -0.148055,-0.330689 -0.330689,-0.330689z", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );
             
//          scene.create( { t: "path", d:"M89.171,155.172c2.668-0.742,4.23-3.531,3.475-6.238c-0.76-2.678-3.549-4.266-6.265-3.492 c-2.669,0.76-4.235,3.568-3.476,6.24C83.673,154.365,86.454,155.948,89.171,155.172z", strokeColor: 0x000000ff, strokeWidth: 7, parent: bg} );
     
//             scene.create( { t: "path", d:"rect( rx:'15', ry:'15' )", strokeColor: 0x00FF00ff, strokeWidth: 3, parent: bg, x: 600, y: 800, w: 300, h: 150;} );
//         scene.create( { t: "path", d:"M50 200 L350 200", strokeColor: 0x00FF00ff, strokeWidth: 3, parent: bg} );
             
//       scene.create( { t: "path", d:"M10 80 l100 0   0 100   -100 0   0 -100", fillColor: 0x00FF88ff, parent: bg} );
             
//       scene.create( { t: "path", d:"M10 80 Q50 10, 95 80 T 180 80 270 110", fillColor: 0x00FF88ff, parent: bg} );
//       scene.create( { t: "path", d:"M10 80 Q50 10, 95 80 T 180 80 270 110 M100 100 L200 200", fillColor: 0x00FF88ff, parent: bg} );
        
//##################################################################################################################################
    }
  });  

  function updateSize(w,h)
  {
    console.log("SVG >> Resizing... WxH: " + w + " x " +h);

    bg.w = w;
    bg.h = h;
  }

  scene.on("onResize", function(e) { updateSize(e.w, e.h); });

  Promise.all([ bg ])
      .catch( (err) => 
      {
          console.log("SVG >> Loading Assets ... err = " + err);
      })
      .then( (success, failure) =>
      {
            console.log("\n START SVG >> Children of [bg] = " + bg.children.length);
            
        updateSize(scene.w, scene.h);
      });

}).catch( function importFailed(err){
  console.error("SVG >> Import failed for svgtest.js: " + err);
});





/*
 canvas.on("onMouseUp", function(e)
 {
 if(false)
 {
 console.log("Calling PATH draw !");
 scene.create( {t: "path", d:"M10 80 Q 50 10, 95 80 T 180 80 270 110", fillColor: 0x00FF88ff, a: 1.0 } );
 }
 
 if(false)
 {
 canvas.drawCurve(     {id: "QUAD", x1: 10, y1: 10, x2: 300, y2: 300, x3: 500, y3: 10, fillColor: 0x00FF88ff, a: 1.0 } );
 canvas.drawRectangle( {id: "MARKER", x:  10, y:  10, w: 10, h: 10, fillColor: 0x00FF00ff, a: 1.0 } );
 canvas.drawRectangle( {id: "MARKER", x: 300, y: 300, w: 10, h: 10, fillColor: 0x00FF00ff, a: 1.0 } );
 canvas.drawRectangle( {id: "MARKER", x: 500, y:  10, w: 10, h: 10, fillColor: 0x00FF00ff, a: 1.0 } );
 canvas.closePath();
 }
 
 if(false)
 {
 var h = canvas.h;
 
 canvas.drawCurve(     {id: "CUBIC", x1: 10, y1: h - 10, x2: 110, y2: h - 180, x3: 280, y3: h - 20, x4: 500, y4: h - 210, fillColor: 0x00FF88ff, a: 1.0 } );
 canvas.drawRectangle( {id: "MARKER", x:  10, y: h -  10, w: 10, h: 10, fillColor: 0x00FF00ff, a: 1.0 } );
 canvas.drawRectangle( {id: "MARKER", x: 110, y: h - 180, w: 10, h: 10, fillColor: 0x00FF00ff, a: 1.0 } );
 canvas.drawLine(      {id: "LINE", x1:  10, y1: h -  10,x2: 110, y2:h -  180, fillColor: 0x00FF00ff, a: 1.0 } );
 
 canvas.drawRectangle( {id: "MARKER", x: 280, y: h -  20, w: 10, h: 10, fillColor: 0x00FF00ff, a: 1.0 } );
 canvas.drawRectangle( {id: "MARKER", x: 500, y: h - 210, w: 10, h: 10, fillColor: 0x00FF00ff, a: 1.0 } );
 canvas.drawLine(      {id: "LINE", x1: 280, y1: h - 20, x2: 500, y2:h -  210, fillColor: 0x00FF00ff, a: 1.0 } );
 
 canvas.drawLine(      {id: "LINE", x2:  110, x1: 280, y1:h -  20,y2: h -  180, fillColor: 0x00FF00ff, a: 1.0 } );
 }
 
 if(false)
 {
 var r = 10
 canvas.drawCurve( {id: "CUBIC", x1: 110, y1:  10, x2:  10, y2:  50, x3:  10, y3: 100, x4: 110, y4: 140 } );
 canvas.drawCurve( {id: "CUBIC", x1: 110, y1: 140, x2: 210, y2: 100, x3: 210, y3:  50, x4: 110, y4:  10 } );
 }
 
 //    canvas.drawCurve( {id: "Dummy drawCurve",  x2: e.x, y2: e.y, x3: e.x + 100, y3: e.y + 50, fillColor: 0x00FF88ff, a: 1.0 } );
 
 //  canvas.drawCircle( {id: "Dummy drawCircle",  x: e.x, y: e.y, r:88, fillColor: 0x00FF88ff, a: 1.0 } );
 
 //  canvas.drawRectangle( {id: "Dummy drawRectangle",  x: e.x, y: e.y, w: 60, h: 60, fillColor: 0x112233FF, a: 1.0 } );
 
 //  canvas.drawRectangle( {id: "Dummy drawRectangle",  x: e.x, y: e.y, w: 60, h: 60, rx: 5, fillColor: 0xFF0000ff, a: 1.0 } );
 // canvas.drawRectangle( {id: "ROUNDED",  x: e.x, y: e.y, w: 160, h: 160, rx: 15, ry: 30, fillColor: 0xFF0000ff, a: 1.0 } );
 
 
 //                 canvas.drawPath("M10 80 Q 95 10 180 80");
 
 // canvas.drawLine( { name: "Dummy drawRectangle", x0: e.x, y0: e.y, x1: canvas.w, y1: canvas.h, fillColor: 0x000000ff, stroke: 10 }  );
 
 //            var line = canvas.create( { t: "line", x0: e.x, y0: e.y, x1: canvas.w, y1: canvas.h, fillColor: 0x000000ff, stroke: 10 }  );
 //            var rect = canvas.create( { t: "rectangle", x0: e.x, y0: e.y, x1: canvas.w, y1: canvas.h, fillColor: 0x000000ff, stroke: 10 }  );
 //   var rect = canvas.drawRectangle( { name: "Dummy drawRectangle", x: e.x, y: e.y, w: 55, h: 85, fillColor: 0x00FF88ff }  );
 
 //             canvas.drawCircle( { name:  "Dummy drawCircle", x: e.x, y: e.y, fillColor: 0x00FF00ff } );
 //            canvas.drawEllipse( { name:  "Dummy drawEllipse", x: e.x, y: e.y, rx: 100, ry: 160, fillColor: 0xFF000088 } );
 
 //                  canvas.drawPolyline(  "Dummy drawPolyline"  );
 //                  canvas.drawPolygon(   "Dummy drawPolygon"   );
 });
 */