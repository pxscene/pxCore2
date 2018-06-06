px.import({ scene:      'px:scene.1.js',
}).then( function importsAreReady(imports)
{
  // global pathJoin testing
  console.log('pathJoin test', pathJoin('3', '2'));
  // rtHttpGetBinding testing
  httpGet('http://pxscene.org/examples/px-reference/gallery/fancy.js', function (res) {
      console.log('httpGet callback', res.statusCode, res.message);
      res.on('data', function (data) {
        console.log('httpGet data', data);
      });
      res.on('error', function (data) {
        console.log('httpGet error', data);
      });
      res.on('end', function (data) {
        console.log('httpGet end', data);
      });
  });
  // rtHttpGetBinding failed testing
  httpGet('xxx');
  httpGet(0, 1);
  httpGet("0", 1);
  // loadlib testing
  console.log('loadLib test', Duktape.loadlib('libau', '/usr/lib/libau.so'));
}).catch( function importFailed(err){
  console.error("Import failed for browser.js: " + err);
});
