/**
 * Test for pxscene security model
 *
 * Verifies pxscene's default (bootstrap) permissions config
 */

px.import({
  scene: 'px:scene.1.js',
  url: 'url',
  http: 'http',
  https: 'https'
})
.then(function(imports) {

  var http_module = imports.http;
  var https_module = imports.https;

  function make_url_test(url, allow) {
    return function () {
      return new Promise(function(resolve) {
        var moduleFn = url.indexOf("https") === 0 ? https_module.request : http_module.request;
        var req = moduleFn(url);
        if (req) {
          req.abort();
          resolve(allow ? "SUCCESS" : "FAILURE");
        } else {
          resolve(allow ? "FAILURE" : "SUCCESS");
        }
      });
    };
  }

  function make_service_test(name, allow) {
    return function () {
      return new Promise(function (resolve) {
        try {
          var ret = imports.scene.getService(name);
          resolve((ret && allow) ? "SUCCESS" : "FAILURE");
        } catch (e) {
          resolve(allow ? "FAILURE" : "SUCCESS");
        }
      })
    };
  }

  function make_screenshot_test(allow) {
    return function () {
      return new Promise(function (resolve) {
        try {
          var ret = imports.scene.screenshot('image/png;base64');
          resolve((ret && allow) ? "SUCCESS" : "FAILURE");
        } catch (e) {
          resolve(allow ? "FAILURE" : "SUCCESS");
        }
      })
    };
  }

  function getUrlOrigin(url) {
    if (typeof url === 'string') {
      var urlObject = imports.url.parse(url);
      if (urlObject.host && urlObject.protocol) {
        return urlObject.protocol + (urlObject.slashes ? "//" : "") + urlObject.host;
      }
    }
    return null;
  }

  var this_file_origin = getUrlOrigin(px.getPackageBaseFilePath());
  var no_origin = !this_file_origin;
  console.log("this file's origin: "+this_file_origin);
  var isSTB = px.appQueryParams.stb === "true";
  if (isSTB) {
    // permissions config on STB should allow everything
    // for the trusted hosts
    console.log("STB mode");
  }

  module.exports.tests = {
    test_url_google: make_url_test('https://google.com/', true),
    test_url_localhost: make_url_test('http://localhost', no_origin || isSTB),
    test_url_localhostHttps: make_url_test('https://localhost', no_origin || isSTB),
    test_url_localhostPort: make_url_test('http://localhost:50050', no_origin || isSTB),
    test_url_127001: make_url_test('http://127.0.0.1', no_origin || isSTB),
    test_url_127001Port: make_url_test('http://127.0.0.1:50050', no_origin || isSTB),
    test_url_1: make_url_test('http://[::1]', no_origin || isSTB),
    test_url_1Port: make_url_test('http://[::1]:50050', no_origin || isSTB),
    test_url_00000001: make_url_test('http://[0:0:0:0:0:0:0:1]', no_origin || isSTB),
    test_url_00000001Port: make_url_test('http://[0:0:0:0:0:0:0:1]:50050', no_origin || isSTB),
    test_service_warehouse: make_service_test("Warehouse", isSTB),
    test_feature_screenshot: make_screenshot_test(true)
  };
});
