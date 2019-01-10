px.import("px:scene.1.js" ).then(function ready(scene){
var root = scene.root;
var text = scene.create({t:"text", parent:root, x:20, y:100, textColor:0xFFFFFFFF, pixelSize:20,clip:false,text:"Press 1 to hide key logs.  Press 2 to show key logs."});
//this image will not be visible for this test

root.on('onKeyDown', function (e) {
  if (e.keyCode == 49)
  {
    process.env.PXSCENE_KEY_LOGGING_DISABLED = 1;
  }
  else if (e.keyCode == 50)
  { 
    process.env.PXSCENE_KEY_LOGGING_DISABLED = 0;
  }
});


}).catch(function importFailed(e){
  console.error("Import failed for test_xre2-1056.js: " + e)
});

