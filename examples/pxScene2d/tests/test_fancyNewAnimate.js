
px.import("px:scene.1.js").then( function ready(scene) {
var root = scene.root;
var basePackageUri = px.getPackageBaseFilePath();
var animateX,animateY,animateR,animateS;

var url;

var txt1 = scene.create({t:"text",x:10,text:"",parent:root,pixelSize:64});

url = basePackageUri + "/images/ball.png"
var ball = scene.create({t:"image",url:url,parent:root});
ball.ready.then(function() {
  ball.cx = ball.resource.w/2;
  ball.cy = ball.resource.h/2;

  fancy(ball);
});
function fancy(o) {
  var startX = 450;
  var startY = 100;

  o.x = startX;
  animateX = o.animate({x:0}, 2.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
  o.y = startY;
  animateY = o.animate({y:0}, 40.0, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_LOOP, 1);
  o.r = 0;
  animateR = o.animate({r:-360}, 30, scene.animation.EASE_OUT_ELASTIC, scene.animation.OPTION_LOOP, 1);
  animateS = o.animate({sx:0.2,sy:0.2},1 , scene.animation.TWEEN_LINEAR, scene.animation.OPTION_LOOP, 1);
  animateX.done.then(function(o){
      animateY.cancel();
      animateR.cancel();
      setTimeout( function()
          {
            animateX = o.animate({x:900}, 2.0, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP, 1);
            animateX.done.then(function(o){
                fancy(o);
            });},
      2000);
  });
}



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

}).catch( function importFailed(err){
  console.error("Import failed for fancy.js: " + err)
});
