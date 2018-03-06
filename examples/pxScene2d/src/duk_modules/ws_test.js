
px.import({ scene: 'px:scene.1', ws: 'ws' }).then( function importsAreReady(imports)
{
    var ws = imports.ws;
    var wsock = ws.connect('wss://echo.websocket.org');

    wsock.on('open', function () {
        console.log('ws on open');

    	wsock.send('All glory to WebSockets!');
    });

    wsock.on('close', function () {
        console.log('ws on closed');
    });

	wsock.on('message', function (data) {
		console.log(data);
	});

}).catch( function importFailed(err){
    console.error("Import failed for ws_test.js: " + err.stack);
});
