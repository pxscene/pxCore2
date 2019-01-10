px.import("px:scene.1.js").then( function ready(scene) {

var rect = scene.create({t:"rect", parent:scene.root, w:400, h:400, lineColor:0xFF0000FF, lineWidth:1});
var container = scene.create({t:"object", parent:scene.root, w:400, h:400});
var text2 = scene.create({t:"textBox", parent:container,h:400,w:400,textColor:0xFFDDFFFF,pixelSize:20,fontUrl:"FreeSans.ttf",alignHorizontal:1,alignVertical:1,text:"Pass"});

}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-71.js: " + err)
});
