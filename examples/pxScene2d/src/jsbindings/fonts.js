var root = scene.root;

var bg = scene.createRectangle({fillColor:0xccccccff, parent:root});
function updateSize(w, h) {
    bg.w = w;
    bg.h = h;
}

scene.on("resize", updateSize);
updateSize(scene.w, scene.h);


// null or "" is the default face FreeSans.ttf
var faces = ["",
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
            ];

console.log("faces: ", faces.length);

var scroll = scene.createImage({parent:root});
var scrollContent = scene.createImage({parent:scroll});

var p = 0; 
for (var i=0; i < faces.length; i++)
{
    var faceName = faces[i]?faces[i]:"FreeSans.ttf";
    console.log(faceName);
    var t = scene.createText({text:faceName, parent:root,x:10,y:p,
                              textColor:0x000000ff, pixelSize:36,
                              faceURL:faces[i]});
    p += t.h;
}

function updateText(s) {
    for (var i = 0; i < scrollContent.children.length; i++) {
        scrollContent.children[i].text = s;
    }
}

scene.on("keydown", function (keycode, flags) {
    if (keycode == 65) updateText("Hello");
    else updateText("There");
});

//root.painting = false;
