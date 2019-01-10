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
        function resolveAfterDelay() {
          setTimeout(function () {
            resolve("SUCCESS");
          }, 2000);
        }
        var moduleFn = url.indexOf("https") === 0 ? https_module.request : http_module.request;
        var req = moduleFn(url, function () {
          if (allow) {
            resolveAfterDelay();
          } else {
            resolve("FAILURE: received response while should not make request");
          }
        });
        req.on('response', function () {
          if (allow) {
            resolveAfterDelay();
          } else {
            resolve("FAILURE: received response while should not make request");
          }
        });
        req.on('socket', function () {
          if (!allow) {
            resolve("FAILURE: received socket while should not make request");
          }
        });
        req.on('error', function (e) {
          if (e.message.indexOf("Permissions block") === 0 && req.blocked === true) {
            if (allow) {
              resolve("FAILURE: received 'Permissions block' while should not be blocked");
            } else {
              // wait 'blocked' event
            }
          } else if (e.message.indexOf("CORS block") === 0 && req.blocked === true) {
            if (!allow) {
              resolve("FAILURE: received 'CORS block' while should not make request");
            } else {
              // wait 'blocked' event
            }
          } else {
            if (allow) {
              resolveAfterDelay();
            } else {
              resolve("FAILURE: received error but it is not related to CORS or permissions");
            }
          }
        });
        req.setTimeout(10000, function () {
          resolve("FAILURE: received socket time out");
        });
        req.on('blocked', function (e) {
          if (e.message.indexOf("Permissions block") === 0 && req.blocked === true) {
            if (allow) {
              resolve("FAILURE: received 'Permissions block' while should not be blocked");
            } else {
              resolveAfterDelay();
            }
          } else if (e.message.indexOf("CORS block") === 0 && req.blocked === true) {
            if (!allow) {
              resolve("FAILURE: received 'CORS block' while should not make request");
            } else {
              resolveAfterDelay();
            }
          } else {
            resolve("FAILURE: received 'blocked' event but of wrong type");
          }
        });
        req.end();
      });
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

  // Figure out which default permissions are for this origin and platform
  var this_file_origin = null;
  var path = px.getPackageBaseFilePath();
  if (typeof path === 'string') {
    var urlObject = imports.url.parse(path);
    if (urlObject.host && urlObject.protocol) {
      this_file_origin = urlObject.protocol + (urlObject.slashes ? "//" : "") + urlObject.host;
    }
  }
  var isSTB = px.appQueryParams.stb === "true";
  var no_origin = !this_file_origin;
  var is_pxscene_org = this_file_origin === "https://www.pxscene.org";
  var is_localhost_allowed = no_origin || isSTB || is_pxscene_org;
  console.log("this file's origin:",this_file_origin,"isSTB:",isSTB);

  module.exports.tests = {
    test_url_google: make_url_test('https://google.com/', true),
  /* test_url_localhost: make_url_test('http://localhost', is_localhost_allowed),
    test_url_localhostHttps: make_url_test('https://localhost', is_localhost_allowed),
    test_url_localhostPort: make_url_test('http://localhost:50050', is_localhost_allowed),
    test_url_127001: make_url_test('http://127.0.0.1', is_localhost_allowed),
    test_url_127001Port: make_url_test('http://127.0.0.1:50050', is_localhost_allowed),
    test_url_1: make_url_test('http://[::1]', is_localhost_allowed),
    test_url_1Port: make_url_test('http://[::1]:50050', is_localhost_allowed),
    test_url_00000001: make_url_test('http://[0:0:0:0:0:0:0:1]', is_localhost_allowed),
    test_url_00000001Port: make_url_test('http://[0:0:0:0:0:0:0:1]:50050', is_localhost_allowed), */
    test_feature_screenshot: make_screenshot_test(true)
  };
});
