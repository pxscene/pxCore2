px.import("px:scene.1.js").then( function ready(scene) {
  
var obj = scene.create({t:"object", parent:scene.root, w:1000, h:1000, clip:true});
// rect with fillColor ff - we see line rendered
scene.create({t:"rect", parent:obj, w:400, h:400,fillColor:0xCC0088ff, lineWidth:2,lineColor:0x00FF00FF});

// rect with fillColor 00 - we should still see line rendered (bug: but we don't) 
scene.create({t:"rect", parent:obj, x:450, w:400, h:400,fillColor:0xCC008800, lineWidth:2,lineColor:0x00FF00FF});

}).catch( function importFailed(err){
  console.error("Import failed for test_rect_lines.js: " + err)
});
