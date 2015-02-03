var px = require("./build/Debug/px");

var scene = px.getScene();

//scene.showOutlines = true;

var text = scene.createText();
text.text = "One\nabcdefghijklmnopqrstuvwxyz\nThree";
text.x = 50;
text.rx = 1;
text.rz = 0;
text.parent = scene.root;
text.animateTo("r", 360, 1.0, 0, 2);

scene.on('resize', function(width, height) {
  console.log('resize:' + width + ' height:' + height);
    text.y = height/2;


});

scene.on('keydown', function(code, flags) {
  console.log("keydown:" + code);
});




