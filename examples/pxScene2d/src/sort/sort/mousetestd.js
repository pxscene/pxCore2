//scene.showOutlines = true;
debugger;
var text = scene.createText({text:"CLICK ME!!",parent:scene.root,pixelSize:64});
text.x = (scene.w-text.w)/2;
text.y = (scene.h-text.h)/2;
text.cx = text.w/2;
text.cy = text.h/2;
//text.animateTo({"r": -360}, 5.0, 0, 2);
var rTarget = 0;
text.on("onMouseDown", function(e) {
    rTarget += 360;
    text.animateTo({r:rTarget}, 1.0, 4, 0); 
//  console.log(e);
  if (e.target == text)
    console.log("object identity working");
  else
    console.log("object identiy broken");
  
1});

scene.root.id="editorroot";
text.id="editortext";

scene.on('onResize', function(e) {
    text.y = (e.h-text.h)/2;
});

scene.on('onKeyDown', function(e) {
  console.log("keydown:" + e.keyCode);
});




