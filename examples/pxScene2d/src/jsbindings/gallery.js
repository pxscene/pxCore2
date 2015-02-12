var root = scene.root;
//scene.showOutlines = true;

var appURLs = ["picturepile.js", "hello.js", "mousetest2.js", "fancy.js", "cliptest.js", "masktest.js", "mousetest.js"];
var url;
url = process.cwd() + "/../../images/skulls.png";
var bg = scene.createImage({url:url,xStretch:2,yStretch:2,parent:root});

url = process.cwd() + "/../../images/radial_gradient.png";
var bgShade = scene.createImage({url:url,xStretch:1,yStretch:1,parent:root});

var childPad = 32;
var childAppWidth = 1280;
var childAppHeight = 720;
var childAcross = 2;

var apps = scene.createImage({parent:root, sx:0.25, sy:0.25, w:1280, h:720});

for (var i = 0; i < appURLs.length; i++) {
    scene.createScene({url:appURLs[i], parent:apps, 
                       w:childAppWidth, h:childAppHeight, clip:(i<2),
                       x:((i%childAcross)*(childAppWidth+childPad))+childPad, 
                       y:(Math.floor(i/childAcross)*(childAppHeight+childPad))+childPad});
}


//apps.painting=false;

scene.on('keydown', function(code, flags) {
  console.log("keydown:" + code);
});

/*
scene.on("mousemove", function(x, y) {
    txt1.text = "" + x+ ", " + y;
});
*/

function updateSize(w, h) {
    bg.w = w;
    bg.h = h;
    bgShade.w = w;
    bgShade.h = h;
}

scene.on("resize", updateSize);
updateSize(scene.w, scene.h);
