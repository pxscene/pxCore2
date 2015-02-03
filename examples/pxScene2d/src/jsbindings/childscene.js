var px = require("./build/Debug/px");

var scene = px.getScene();
var root = scene.root;

scene.showOutlines = true;

var text = scene.createText();
text.text = "One\nabcdefghijklmnopqrstuvwxyz\nThree";
text.x = 50;
text.rx = 1;
text.rz = 0;
text.parent = root;
text.animateTo("r", 360, 1.0, 0, 2);

var childScene = scene.createScene();
childScene.parent = root;

var childInnerScene = childScene.innerScene;
var childRoot = childInnerScene.root;

var childText = childInnerScene.createText();
childText.text = "Hello from child";
childText.parent = childRoot;
childText.animateTo("r", 360, 1.0, 0, 2);
childText.cx = 300;
childText.y = 300;

scene.on('resize', function(width, height) {
  console.log('resize:' + width + ' height:' + height);
    text.y = height/2;
});

scene.on('keydown', function(code, flags) {
  console.log("keydown:" + code);
});
