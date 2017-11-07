
px.import({ scene: 'px:scene.1.js', http: 'http.js' }).then( function importsAreReady(imports)
{
    var http = imports.http;

    respData = '';

    http.get('https://bellard.org/pi/pi2700e9', function (resp) {
        console.log('http in func cb');

        resp.on('data', function (chunk) {
            console.log('http on data');
            respData += chunk;
        });

        resp.on('end', function () {
            console.log('http done. status_code = ' + resp.statusCode
                + ' response size = ' + respData.length);
        });
    }).on('error', function (e) {
        console.log('error fetching data: ' + e.message);
    });

}).catch( function importFailed(err){
    console.error("Import failed for http_test.js: " + err);
});
