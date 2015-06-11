var root = scene.root;

var appURLs = [/*"play.js","play2.js","playmask.js","playmask_star.js","playmask_star2.js",*/
               "dom.js","events.js","hello.js","picturepile.js","dynamics.js",
               "mousetest2.js","fancy.js","cliptest.js","masktest.js",
               "mousetest.js","external.js","fonts.js","drag.js"];

var url;
url = process.cwd() + "/../../images/skulls.png";
var bg = scene.createRectangle({url:url,xStretch:2,yStretch:2,parent:root,fillColor:0xe0e0e0ff});

url = process.cwd() + "/../../images/radial_gradient.png";
var bgShade = scene.createImage({url:url,xStretch:1,yStretch:1,parent:root});

var childPad = 48;
var childAppWidth = 1280;
var childAppHeight = 720;
var childAcross = 2;

var select;

var apps = scene.createImage({parent:root, sx:0.25, sy:0.25, w:1280, h:720});

for (var i = 0; i < appURLs.length; i++) {
  var c = scene.createScene({url:appURLs[i], parent:apps, 
                             w:childAppWidth, h:childAppHeight, clip:true
                            });
  
  c.on("onMouseDown", function(e){
    var c = e.target;
    console.log("flags:", e.flags);
    if (e.flags == 4) {  // ctrl-mousedown
      c.cx = c.w/2;
      c.cy = c.h/2;
      c.animateTo({r:c.r+360},3,scene.PX_STOP,scene.PX_END);
    }
    scene.setFocus(c);
    select.animateTo({x:(c.x-childPad)*0.25,y:(c.y-childPad)*0.25},
                     0.3,scene.PX_STOP,scene.PX_END);
  });
  
  if (i == 0) 
    scene.setFocus(c);
}

var url = process.cwd() + "/../../images/select.png";
select = scene.createImage9({parent:root,url:url,lInset:16,tInset:16,rInset:16,bInset:16,
                             w:1368*0.25,h:808*0.25,x:0,y:0,interactive:false});

scene.root.on('onKeyDown', function(e) {
  if (e.keyCode == 32) {
      root.painting = !root.painting;
  }
});

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
