var text = scene.createText();
text.text = 'Hello from context1!';
text.x = 50;
text.y = 100;
text.rx = 1;
text.rz = 0;
text.parent = scene.root;
text.animateTo("r", 360, 1.0, 0, 2);

var child = scene.createScene();
child.parent = scene.root;
runtime.loadScriptForScene(child.innerScene, "childapp2.js");
