/**
 * A test for the texture memory limit.
 *
 * How to see it works:
 *   pass args: -textureMemoryLimitInMb=2 -enableTextureMemoryMonitoring=true
 *   pass args: -enableTextureMemoryMonitoring=false
 */

px.import("px:scene.1.js")
.then(function(scene) {
  var w = scene.root.w
    , h = scene.root.h
    , img_w = Math.floor(w / 10)
    , img_h = Math.floor(img_w * h / w)
    , img_url = "https://unsplash.it/"+img_w+"/"+img_h
    , num_concurrent = 10
    , num_images = 0
    , grid
    , text_obj
    ;
  function setupUI() {
    grid = scene.create({
      t:"rect",
      parent:scene.root,
      x:0, y:0, w:w, h:h,
      fillColor:0x000000ff,
      lineWidth:0
    });
    var text_osd = scene.create({
      t:"rect",
      parent:scene.root,
      x:0, y:0, w:100, h:100,
      fillColor:0x00000088,
      lineWidth:0
    });
    text_obj = scene.create({
      t:"text",
      parent:text_osd,
      textColor:0xffffffff,
      pixelSize:30,
      text:"0"
    });
    return Promise.all([grid.ready, text_osd.ready, text_obj.ready]);
  }
  function drawImage() {
    return scene.create({
      t:"image",
      url:img_url + "?" + Math.random(),
      parent:grid,
      x:Math.floor(Math.random() * w / img_w) * img_w,
      y:Math.floor(Math.random() * h / img_h) * img_h,
      w:img_w, h:img_h,
      stretchX:scene.stretch.STRETCH,
      stretchY:scene.stretch.STRETCH
    }).ready;
  }
  function populateGrid() {
    return drawImage().then(function () {
      num_images++;
      text_obj.text = num_images;
      return populateGrid();
    });
  }
  setupUI().then(function () {
    for (var i = 0; i < num_concurrent; i++) {
      populateGrid();
    }
  });
}, function(){
  console.error("import failed");
});
