var px = require('./build/Debug/px');
var win = new px.Window(50, 50, 640, 480);
win.title = 'Hello World!';
win.on('keydown', function(code, flags) {
  console.log('code:' + code);
});
win.on('resize', function(width, height) {
  console.log('resize: ' + width + 'x' + height);
});
win.visible = true;

var scene = win.scene;

// desc is a method. Testing getting a method
var desc = scene.get('description');
console.log('here:' + JSON.stringify(desc));
console.log('there:' + desc);
console.log('description:' + desc.call());

console.log('before');
console.log(JSON.stringify(scene, null, 4));
console.log('after');

// set some properties by name
var p = scene;
p.w = 300;
p.h = 30;
p.fillColor = 0x00ff00ff;
p.lineColor = 0xffffff80;
p.lineWidth = 10;

// now get to test sets
console.log("p.w:" + p.w);
console.log("p.h:" + p.h);
console.log("p.fillColor:" + p.fillColor);
console.log("p.lineColor:" + p.lineColor);
console.log("p.lineWidth:" + p.lineWidth);

setTimeout(function() { console.log('quitting') }, 10000000);
