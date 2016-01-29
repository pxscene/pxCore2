var root = scene.root;


//var bg = scene.create({t:"rect", fillColor:0xccccccff, parent:root});
function updateSize(w, h) {
//    bg.w = w;
//    bg.h = h;
}

scene.on("onResize", function(e){updateSize(e.w,e.h);});
updateSize(scene.w, scene.h);


// null or "" is the default font FreeSans.ttf
var fonts = ["",
             "http://54.146.54.142/tom/xre2/apps/receiver/fonts/XFINITYSansTT-New-Bold.ttf",
             "http://54.146.54.142/tom/xre2/apps/receiver/fonts/XFINITYSansTT-New-BoldCond.ttf",
             "http://54.146.54.142/tom/xre2/apps/receiver/fonts/XFINITYSansTT-New-ExLgt.ttf",
             "http://54.146.54.142/tom/xre2/apps/receiver/fonts/XFINITYSansTT-New-Lgt.ttf",
             "http://54.146.54.142/tom/xre2/apps/receiver/fonts/XFINITYSansTT-New-Med.ttf",
             "http://54.146.54.142/tom/xre2/apps/receiver/fonts/XFINITYSansTT-New-MedCond.ttf",
             "http://54.146.54.142/tom/xre2/apps/receiver/fonts/DejaVuSans.ttf",
             "http://54.146.54.142/tom/xre2/apps/receiver/fonts/DejaVuSerif.ttf",
            ];

console.log("faces: ", fonts.length);

var scroll = scene.create({t:"image", parent:root});
var scrollContent = scene.create({t:"image", parent:scroll});

var rowcontainer = scene.create({t:"image", parent:scrollContent});

var p = 0; 
for (var i=0; i < fonts.length; i++)
{
    var row = scene.create({t:"image", parent:rowcontainer,y:p});
    var faceName = fonts[i]?fonts[i]:"FreeSans.ttf";
    console.log(faceName);
    var t = scene.create({t:"text", text:"Enter in some text...", 
                              parent:row,x:10,
                              textColor:0xffffffff, pixelSize:24,
                              fontUrl:fonts[i]});
    var t2 = scene.create({t:"text", text:faceName, 
                               parent:row,x:20,y:t.h,
                               textColor:0x000000ff, pixelSize:14,a:0.6});
    
    row.h = t.h+t2.h;
    row.w = 800;

    p += row.h;
}
var select = scene.create({t:"rect", parent:scrollContent, fillColor:0x000000, 
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
scene.root.on("onKeyDown", function (e) {
    var keycode = e.keyCode; var flags = e.flags;
    if (keycode == 38) scrollUp();
    else if (keycode == 40) scrollDn();
    else if (keycode == 8) {
//        str = str.substr(0,str.length-1);
//        str = str.slice(0,str.length-2);
      str = str.slice(0,-1);
      updateText(str);
    }
});

scene.root.on("onChar", function(e) {
  if (e.charCode != 8) {
    str += String.fromCharCode(e.charCode);
    updateText(str);
  }
});

