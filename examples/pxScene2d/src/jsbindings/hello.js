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

console.log('description:' + scene.get('description'));

console.log('before');
console.log(JSON.stringify(scene, null, 4));
console.log('after');

setTimeout(function() { console.log('quitting') }, 10000000);
