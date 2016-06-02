
var px = require("./build/Debug/px");
var scene = px.getScene(0, 0, 200, 200);
var rect = scene.createRectangle();
// rect.parent = scene.root;
rect.parent = root;
rect.x = 10;
rect.y = 10;
rect.w = 50;
rect.h = 50;
rect.fillColor = 0xffff00ff;
rect.lineColor = 0x00ff00ff;
rect.lineWidth = 10;
