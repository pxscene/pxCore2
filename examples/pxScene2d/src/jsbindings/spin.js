var root = scene.root;

var url;
url = process.cwd() + "/../../images/skulls.png";
var bg = scene.create({t:"image", url:url,stretchX:scene.stretch.REPEAT,stretchY:scene.stretch.REPEAT,
                            parent:root});
bg.animateTo({r:360},60.0,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP);

url = process.cwd() + "/../../images/radial_gradient.png";
var bgShade = scene.create({t:"image", url:url,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH,
                                 parent:root});

scene.create({t:"scene", url:"fonts2.js",parent:root,w:1280,h:720});

function updateSize(w, h) {
  //var d = Math.pow(Math.pow(w,2)+Math.pow(h,2),0.5);
  var d = Math.pow(1280*1280+1280*1280,0.5);
  bg.x = -d;
  bg.y = -d;
  bg.cx = d;
  bg.cy = d;
  bg.w = d*2;
  bg.h = d*2;
  bgShade.w = w;
  bgShade.h = h;
 }

scene.on("onResize", function(e) {console.log("fancy resize", e.w, e.h); updateSize(e.w,e.h);});
updateSize(scene.w, scene.h);
