var root = scene.root;
//scene.showOutlines = true;

var appURLs = ["events.js","hello.js","picturepile.js", "dynamics.js","mousetest2.js", "fancy.js", "cliptest.js", "masktest.js", "mousetest.js", "external.js", "fonts.js","drag.js"];
var url;
url = process.cwd() + "/../../images/skulls.png";
var bg = scene.createRectangle({url:url,xStretch:2,yStretch:2,parent:root,fillColor:0xe0e0e0ff});

url = process.cwd() + "/../../images/radial_gradient.png";
var bgShade = scene.createImage({url:url,xStretch:1,yStretch:1,parent:root});

var childPad = 48;
var childAppWidth = 1280;
var childAppHeight = 720;
var childAcross = 2;

var paint = true;

var select;

var apps = scene.createImage({parent:root, sx:0.25, sy:0.25, w:1280, h:720});

for (var i = 0; i < appURLs.length; i++) {
    var c = scene.createScene({url:appURLs[i], parent:apps, 
                       w:childAppWidth, h:childAppHeight, clip:true,
  //                     x:((i%childAcross)*(childAppWidth+childPad))+childPad, 
//                       y:(Math.floor(i/childAcross)*(childAppHeight+childPad))+childPad
                              });
/*
  var f = function() {
    return new function(e) {
      scene.setFocus(c);
    }
  }();
*/

  

  c.on("onMouseDown", function(){var foo = c; return function(e){
    console.log(foo.url); 
    scene.setFocus(foo); 
    select.animateTo({x:(foo.x-childPad)*0.25,y:(foo.y-childPad)*0.25},
                     0.3,scene.PX_STOP,scene.PX_END);
  }}());

  if (i == 0) 
    scene.setFocus(c);

}

var url = process.cwd() + "/../../images/select.png";
select = scene.createImage9({parent:root,url:url,x1:16,y1:16,x2:16,y2:16,w:1368*0.25,h:808*0.25,x:0,y:0,interactive:false});

//apps.painting=false;

scene.on('onKeyDown', function(e) {
  if (e.keyCode == 32) {
      paint = !paint;
      root.painting = paint;
  }
});

/*
scene.on("mousemove", function(x, y) {
    txt1.text = "" + x+ ", " + y;
});
*/

function positionApps() {
  for (var i = 0; i < apps.children.length; i++) {
    var c = apps.children[i];
    c.animateTo({x:((i%childAcross)*(childAppWidth+childPad))+childPad,
    y:(Math.floor(i/childAcross)*(childAppHeight+childPad))+childPad},
                0.3, scene.PX_STOP, scene.PX_END);
  }
}

function updateSize(w, h) {
  bg.w = w;
  bg.h = h;
  bgShade.w = w;
  bgShade.h = h;
  root.w = w;
  root.h = h;
  childAcross = Math.floor(w/((childAppWidth+childPad)*0.25));
  if (childAcross<1) 
    childAcross = 1;
  positionApps();
}

scene.on("onResize", function(e){updateSize(e.w,e.h);});
updateSize(scene.w, scene.h);
