var root = scene.root;


var width = 800;
//var bg = scene.createRectangle({fillColor:0xccccccff, parent:root});
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

console.log("fonts: "+fonts.length);
scene.w = width;
console.log("scene.w="+scene.w);

var scroll = scene.createImage({parent:root});
var scrollContent = scene.createImage({parent:scroll,w:width});

var rowcontainer = scene.createImage({parent:scrollContent});

pleaseWait = scene.create({t:"textBox",text:"Please wait while fonts load...", 
                              parent:root,x:10,y:0,
                              textColor:0xfaebd7ff, pixelSize:24,
                              fontUrl:fonts[i], clip:true, w:width,h:100});
var elems = [];
var promises = [];
var p = 0; 
for (var i=0; i < fonts.length; i++)
{
    var row = scene.createImage({parent:rowcontainer,y:0, clip:false});
    var faceName = fonts[i]?fonts[i]:"FreeSans.ttf";
    console.log("fontFace: "+faceName);
    var t = scene.create({t:"textBox",text:"Enter in some text...", 
                              parent:row,x:10,y:0,
                              textColor:0xfaebd7ff, pixelSize:24,
                              fontUrl:fonts[i], clip:true,w:width,h:100, draw:false});
    elems[i] = t;                           
    promises[i] = t.ready;

}
Promise.all(promises).then(function(success, failure) {

  pleaseWait.remove();
  pleaseWait = null;
  for(var n = 0; n < elems.length; n++) {

    console.log("IN PROMISE n="+n);
                var t = elems[n];
                t.draw = true;
                var fontMetrics = t.getFontMetrics();
                console.log("natural leading is "+fontMetrics.naturalLeading);
                console.log("fontMetrics.height="+fontMetrics.height);
                console.log("fontMetrics.baseline="+fontMetrics.baseline);
                console.log("fontMetrics.ascent="+fontMetrics.ascent);
                console.log("fontMetrics.descent="+fontMetrics.descent);
                t.h = fontMetrics.height;
                t.parent.h = t.h+(fontMetrics.naturalLeading);
                t.parent.w = width;
                if( n != 0) {
                    var prevParent = elems[n-1].parent;          

                          console.log("PrevParent elem is "+elems[n-1].fontUrl);

                          t.parent.y = prevParent.h+prevParent.y; 
                          console.log("Prevparent y is "+prevParent.y);
                          console.log("PrevParent.h set to "+prevParent.h);
                          
                  } 
                  else  {
                      var row = rowcontainer.children[n];
                      select.animateTo({x:row.x,y:row.y,h:row.h,w:row.w},0.3,0,0);                   
                  }
                }

              });
//}

var select = scene.createRectangle({parent:scrollContent, fillColor:0x000000, 
                                    lineColor:0xffff00ff,
                                    lineWidth:4,w:scene.w,h:100});


function clamp(v, min, max) {
    return Math.min(Math.max(min,v),max);
}
var measurements;

var currentRow = 0;
function selectRow(i) {
    currentRow = i;
    var row = rowcontainer.children[i];
    measurements = row.children[0].measureText();
    //width = measurements.bounds.x2 + row.children[0].x;
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

