
px.import({ scene: 'px:scene.1', crypto: 'crypto' }).then( function importsAreReady(imports)
{
    var crypto = imports.crypto;

    var shasum = crypto.createHash('sha1');
    shasum.update('13-1515969045324258EAFA5-E914-47DA-95CA-C5AB0DC85B11');
    var serverKey = shasum.digest('base64');
    //var serverKey = shasum.digest('hex');

    print(serverKey);

}).catch( function importFailed(err){
    console.error("Import failed for crypto_test.js: " + err);
});
