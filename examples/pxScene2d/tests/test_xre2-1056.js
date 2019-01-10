px.import("px:scene.1.js" ).then(function ready(scene){
var root = scene.root;
var text = scene.create({t:"text", parent:root, x:20, y:100, textColor:0xFFFFFFFF, pixelSize:20,clip:false,text:"Press 1 to reset object.  Object count will be logged and should not grow."});
var url = "http://edge.myriad-gn-xcr.xcr.comcast.net/select/image?width=249&height=332&entityId=5881303020318615112";
//this image will not be visible for this test
var image = scene.create({t:"image",url:url, clip:false, x:30, y:150, w:123, h:164,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH});

root.on('onKeyDown', function (e) {
  if (e.keyCode == 49)
  {
    image = scene.create({t:"image",url:url, clip:false, x:30, y:150, w:123, h:164,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH});  
    scene.logDebugMetrics();
  }
});


}).catch(function importFailed(e){
  console.error("Import failed for test_xre2-1056.js: " + e)
});

