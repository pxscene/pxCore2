var root = scene.root;

var url;
url = process.cwd() + "/../../images/skulls.png";
var bg = scene.createImage({url:url,stretchX:2,stretchY:2,parent:root});

url = process.cwd() + "/../../images/radial_gradient.png";
var bgShade = scene.createImage({url:url,stretchX:1,stretchY:1,parent:root});

var txt1 = scene.createText({x:10,text:"",parent:root,pixelSize:64});

var rect = scene.createRectangle({parent:root, draw:true});
rect.w= root.w;
rect.h= root.h;
rect.fillColor = 0x00000000;

url = process.cwd() + "/../../images/ball.png";
var ball = scene.createImage({url:url,parent:rect,clip:false, draw:true});
ball.x = 0;
ball.y = 0;
rect.w = ball.w;
rect.h = ball.h;


url = process.cwd() + "/../../images/postermask2.png";
var logo = scene.createImage({url:url,parent:rect,drawAsMask:true,draw:false});

var childText = scene.createText({text:"Hello There!!!",parent:logo,textColor:0xff0000ff,pixelSize:64});
childText.y = 0;
childText.x = 0;
logo.x = (ball.w-logo.w)/2;
logo.y = (ball.h-logo.h)/2
logo.cx = logo.w/2;
logo.cy = logo.h/2;
logo.animateTo({r:360}, 10, 0, 2);


function fancy(o) {
  var startX = 450;
  var startY = 100;

  // animate x and restart the overall animation at end
  o.x = startX;
  o.animateTo({x:50}, 1.0, scene.PX_LINEAR, scene.PX_END)
    .then(function(z){
      z.animateTo({x:startX}, 3.0, scene.PX_EASEOUTELASTIC, scene.PX_END)
        .then(function(z) {
          fancy(z);
      	  return z;
        })
    });

  // animate y
  o.y = startY;
  o.animateTo({y:350}, 1.0, scene.PX_EASEOUTBOUNCE, scene.PX_END)
    .then(function(z) {
      z.animateTo({y:startY}, 1.0, scene.PX_EASEOUTELASTIC, scene.PX_END);
      return z;
    });

  // animate r
  o.r = 0;
  o.animateTo({r:-360}, 2.5, scene.PX_EASEOUTELASTIC, scene.PX_END);

  // animate sx, sy
  o.animateTo({sx:0.2,sy:0.2}, 1, scene.PX_LINEAR, scene.PX_END)
    .then(function(z){
       z.animateTo({sx:2.0,sy:2.0}, 1.0, scene.PX_EXP1, scene.PX_END)
         .then(function(z) {
            z.animateTo({sx:1.0,sy:1.0}, 1.0, scene.PX_EASEOUTELASTIC, scene.PX_END);
         })
    });
}

fancy(rect);

scene.on('onKeyDown', function(e) {
  console.log("keydown:" + e.keyCode);
});

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

scene.on("onResize", function(e){updateSize(e.w,e.h);});
updateSize(scene.w, scene.h);
