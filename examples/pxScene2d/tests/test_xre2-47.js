px.import("px:scene.1.js").then( function ready(scene) {
  
var obj = scene.create({t:"object", parent:scene.root, w:500, h:9999, clip:true});
scene.create({t:"rect", parent:obj, w:100, h:100, fillColor:0xFF0000FF});

}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-47.js: " + err)
});
