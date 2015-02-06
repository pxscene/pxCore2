//scene.showOutlines = true;
var root = scene.root;

var bg = scene.createImage();
bg.url = process.cwd() + "/../../images/skulls.png";
bg.xStretch = 2;
bg.yStretch = 2;
bg.parent = root;
var bgShade = scene.createImage();
bgShade.url = process.cwd() + "/../../images/radial_gradient.png";
bgShade.xStretch = 1;
bgShade.yStretch = 1;
bgShade.parent = root;

var txt1 = scene.createText();
txt1.x = 10;
txt1.text = "";
txt1.parent = root;

var ball = scene.createImage();
ball.url = process.cwd() + "/../../images/ball.png"
ball.x = 450;
ball.y = 350;
ball.cx = ball.w/2;
ball.cy = ball.h/2;
ball.parent = root;

ball.mask = process.cwd() + "/../../images/ball.png"

var childText = scene.createText();
childText.text = "Hello There!!!";
childText.parent = ball;
childText.y = ball.h/2-childText.h/2;
childText.x = ball.w/2-childText.w/2;
childText.cx = childText.w/2;
childText.cy = childText.h/2;
childText.textColor = 0xff0000ff;
childText.animateTo("r", 360, 1, 0, 2);

// clean up these names and expose as properties off of some object
var pxInterpLinear = 0;
var easeOutElastic = 1;
var easeOutBounce  = 2;
var pxExp = 3;
var pxStop = 4;

function fancy(p) {
  x1(p);
  y1(p);
  rotate1(p);
  scale1(p);
}

function x1(p) {
// animateTo ugliness until I can figure out default/optional args
  p.animateTo("x", 50, 1.0, pxInterpLinear, 0, x2);
}

function x2(p) {
  p.animateTo("x", 450, 3.0, easeOutElastic, 0, fancy);
}

function y1(p) {
  p.y = 100;
  p.animateTo("y", 350, 1.0, easeOutBounce, 0, y2);
}

function y2(p) {
  p.animateTo("y", 150, 1.0, easeOutElastic, 0);
}

function rotate1(p) {
  p.r = 0;
  p.animateTo("r", -360, 2.5, easeOutElastic, 0);
}

function scale1(p) {
  p.animateTo("sx", 0.2, 1, pxInterpLinear, 0, scale2);
  p.animateTo("sy", 0.2, 1, pxInterpLinear, 0);
}

function scale2(p) {
  p.animateTo("sx", 2.0, 1.0, pxExp, 0, scale3);
  p.animateTo("sy", 2.0, 1.0, pxExp, 0);
}

function scale3(p) {
  p.animateTo("sx", 1.0, 1.0, easeOutElastic, 0);
  p.animateTo("sy", 1.0, 1.0, easeOutElastic, 0);
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

scene.on("resize", updateSize);
updateSize(scene.w, scene.h);
