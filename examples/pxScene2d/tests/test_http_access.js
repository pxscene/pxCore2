px.import({scene:"px:scene.1.js",http:"http"}).then( function ready(imports) {
var scene = imports.scene;
var root = imports.scene.root;
var basePackageUri = px.getPackageBaseFilePath();
var X_axis =0;
var Y_axis =0;
var str = '';
var output = "ERROR";

var c = scene.create({
  t:"scene", url: "http://96.118.6.151/test_http_access_file.js", parent:root});

var d = scene.create({
  t:"scene", url: "test_http_access_file.js", parent:root});

var opt = {
          port: 80,
          hostname: 'localhost',
          path: 'test_http_access.txt',
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
