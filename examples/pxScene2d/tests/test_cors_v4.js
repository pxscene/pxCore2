/**
 * Test for CORS 'Access-Control-Allow-Origin'
 *
 * Verifies the following:
 * - server responds with header "Access-Control-Allow-Origin: <origin>" => allow
 * - server responds with header "Access-Control-Allow-Origin: *" => allow
 * - server responds with header "Access-Control-Allow-Origin: foo.bar" => block
 * - server responds with no Access-Control-Allow-Origin header => block
 * - server responds with header "Access-Control-Allow-Origin:" => block
 * - server responds with header "Access-Control-Allow-Origin: null" => allow
 * - a request to the same origin does not block
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
        var req = moduleFn(url, function (res) {
          if (res.socket || req.socket || req.connection) {
            resolve("FAILURE: able to get socket");
          }
          if (allow) {
            resolveAfterDelay();
          } else {
            resolve("FAILURE: able to read response");
          }
        });
        req.on('response', function (res) {
          if (res.socket || req.socket || req.connection) {
            resolve("FAILURE: able to get socket");
          }
          if (allow) {
            resolveAfterDelay();
          } else {
            resolve("FAILURE: able to read response");
          }
        });
        req.on('socket', function (socket) {
          if (socket || req.socket || req.connection) {
            resolve("FAILURE: able to get socket");
          }
        });
        req.on('error', function (e) {
          if (!(e.message.indexOf("CORS block") === 0 && req.blocked === true)) {
            resolve("FAILURE: received error but it is not related to CORS: " + e);
          }
        });
        req.setTimeout(10000, function () {
          resolve("FAILURE: socket time out");
        });
        req.on('blocked', function (e) {
          if (e.message.indexOf("Permissions block") === 0 && req.blocked === true) {
            resolve("FAILURE: received 'Permissions block'. Incorrect setup!");
          } else if (e.message.indexOf("CORS block") === 0 && req.blocked === true) {
            if (allow) {
              resolve("FAILURE: received 'CORS block' while should not be blocked");
            } else {
              resolveAfterDelay();
            }
          } else {
            resolve("FAILURE: received 'blocked' event but of wrong type");
          }
        });
        var originValue = req.getHeader('Origin');
        try { req.setHeader('Origin', "blabla"); } catch (ignore) {}
        try { req.removeHeader('Origin'); } catch (ignore) {}
        try { req.removeHeader('origin'); } catch (ignore) {}
        if (req.getHeader('Origin') !== originValue) {
          resolve("FAILURE: was able to modify the Origin header");
        }
        if (req.socket || req.connection) {
          resolve("FAILURE: able to get socket");
        }
        req.end();
      });
    };
  }

  // Figure out whether the request will be blocked or not for this origin
  var no_origin = true;
  var path = px.getPackageBaseFilePath();
  if (typeof path === 'string') {
    var urlObject = imports.url.parse(path);
    no_origin = !urlObject.host;
  }

  module.exports.tests = {
    test_origin: make_url_test('https://server.test-cors.org/server?id=6577145&enable=true&status=200&credentials=false', true),
    test_anonymous: make_url_test('https://server.test-cors.org/server?id=67633&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20*', true),
    test_foobar: make_url_test('https://server.test-cors.org/server?id=646102&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20foo.bar', no_origin),
    test_no_header: make_url_test('https://server.test-cors.org/server?id=9732930&enable=false&status=200&credentials=false', no_origin),
    test_empty: make_url_test('https://server.test-cors.org/server?id=8470626&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A', no_origin),
    test_null: make_url_test('https://server.test-cors.org/server?id=8470626&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20null', true),
  };

  if (path.indexOf("http://") === 0 || path.indexOf("https://") === 0) {
    module.exports.tests.test_same_origin = make_url_test(path, true);
  }
});
