var px = require('./build/Release/px');
var eventLoop = new px.EventLoop();
var win = new px.Window(50, 50, 640, 480);
win.texture = new px.Offscreen();
win.title = 'Hello World!';
win.on('closerequest', function() { eventLoop.exit(); });
win.on('keydown', function(code, flags) {
  console.log('code:' + code);
});
win.on('resize', function(width, height) {
  win.texture.init(width, height);
});
win.visible = true;

eventLoop.run();

setTimeout(function() { console.log('quitting') }, 10000000);
