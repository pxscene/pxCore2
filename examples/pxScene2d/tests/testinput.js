px.import({scene:"px:scene.1.js",http:"http"}).then( function ready(imports) {
var scene = imports.scene;
var root = imports.scene.root;
var basePackageUri = px.getPackageBaseFilePath();
var X_axis =0;
var Y_axis =0;
var str = '';
var output = "ERROR";

var c = scene.create({
  t:"scene", url: "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/test_xre2-973.js", parent:root});

//var d = scene.create({
//  t:"scene", url: "test_xre2-973.js", parent:root});

var opt = {
          port: 80,
          hostname: 'localhost',
          path: '/htmldiag/first.html',
          method: 'POST',
        };
var req = imports.http.request(opt, function (res) {
  res.on('data', function (chunk) {
    str += chunk;
  });
  res.on('end', function () {
    console.log("RESPONSE", str);
    try { 
      var data = JSON.parse(str);
      output = "ImageName:" + data.imageName + ", Status:" + data.status;

    } catch (ignore) {}
  });
});
req.on('error', function() {
});
req.end();
}).catch( function importFailed(err){
  console.log("error ..... " + err);
});
