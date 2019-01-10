"use strict";

/** Test multi-byte character rendering when wordwrap is on **/

var charsToTest = ["\u2018","\u2019", "\u00A9","\u2117", "\u2022", "\u2014", "\u2026", "\u2039", "\u203A","\u2109","\u2103"];         
var charsIcons = ["\uE215", "\uE017","\uE01A","\uE01B","\uE01E","\uE025","\uE027","\uE03A","\uE07C","\uE029",];
px.import({scene:"px:scene.1.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;

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
             "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITY_Sans_icons_v1.1.ttf"
            ];
            

  var x = 50;
  var y = 50;
  var pixelSize = 28;
  var size = charsToTest.length;

  y= 0;
  x=0;
  for(var i = 0; i < fonts.length; i++) {
    for(var c = 0; c < size; c++) {
      scene.create({t:"textBox",parent:root,fontUrl:fonts[i],x:x, y:y, w:150,h:100,wordWrap:true,truncation:scene.truncation.TRUNCATE_AT_WORD,pixelSize:pixelSize, text:charsToTest[c]});
      x+= 75;
    }
    y+=35;
    x = 0;
   
  }

  // Now, render various icons from the Xfinity icon ttf
  var specialCharSize = charsIcons.length;
  var symbolFont = fonts.length - 1;
  for(var c = 0; c < specialCharSize; c++) {
    scene.create({t:"textBox",parent:root,fontUrl:fonts[symbolFont],x:x, y:y, w:150,h:100,wordWrap:true,truncation:scene.truncation.TRUNCATE_AT_WORD,pixelSize:pixelSize, text:charsIcons[c]});
    x+= 75;
  }

}).catch( function importFailed(err){
  console.error("Import for test_chars.js failed: " + err)
});
