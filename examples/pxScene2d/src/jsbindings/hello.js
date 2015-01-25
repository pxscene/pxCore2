var px = require('./build/Release/px');
var eventLoop = new px.EventLoop();
var win = new px.Window(50, 50, 6480, 480);
win.title = 'Hello World!';
win.on('closerequest', function() { eventLoop.exit(); });
win.on('keydown', function(code, flags) {
  console.log('code:' + code);
});
win.on('resize', function(width, height) {
  consoole.log('w: ' + width + ' h: ' + height);
});
win.visible = true;

eventLoop.run();

setTimeout(function() { console.log('quitting') }, 10000000);
