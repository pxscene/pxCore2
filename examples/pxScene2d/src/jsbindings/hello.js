var px = require("./build/Debug/px");

var win = new px.Window(50, 50, 640, 480);
win.title = "Hello World!";
win.on("keydown", function(code, flags) {
  console.log("code:" + code);
});
win.on("resize", function(width, height) {
  console.log("resize: " + width + "x" + height);
});
win.visible = true;

var scene = win.scene;

function testScene() 
{
  var root = scene.root;

  var n = 10;
  var nx = 100;
  var ny = 100;

  for (i = 0; i < n; ++i) {
    if (i < 1) {
      p = scene.createRectangle();
      p.w = 300;
      p.h = 30;
      p.fillColor = 0x00ff00ff;
      p.lineColor = 0xffffff80;
      p.lineWidth = 10;  
      p.animateTo("h", 600, 0.5, 0, 0);
    }
    else if (i < n-1) {
      p = scene.createImage();
      p.url = process.cwd() + "/../images/banana.jpg";
      p.cx = p.w/2;
      p.cy = p.h/2;
      ny = 100;
    }
    else {
      p = scene.createText();
      p.text = "pxCore!";
      p.cx = 250;
      p.animateTo("sx", 2.0, 1.0, 0, 0);
      p.animateTo("sy", 2.0, 1.0, 0, 0);
      nx = 400;
      ny = 400;
    }
    
    nx += 10;
    if (nx > 1000) {
      nx = 0;
      ny += 10;
    }

    p.parent = root;
    p.x = nx;
    p.y = ny;
    
    p.rx = 0;
    p.ry = 1.0;
    p.rz = 0;

    p.animateTo("r", 360, 1.0+(i*0.3), 0, 0);
    if (i < n-1) {
      p.animateTo("x", 800, 1.0+(i*0.3), 0, 0);
    }

    p.animateTo("a", 0.1, 2.0, 0, 0);
  }
}

testScene();

setTimeout(function() { console.log("quitting") }, 10000000);
