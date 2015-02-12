//scene.showOutlines = true;

var text = scene.createText({text:"CLICK ME!!",parent:scene.root,pixelSize:64});
text.x = (scene.w-text.w)/2;
text.y = (scene.h-text.h)/2;
text.cx = text.w/2;
text.cy = text.h/2;
//text.animateTo({"r": -360}, 5.0, 0, 2);
text.on("mousedown", function() {
    text.animateTo({r:360}, 1.0, 4, 0, function(o) { o.r = 0; }); 
1});

scene.root.id="editorroot";
text.id="editortext";

scene.on('resize', function(width, height) {
  console.log('resize:' + width + ' height:' + height);
    text.y = height/2;
});

scene.on('keydown', function(code, flags) {
  console.log("keydown:" + code);
});




