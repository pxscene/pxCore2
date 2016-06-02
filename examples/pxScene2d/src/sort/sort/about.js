var root = scene.root;

var bg = scene.createRectangle({fillColor:0xccccccff, parent:root});

function updateSize(w, h) {
    bg.w = w;
    bg.h = h;
}

scene.on("onResize", function(e){updateSize(e.w,e.h);});
updateSize(scene.w, scene.h);

var scroll = scene.createImage({parent:root});
var scrollContent = scene.createImage({parent:scroll});

var rowcontainer = scene.createImage({parent:scrollContent});

var p = 0; 
var row = scene.createImage({parent:rowcontainer,y:p});
var faceName = "FreeSans.ttf";
console.log(faceName);
var t = scene.createText({text:"Version 1.0", 
                          parent:row,x:10,
                          textColor:0x000000ff, pixelSize:18,
                          faceURL:faceName});
row.h = t.h;
row.w = 800;
p += row.h;
