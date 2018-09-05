
px.import({ scene: 'px:scene.1', zlib: 'zlib' }).then( function importsAreReady(imports)
{
    var zlib = imports.zlib;

    var deflateObj = zlib.createDeflateRaw({
        flush: zlib.Z_SYNC_FLUSH,
        windowBits: 15,
        memLevel: 8,
    });

    var buffers = [];

    var onData = function (data) { buffers.push(data); };
    var onError = function (err) { print("error deflate: " + err); };

    deflateObj.writeInProgress = true;
    deflateObj.pendingClose = true;
    deflateObj.on('error', onError).on('data', onData);

    var buf = new Buffer([0x23, 0x24, 0x25, 0x26, 0x27])
    var bufArr = [];

    for (i = 0; i < 1024; ++i) {
        bufArr.push(buf);
    }

    var data = Buffer.concat(bufArr);
    deflateObj.write(data);

    var compressedData;
    deflateObj.flush(function() {
        compressedData = Buffer.concat(buffers);
    });
    deflateObj.close();

    deflateObj.removeListener('error', onError);
    deflateObj.removeListener('data', onData);
    deflateObj.writeInProgress = false;
    deflateObj.pendingClose = false;

    print("compressed_len: " + compressedData.length);

    var inflateObj = zlib.createInflateRaw({
        windowBits: 15,
    });

    var decompressedArr = [];
    var onData2 = function (b) { decompressedArr.push(b); };
    var onError2 = function (err) { print("error inflate: " + err); };

    inflateObj.writeInProgress = true;
    inflateObj.pendingClose = true;
    inflateObj.on('error', onError2).on('data', onData2);
    inflateObj.write(compressedData);
    inflateObj.flush(function () {
        var dbuf = Buffer.concat(decompressedArr);
        print("decompressed_len: " + dbuf.length);
        print("compare result: " + Buffer.compare(data, dbuf));
    });
    inflateObj.close();

}).catch( function importFailed(err){
    console.error("Import failed for zlib_test.js: " + err);
});
