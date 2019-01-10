px.import("px:scene.1.js").then( function ready(scene) {
  
var obj = scene.create({t:"object", parent:scene.root, w:500, h:720, clip:true});
//scene.create({t:"rect", parent:obj, w:100, h:100, fillColor:0xFF0000FF});

    var picture = scene.create({t:"image",parent:obj, url:"https://px-apps.sys.comcast.net/pxscene-samples/images/dolphin.jpg"});
picture.ready.then(function() {
setTimeout(function() 
{ 
  picture.url = "";
}, 3000);
});
}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-41.js: " + err)
});
