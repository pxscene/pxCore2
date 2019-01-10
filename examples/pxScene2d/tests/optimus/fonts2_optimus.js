px.import("px:scene.1.js").then( function ready(scene) {
  
var root = scene.root;


// null or "" is the default font FreeSans.ttf
// Using "" in fonts array tests that local font files can be loaded, too

var fonts = ["",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-Bold.ttf",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-MedCond.ttf",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-Bold.ttf",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-BoldCond.ttf",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-ExLgt.ttf",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-Lgt.ttf",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-Med.ttf",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-MedCond.ttf",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/DejaVuSans.ttf",
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/DejaVuSerif.ttf",
            ];

// Example for using getFont for font metrics
var myFont = scene.create({t:"fontResource",url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-Bold.ttf"});

myFont.ready.then(function(font) {
  console.log("!CLF: First Promise received");

	console.log("inside font.ready");

	metrics = font.getFontMetrics(35);
	console.log("metrics h="+metrics.height);
	console.log("metrics a="+metrics.ascent);
	console.log("metrics d="+metrics.descent);
  console.log("metrics naturalLeading="+metrics.naturalLeading);
  console.log("metrics baseline="+metrics.baseline);
  
  var measure = font.measureText( 35, "Please type some text...");
  console.log("measure w="+measure.w);
  console.log("measure h="+measure.h);
  });

var scroll = scene.create({t:"image",parent:root});
var scrollContent = scene.create({t:"image",parent:scroll});

var rowcontainer = scene.create({t:"image",parent:scrollContent});

var prevRow;

var p = 0; 
for (var i=0; i < fonts.length; i++)
{
  var row = scene.create({t:"image",parent:rowcontainer,a:0});
  
  var faceName = fonts[i]?fonts[i]:"FreeSans.ttf";
  var t = scene.create({ 
    t:"text",
    parent:row,x:10,
    textColor:0xfaebd7ff, pixelSize:36,
    fontUrl:fonts[i],
    text: "Please type some text..."});
  var t2 = scene.create({t:"text",text:"" + (i+1) + ". " + faceName, 
                             parent:row,x:20,
                             textColor:0xfaebd7ff, pixelSize:14,a:0.6,
                             fontUrl:"FreeSans.ttf"});
  

  // Use promises to layout the rows as the text becomes ready
  var rowReady = new Promise(
    
    function(fulfill,reject) {

      var prevRowCopy = prevRow;
      var rowCopy = row;
      var tCopy = t;
      var t2Copy = t2;

      // Please note that rowReady at this point is the rowReady for the previous row
      Promise.all([t.ready,t2.ready,rowReady]).then(function() {
        console.log("IN PROMISE ALL!");
        t2Copy.y = tCopy.h;
        rowCopy.h = tCopy.h+t2Copy.h;

        if (prevRowCopy) {
          rowCopy.y = prevRowCopy.y + prevRowCopy.h;
        }
        else
          selectRow(0); // This resizes the select rectangle once we have the first one

        rowCopy.animateTo({a:1},0.6,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1);
        fulfill(rowCopy);  // done with this row
      }, function() {
        // If .all fails to resolve set the row height to zero and hide it
        // Fulfill the rowReady anyway so the next row can be layed out
        rowCopy.h = 0;
        rowCopy.a = 0;
        fulfill(rowCopy);
      });
      
    });
  
  row.w = 800;
  prevRow = row;
}

var select = scene.create({t:"rect",parent:scrollContent, fillColor:0x000000, 
                                    lineColor:0xffff00ff,
                                    lineWidth:4,w:scene.w,h:100,
                                    a:0});


function clamp(v, min, max) {
    return Math.min(Math.max(min,v),max);
}

var currentRow = 0;
function selectRow(i) {
    currentRow = i;
    var row = rowcontainer.children[i];
    select.animateTo({x:row.x,y:row.y,h:row.h},0.3,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1);
    // animate to bring selection into view
    var t = -scrollContent.y;
    if (row.y < t) {
        t = -row.y
        scrollContent.animateTo({y:t},0.3, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1);
    }
    else if (row.y+row.h-scene.h > t) {
        t = -(row.y+row.h-scene.h);
        scrollContent.animateTo({y:t},0.3, scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1);
    }
}

selectRow(currentRow);

function scrollUp() {
    var numRows = rowcontainer.children.length;
    selectRow(clamp(currentRow-1, 0, numRows-1));
}

function scrollDn() {
    var numRows = rowcontainer.children.length;
    selectRow(clamp(currentRow+1, 0, numRows-1));
}

function updateText(s) {
    for (var i = 0; i < rowcontainer.children.length; i++) {
        rowcontainer.children[i].children[0].text = s;
    }
}

var str = "";
root.on("onKeyDown", function (e) {
    var keycode = e.keyCode; var flags = e.flags;
    if (keycode == 38) scrollUp();
    else if (keycode == 40) scrollDn();
    else if (keycode == 8) {
      str = str.slice(0,-1);
      updateText(str);
    }
});
root.on("onPreKeyDown", function (e) {
  console.log("Fonts2.js onPreKeyDown");
});
root.on("onChar", function(e) {
  if (e.charCode != 8) {
    str += String.fromCharCode(e.charCode);
    updateText(str);
  }
});

function updateSize(w, h) {
  select.w = w;
}
module.exports.resume = function(reason) {
  console.log("fonts2_optimus got resume call");
  select.a = 1;
}
module.exports.suspend = function(reason) {
  console.log("fonts2_optimus got suspend call");
  select.a = 0;

}
scene.on("onResize", function(e){updateSize(e.w,e.h);});
updateSize(scene.w, scene.h);

var opt = scene.getService('.optimus');
var basePackageUri = px.getPackageBaseFilePath();
opt.createApplication({x:0,y:500,w:scene.w,h:scene.h,id:"answers",launchParams:{cmd:"spark",uri:basePackageUri+"/../../gallery/answers.js"}} );

}).catch( function importFailed(err){
  console.error("Import failed for fonts2.js: " + err)
});
