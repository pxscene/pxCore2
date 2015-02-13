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
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
             "FontdinerSwanky.ttf",
             "IndieFlower.ttf",
             "PoiretOne-Regular.ttf",
             "DancingScript-Bold.ttf",
             "Pacifico.ttf",
            ];

console.log("faces: ", faces.length);

var scroll = scene.createImage({parent:root});
var scrollContent = scene.createImage({parent:scroll});

var rowcontainer = scene.createImage({parent:scrollContent});

var p = 0; 
for (var i=0; i < faces.length; i++)
{
    var row = scene.createImage({parent:rowcontainer,y:p});
    var faceName = faces[i]?faces[i]:"FreeSans.ttf";
    console.log(faceName);
    var t = scene.createText({text:"Enter in some text...", 
                              parent:row,x:10,
                              textColor:0x000000ff, pixelSize:36,
                              faceURL:faces[i]});
    var t2 = scene.createText({text:faceName, 
                               parent:row,x:20,y:t.h,
                               textColor:0x000000ff, pixelSize:14, a:0.6});
    
    row.h = t.h+t2.h;
    row.w = 100;
    p += row.h;
}
var select = scene.createRectangle({parent:scrollContent, fillColor:0x000000, 
                                    lineColor:0xffff00ff,
                                    lineWidth:4,w:scene.w,h:100});


function clamp(v, min, max) {
    return Math.min(Math.max(min,v),max);
}

var currentRow = 0;
function selectRow(i) {
    currentRow = i;
    var row = rowcontainer.children[i];
    select.animateTo({x:row.x,y:row.y,h:row.h},0.3,0,0);
    // animate to bring selection into view
    var t = -scrollContent.y;
    if (row.y < t) {
        t = -row.y
        console.log("one");
        scrollContent.animateTo({y:t},0.3, 0, 0);
    }
    else if (row.y+row.h-scene.h > t) {
        t = -(row.y+row.h-scene.h);
        console.log("two");
        scrollContent.animateTo({y:t},0.3, 0, 0);
    }
}

selectRow(currentRow);

function scrollUp() {
    var numRows = rowcontainer.children.length;
//    selectRow(currentRow>0?currentRow-1:0);
    selectRow(clamp(currentRow-1, 0, numRows-1));
}

function scrollDn() {
    var numRows = rowcontainer.children.length;
    console.log("numRows", numRows);
    console.log(currentRow);
//    selectRow((currentRow<(numRows-1))?currentRow+1:numRows-1);
    selectRow(clamp(currentRow+1, 0, numRows-1));
}

function updateText(s) {
    for (var i = 0; i < rowcontainer.children.length; i++) {
        rowcontainer.children[i].children[0].text = s;
    }
}

var str = "";
scene.on("keydown", function (keycode, flags) {
    if (keycode == 38) scrollUp();
    else if (keycode == 40) scrollDn();
    else if (keycode == 8) {
        str = str.substr(0,str.length-1);
        updateText(str);
    }
});

scene.on("onchar", function(c) {
    str += c;
    updateText(str);
});

//root.painting = false;
