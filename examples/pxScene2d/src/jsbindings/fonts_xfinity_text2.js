var root = scene.root;


//var bg = scene.createRectangle({fillColor:0xccccccff, parent:root});
function updateSize(w, h) {
//    bg.w = w;
//    bg.h = h;
}

scene.on("onResize", function(e){updateSize(e.w,e.h);});
updateSize(scene.w, scene.h);


// null or "" is the default face FreeSans.ttf
var faces = ["",
             "XFINITYSansTT-New-Bold.ttf",
             "XFINITYSansTT-New-BoldCond.ttf",
             "XFINITYSansTT-New-ExLgt.ttf",
             "XFINITYSansTT-New-Lgt.ttf",
             "XFINITYSansTT-New-Med.ttf",
             "XFINITYSansTT-New-MedCond.ttf",
             "DejaVuSans.ttf",
             "DejaVuSerif.ttf",
            ];

console.log("faces: ", faces.length);

var scroll = scene.createImage({parent:root});
var scrollContent = scene.createImage({parent:scroll});

var rowcontainer = scene.createImage({parent:scrollContent});

var elems = [];
var p = 0; 
for (var i=0; i < faces.length; i++)
{
    var row = scene.createImage({parent:rowcontainer,y:0, clip:false});
    var faceName = faces[i]?faces[i]:"FreeSans.ttf";
    console.log(faceName);
    var t = scene.createText2({text:"Enter in some text...", 
                              parent:row,x:10,y:0,
                              textColor:0xfaebd7ff, pixelSize:24,
                              faceURL:faces[i], clip:true, w:scene.w,h:100});
    elems[i] = t;                           
    

}
for(var n = 0; n < elems.length; n++) {
  
          elems[n].ready.then(function(t) {
             console.log("Inside ready for face="+t.faceURL);

                var fontMetrics = t.getFontMetrics();
                console.log("natural leading is "+fontMetrics.naturalLeading);
                console.log("fontMetrics.height="+fontMetrics.height);
                t.h = fontMetrics.height;
                t.parent.h = t.h+(fontMetrics.naturalLeading/2);
                t.parent.w = 800;
                for( var i = 0; i < elems.length; i++) {
                  if( i != 0 && t === elems[i]) { 
                    var prevParent = elems[i-1].parent;
                    console.log("PrevParent elem is "+elems[i-1].faceURL);
                    console.log("Prevparent y is "+prevParent.y);
                    console.log("PrevParent.h set to "+prevParent.h);
                    t.parent.y = prevParent.h+prevParent.y; 
                    
                    break;
                  } else if( i == 0) {
                      var row = rowcontainer.children[i];
                      select.animateTo({x:row.x,y:row.y,h:row.h,w:row.w},0.3,0,0);                   
                  }
                }

              });
}

var select = scene.createRectangle({parent:scrollContent, fillColor:0x000000, 
                                    lineColor:0xffff00ff,
                                    lineWidth:4,w:scene.w,h:100});


function clamp(v, min, max) {
    return Math.min(Math.max(min,v),max);
}
var measurements;
var width;
var currentRow = 0;
function selectRow(i) {
    currentRow = i;
    var row = rowcontainer.children[i];
    measurements = row.children[0].measureText();
    width = measurements.bounds.x2 + row.children[0].x;
    select.animateTo({x:row.x,y:row.y,h:row.h, w:width},0.3,0,0);
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

    selectRow(clamp(currentRow-1, 0, numRows-1));
}

function scrollDn() {
    var numRows = rowcontainer.children.length;
    console.log("numRows", numRows);
    console.log(currentRow);

    selectRow(clamp(currentRow+1, 0, numRows-1));
}

function updateText(s) {
    for (var i = 0; i < rowcontainer.children.length; i++) {
        console.log("updateText row i="+i+" with text="+s);
        rowcontainer.children[i].children[0].text = s;
        
    }
    //selectRow(currentRow);
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

