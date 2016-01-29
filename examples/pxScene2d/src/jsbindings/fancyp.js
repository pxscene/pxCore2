var root = scene.root;

var url;
url = process.cwd() + "/../../images/skulls.png";
var bg = scene.create({t:"image", url:url,stretchX:scene.stretch.REPEAT,stretchY:scene.stretch.REPEAT,parent:root});

url = process.cwd() + "/../../images/radial_gradient.png";
var bgShade = scene.create({t:"image", url:url,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH,parent:root});

var txt1 = scene.create({t:"text", x:10,text:"",parent:root});

url = process.cwd() + "/../../images/ball.png"
var ball = scene.create({t:"image", url:url,parent:root});
ball.cx = ball.w/2;
ball.cy = ball.h/2;

function fancy(o) {
  var startX = 450;
  var startY = 100;

  // animate x and restart the overall animation at end
  o.x = startX;
  o.animateTo({x:50}, 1.0, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_END)
    .then(function(o){
      return o.animateTo({x:startX},3.0,scene.animation.EASE_OUT_ELASTIC,
                          scene.animation.OPTION_END)})
    .then(function(){console.log("restart");fancy(o);});

  // animate y
  o.y = startY;
  o.animateTo({y:350}, 1.0, scene.animation.EASE_OUT_BOUNCE, scene.animation.OPTION_END)
    .then(function(o) {
      o.animateTo({y:startY}, 1.0, scene.animation.EASE_OUT_ELASTIC, 
                  scene.animation.OPTION_END);
    });

  // animate r
  o.r = 0;
  o.animateTo({r:-360},2.5,scene.animation.EASE_OUT_ELASTIC,scene.animation.OPTION_END);

  // animate sx, sy
  o.animateTo({sx:0.2,sy:0.2},1,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_END)
    .then(function(o){
      return o.animateTo({sx:2.0,sy:2.0},1.0,scene.animation.TWEEN_EXP1,
                         scene.animation.OPTION_END);
    })
    .then(function(o) {
      o.animateTo({sx:1.0,sy:1.0}, 1.0, scene.animation.EASE_OUT_ELASTIC, 
                  scene.animation.OPTION_END);
    });
}

fancy(ball);

scene.on("onMouseMove", function(e) {
  txt1.text = "" + e.x+ ", " + e.y;
});

function updateSize(w, h) {
    bg.w = w;
    bg.h = h;
    bgShade.w = w;
    bgShade.h = h;
    txt1.y = h-txt1.h;
}

scene.on("onResize", function(e) {console.log("fancy resize", e.w, e.h); updateSize(e.w,e.h);});
updateSize(scene.w, scene.h);
