'use strict';

px.import({
  scene:"px:scene.1.js",
  scr_cap_utils:"./screen_capture_utils.jar"
})
.then(function(imports) {
  var w = imports.scene.root.w
    , h = imports.scene.root.h
    , img_url = "https://unsplash.it/"+w+"/"+h+"?random"
    , margin = 5
    , preview_w = 0.25*w
    , preview_h = preview_w * h / w
    , text_w = 0.15*w
    , text_h = preview_h
    , overlay_w = preview_w+text_w+4*margin
    , overlay_h = preview_h+2*margin
    , img
    ;
  function drawImage() {
    return imports.scene.create({
      t:"image",
      url:img_url,
      parent:imports.scene.root,
      x:0, y:0, w:w, h:h,
      stretchX:imports.scene.stretch.STRETCH,
      stretchY:imports.scene.stretch.STRETCH
    }).ready;
  }
  function drawOverlay(previewUrl, img) {
    var overlay = imports.scene.create({
      t:"rect",
      parent:imports.scene.root,
      x:0.5*(w-overlay_w), y:0.5*(h-overlay_h), w:overlay_w, h:overlay_h,
      fillColor:0x222222ff,
      lineWidth:0,
      a:0
    });
    imports.scene.create({
      t:"image",
      url:previewUrl,
      parent:overlay,
      x:margin, y:margin, w:preview_w, h:preview_h,
      stretchX:imports.scene.stretch.STRETCH,
      stretchY:imports.scene.stretch.STRETCH
    })
    .ready
    .then(function () {
      overlay.animateTo({a:1},0.2);
    });
    var text = "PNG\n"+img.getWidth() + "x"+img.getHeight()+"\n"+img.getBitDepth()+" bit";
    var colorType;
    switch (img.getColorType()) {
      case 0: colorType = "grayscale"; break;
      case 2: colorType = "RGB"; break;
      case 3: colorType = "palette"; break;
      case 4: colorType = "grayscale with alpha"; break;
      case 6: colorType = "RGBA"; break;
    }
    if (colorType) {
      text += "\n"+colorType;
    }
    imports.scene.create({
      t:"text",
      parent:overlay,
      x:3*margin+preview_w, y:margin, w:text_w, h:text_h,
      textColor:0xffffffff,
      pixelSize:20,
      text:text
    });
  }
  drawImage()
  .then(function () {
    console.log("image loaded. capturing...");
    return new Promise(function(resolve, reject) {
      // Allow a little time for render of image to finish
      setTimeout( function() { 
      var buf = imports.scr_cap_utils.capture();
      img = new imports.scr_cap_utils.PNGImage(buf);
      console.log("captured. uploading...");
      imports.scr_cap_utils.generateOneTimeUrl(img.getBytes()).then(function(link)
      { 
        resolve(link);
      });
      },2);
    });
  })
  .then(function (link) {
    console.log("screenshot uploaded: " + link);
    drawOverlay(link, img);
  }).catch(function(err) {
    console.log("Error occurred while loading image:"+ err);
  });
}, function(){
  console.error("test_xre2-686 import failed");
});

