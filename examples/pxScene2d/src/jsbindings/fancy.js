var root = scene.root;

var url;
url = process.cwd() + "/../../images/skulls.png";
var bg = scene.createImage({url:url,xStretch:2,yStretch:2,parent:root});

url = process.cwd() + "/../../images/radial_gradient.png";
var bgShade = scene.createImage({url:url,xStretch:1,yStretch:1,parent:root});

var txt1 = scene.createText({x:10,text:"",parent:root});

url = process.cwd() + "/../../images/ball.png"
var ball = scene.createImage({url:url,x:450,y:350,parent:root});
ball.cx = ball.w/2;
ball.cy = ball.h/2;

// clean up these names and expose as properties off of some object
/*
var pxInterpLinear = 0;
var easeOutElastic = 1;
var easeOutBounce  = 2;
var pxExp = 3;
var pxStop = 4;
*/

function fancy(p) {
  x1(p);
  y1(p);
  rotate1(p);
  scale1(p);
}

function x1(p) {
    p.animateTo({x:50}, 1.0, scene.PX_LINEAR, scene.PX_END, x2);
}

function x2(p) {
    p.animateTo({x:450}, 3.0, scene.PX_EASEOUTELASTIC, scene.PX_END, fancy);
}

function y1(p) {
    p.y = 100;
    p.animateTo({y:350}, 1.0, scene.PX_EASEOUTBOUNCE, scene.PX_END, y2);
}

function y2(p) {
    p.animateTo({y:150}, 1.0, scene.PX_EASEOUTELASTIC, scene.PX_END);
}

function rotate1(p) {
    p.r = 0;
    p.animateTo({r:-360}, 2.5, scene.PX_EASEOUTELASTIC, scene.PX_END);
}

function scale1(p) {
    p.animateTo({sx:0.2,sy:0.2}, 1, scene.PX_LINEAR, scene.PX_END, scale2);
}

function scale2(p) {
    p.animateTo({sx:2.0,sy:2.0}, 1.0, scene.PX_EXP1, scene.PX_END, scale3);
}

function scale3(p) {
    p.animateTo({sx:1.0,sy:1.0}, 1.0, scene.PX_EASEOUTELASTIC, scene.PX_END);
}

fancy(ball);

scene.on('keydown', function(code, flags) {
  console.log("keydown:" + code);
});

scene.on("mousemove", function(x, y) {
    txt1.text = "" + x+ ", " + y;
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
