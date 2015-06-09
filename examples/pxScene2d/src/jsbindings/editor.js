//scene.showOutlines = true;

var text = scene.createText({text:"One\nabcdefghijklmnopqrstuvwxyz\nThree",parent:scene.root,r:30});
text.x = (scene.w-text.w)/2;
text.y = (scene.h-text.h)/2;
text.cx = text.w/2;
text.cy = text.h/2;
//text.animateToF({"r": -360}, 5.0, 0, 2);
text.on("mousedown", function() {
    text.animateToF({r:360}, 1.0, 0, 0, function(o) { o.r = 0; }); 
    console.log("Yay!");
});

scene.root.id="editorroot";
text.id="editortext";

scene.on('resize', function(width, height) {
  console.log('resize:' + width + ' height:' + height);
    text.y = height/2;
});

scene.on('keydown', function(code, flags) {
  console.log("keydown:" + code);
});




