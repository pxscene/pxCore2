/**
 * Test for CORS 'Access-Control-Allow-Origin'
 *
 * Verifies that HTTP requests succeed when:
 * 1) Origin of this js file matches the value of 'Access-Control-Allow-Origin' response header, or
 * 2) Value of 'Access-Control-Allow-Origin' response header is '*', 'null', or
 * 3) This js file has no Origin (file system or non-http url), or
 * 4) Request is same-origin
 */

px.import({
  scene: 'px:scene.1.js',
  url: 'url',
  http: 'http',
  https: 'https'
})
.then(function(imports) {

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

  function make_request(url, timeout) {
    return new Promise(function(resolve, reject) {
      var moduleFn = url.indexOf("https") === 0 ? imports.https.request : imports.http.request;
      var req = moduleFn(url, resolve);
      if (req) {
        req.setTimeout(timeout, function () {
          reject("request timeout");
        });
        req.on("error", reject);
        req.end();
      } else {
        reject("request is null");
      }
    });
  }

  function make_test(url, allow) {
    return function () {
      return make_request(url, 5000).then(function () {
        return allow ? "SUCCESS" : "FAILURE";
      }, function (why) {
        if (why === "CORS block") {
          return allow ? "FAILURE" : "SUCCESS";
        }
        console.error("Request to '"+url+"' failed unexpectedly: "+why);
        return "SUCCESS";
      });
    };
  }

  module.exports.tests = {
    // Access-Control-Allow-Origin: <origin>
    testA: make_test('http://server.test-cors.org/server?id=6577145&enable=true&status=200&credentials=false', true),
    // Access-Control-Allow-Origin: *
    testB: make_test('http://server.test-cors.org/server?id=67633&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20*', true),
    // Access-Control-Allow-Origin: foo.bar
    testC: make_test('http://server.test-cors.org/server?id=646102&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20foo.bar', no_origin),
    // no Access-Control-Allow-Origin
    testD: make_test('http://server.test-cors.org/server?id=9732930&enable=false&status=200&credentials=false', no_origin),
    // Access-Control-Allow-Origin:
    testE: make_test('http://server.test-cors.org/server?id=8470626&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A', no_origin),
    // Access-Control-Allow-Origin: null
    testG: make_test('http://server.test-cors.org/server?id=8470626&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20null', true),
  };

  if (!no_origin &&
    (this_file_origin.indexOf("http://") == 0 || this_file_origin.indexOf("https://") == 0)) {
    // same-origin
    module.exports.tests.testF = make_test(this_file_origin, true);
  }
});
