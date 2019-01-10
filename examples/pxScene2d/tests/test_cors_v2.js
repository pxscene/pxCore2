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
        var moduleFn = url.indexOf("https") === 0 ? https_module.request : http_module.request;
        var req = moduleFn(url, function (res) {
          if (res.socket || req.socket || req.connection) {
            resolve("FAILURE: able to get socket");
          }
          res.on('data', function () {
            resolve(allow ? "SUCCESS" : "FAILURE: able to read response");
          });
        });
        req.on('response', function (res) {
          if (res.socket || req.socket || req.connection) {
            resolve("FAILURE: able to get socket");
          }
          res.on('data', function () {
            resolve(allow ? "SUCCESS" : "FAILURE: able to read response");
          });
        });
        req.on('socket', function (socket) {
          if (req.socket || req.connection) {
            resolve("FAILURE: able to get socket");
          }
          if (socket && Object.keys(socket).length > 1) {
            resolve("FAILURE: able to get real socket");
          }
        });
        req.on('error', function (e) {
          if (!(e.message.indexOf("CORS block") === 0 && req.blocked === true)) {
            resolve("FAILURE: received error but it is not related to CORS");
          }
        });
        req.setTimeout(5000, function () {
          resolve(allow ? "SUCCESS" : "FAILURE: socket time out");
        });
        req.on('blocked', function (e) {
          if (e.message.indexOf("Permissions block") === 0 && req.blocked === true) {
            resolve("FAILURE: received 'Permissions block'. Incorrect setup!");
          } else if (e.message.indexOf("CORS block") === 0 && req.blocked === true) {
            if (allow) {
              resolve("FAILURE: received 'CORS block' while should not be blocked");
            } else {
              setTimeout(function () {
                resolve("SUCCESS");
              }, 2000);
            }
          } else {
            resolve("FAILURE: received 'blocked' event but of wrong type");
          }
        });
        try {
          req.setHeader('Origin', "blabla");
          resolve("FAILURE: was able to set the Origin header");
        } catch (ignore) {
        }
        try {
          req.removeHeader('Origin');
          resolve("FAILURE: was able to remove the Origin header");
        } catch (ignore) {
        }
        try {
          req.removeHeader('origin');
          resolve("FAILURE: was able to remove the Origin header");
        } catch (ignore) {
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
    test_origin: make_url_test('http://server.test-cors.org/server?id=6577145&enable=true&status=200&credentials=false', true),
    test_anonymous: make_url_test('http://server.test-cors.org/server?id=67633&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20*', true),
    test_foobar: make_url_test('http://server.test-cors.org/server?id=646102&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20foo.bar', no_origin),
    test_no_header: make_url_test('http://server.test-cors.org/server?id=9732930&enable=false&status=200&credentials=false', no_origin),
    test_empty: make_url_test('http://server.test-cors.org/server?id=8470626&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A', no_origin),
    test_null: make_url_test('http://server.test-cors.org/server?id=8470626&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20null', true),
  };

  if (path.indexOf("http://") === 0 || path.indexOf("https://") === 0) {
    module.exports.tests.test_same_origin = make_url_test(path, true);
  }
});
