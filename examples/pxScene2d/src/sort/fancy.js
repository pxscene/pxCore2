console.log("************ loading fancy.js");
px.import("px:scene.1.js").then( function ready(scene) {
var root = scene.root;
var basePackageUri = px.getPackageBaseFilePath();

var url;

/*
url = process.cwd() + "/../../images/skulls.png";
var bg = scene.create({t:"image",url:url,stretchX:2,stretchY:2,parent:root});

url = process.cwd() + "/../../images/radial_gradient.png";
var bgShade = scene.create({t:"image",url:url,stretchX:1,stretchY:1,parent:root});
*/

var txt1 = scene.create({t:"text",x:10,text:"",parent:root,pixelSize:64});

url = "http://www.pxscene.org/examples/px-reference/gallery/images/ball.png"
var ball = scene.create({t:"image",url:url,parent:root});

ball.ready.then(function() {
  ball.cx = ball.resource.w/2;
  ball.cy = ball.resource.h/2;

  fancy(ball);
});


scene.on("onMouseMove", function(e) {
    txt1.text = "" + e.x + ", " + e.y;
});


function updateSize(w, h) {
/*
    bg.w = w;
    bg.h = h;
    bgShade.w = w;
    bgShade.h = h;
*/
    txt1.y = h-txt1.h;
}

scene.on("onResize", function(e) {console.log("fancy resize", e.w, e.h); updateSize(e.w,e.h);});
updateSize(scene.getWidth(), scene.getHeight());

function fancy(o) {
  var startX = 450;
  var startY = 100;

  // animate x and restart the overall animation at end
  o.x = startX;
  o.animateTo({x:50}, 1.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1)
    .then(function(o){
      o.animateTo({x:startX}, 3.0, scene.animation.EASE_OUT_ELASTIC, scene.animation.OPTION_LOOP, 1)
        .then(function(o){
          fancy(o);
      })
  });

  // animate y
  o.y = startY;
  o.animateTo({y:350}, 1.0, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_LOOP, 1)
    .then(function(o) {
      o.animateTo({y:startY}, 1.0, scene.animation.EASE_OUT_ELASTIC, scene.animation.OPTION_LOOP, 1);
  });

  // animate r
  o.r = 0;
  o.animateTo({r:-360}, 2.5, scene.animation.EASE_OUT_ELASTIC, scene.animation.OPTION_LOOP, 1);

  // animate sx, sy
  o.animateTo({sx:0.2,sy:0.2}, 1, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_LOOP, 1)
    .then(function(o){
      o.animateTo({sx:2.0,sy:2.0}, 1.0, scene.animation.TWEEN_EXP1, scene.animation.OPTION_LOOP, 1)
        .then(function(o) {
          o.animateTo({sx:1.0,sy:1.0}, 1.0, scene.animation.EASE_OUT_ELASTIC, scene.animation.OPTION_LOOP, 1);
      })
  });
}

  xmodule.exports = {foo:"Hello"/*function() {console.log('hello via api');}*/};

}).catch( function importFailed(err){
  console.error("Import failed for fancy.js: " + err)
});

