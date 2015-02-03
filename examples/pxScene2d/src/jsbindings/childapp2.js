var text = scene.createText();
text.text = 'Hello from context2!';
text.x = 50;
text.y = 200;
text.rx = 1;
text.rz = 0;
text.parent = scene.root;
text.animateTo('r', 360, 1.0, 0, 2);
