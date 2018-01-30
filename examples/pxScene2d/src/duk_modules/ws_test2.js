
px.import({ scene: 'px:scene.1', ws: 'ws' }).then( function importsAreReady(imports)
{
    var ws = imports.ws;
    var wsock = ws.connect('wss://echo.websocket.org');

    wsock.on('open', function () {
        console.log('connected');
        wsock.send(Date.now());
    });

    wsock.on('close', function () {
        console.log('disconnected');
    });

	wsock.on('message', function (data) {
		console.log("Roundtrip time: " + (Date.now() - data));

  		setTimeout(function () {
    		wsock.send(Date.now());
  		}, 500);
	});

}).catch( function importFailed(err){
    console.error("Import failed for ws_test.js: " + err.stack);
});
