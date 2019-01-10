px.import("px:scene.1.js" ).then(function ready(scene){
var root = scene.root;
var url = "http://edge.myriad-gn-xcr.xcr.comcast.net/select/image?width=249&height=332&entityId=5881303020318615112";
var image = scene.create({t:"image",url:url, parent:root,clip:false, x:30, y:30, w:123, h:164,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH});

root.on('onKeyDown', function (e) {
  if (e.keyCode == 49)
  {
    image.downscaleSmooth = true;
  }
});


}).catch(function importFailed(e){
  console.error("Import failed for test_downscale.js: " + e)
});

