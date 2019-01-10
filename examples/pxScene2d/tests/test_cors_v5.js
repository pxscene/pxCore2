/**
 * Test for CORS's 'Access-Control-Allow-Origin'
 * and 'Access-Control-Allow-Credentials'
 */

px.import({
  scene: 'px:scene.1.js',
  url: 'url',
  http2: 'http2',
  querystring: 'querystring'
})
.then(function(imports) {

  var http2_module = imports.http2;
  var url_module = imports.url;
  var querystring_module = imports.querystring;
  var no_origin = true;
  var path = px.getPackageBaseFilePath();
  if (typeof path === 'string') {
    var urlObject = url_module.parse(path);
    no_origin = !urlObject.host;
  }

  function make_test(name, url, withCredentials, expectAllow) {
    return {
      name: name,
      fn : function () {
        return new Promise(function (resolve) {
          function resolveAfterDelay() {
            setTimeout(function () {
              resolve('SUCCESS');
            }, 2000);
          }
          function resolveAsFailure(details) {
            resolve('FAILURE: ' + details);
          }
          var onResponse = function (res) {
            if (res.socket || req.socket || req.connection) {
              resolveAsFailure('private api access (req/resp socket)');
            }
            var read = 0;
            res.on('data', function (c) {
              read += c.length;
            });
            res.on('end', function () {
              if (read > 0) {
                // did read something
                if (expectAllow) {
                  resolveAfterDelay();
                } else {
                  resolveAsFailure('able to read ' + read + ' bytes');
                }
              } else {
                // did not read anything
                if (res.headers['content-length'] === 0) {
                  resolveAsFailure('no Origin sent');
                } else {
                  if (!expectAllow) {
                    resolveAfterDelay();
                  } else {
                    resolveAsFailure('did not read');
                  }
                }
              }
            });
          };
          if (withCredentials) {
            url = url_module.parse(url);
            url.headers = {
              cookie: "__utma=11676977.838379967.1534154213.1534154213.1534154213.1;"
            };
          }
          var req = http2_module.request(url, onResponse);
          req.on('response', onResponse);
          req.on('socket', function (socket) {
            if (socket || req.socket || req.connection) {
              resolveAsFailure('private api access (req. socket)');
            }
          });
          req.on('error', function (e) {
            if (!(e.message.indexOf('CORS block') === 0 && req.blocked)) {
              resolveAsFailure('unexpected error: ' + e);
            }
          });
          req.setTimeout(10000, function () {
            resolveAsFailure('socket time out');
          });
          req.on('blocked', function (e) {
            if (e.message.indexOf('Permissions block') === 0 && req.blocked) {
              resolveAsFailure('permissions block');
            } else if (e.message.indexOf('CORS block') === 0 && req.blocked) {
              if (expectAllow) {
                resolveAsFailure('should not be blocked');
              } else {
                resolveAfterDelay();
              }
            } else {
              resolveAsFailure('unknown block: ' + e);
            }
          });
          if (req.setHeader || req.removeHeader) {
            resolveAsFailure('private api access (headers)');
          }
          if (req.socket || req.connection) {
            resolveAsFailure('private api access (socket)');
          }
          req.end();
        });
      }
    };
  }

  function make_url(enable, credentials, origin) {
    var url = 'https://server.test-cors.org/server?id=8470626';
    url += '&enable=' + enable;
    url += '&status=200';
    url += '&credentials=' + credentials;
    if (typeof origin === 'string') {
      var headers = 'Access-Control-Allow-Origin: ' + origin;
      url += '&response_headers=' + querystring_module.escape(headers);
    }
    return url;
  }

  var test;
  module.exports.tests = {};
  test = make_test('allow', make_url(true, false, false), false, true);
  module.exports.tests[test.name] = test.fn;
  test = make_test('allow *', make_url(true, false, '*'), false, true);
  module.exports.tests[test.name] = test.fn;
  test = make_test('allow null', make_url(true, false, 'null'), false, true);
  module.exports.tests[test.name] = test.fn;
  test = make_test('disallow foo.bar', make_url(true, false, 'foo.bar'), false, no_origin);
  module.exports.tests[test.name] = test.fn;
  test = make_test('disallow empty', make_url(true, false, ''), false, no_origin);
  module.exports.tests[test.name] = test.fn;
  test = make_test('disallow no headers', make_url(false, false), false, no_origin);
  module.exports.tests[test.name] = test.fn;
  test = make_test('allow with credentials', make_url(true, true, false), true, true);
  module.exports.tests[test.name] = test.fn;
  test = make_test('disallow with credentials', make_url(true, false, false), true, false);
  module.exports.tests[test.name] = test.fn;
  test = make_test('allow without credentials', make_url(true, false, false), false, true);
  module.exports.tests[test.name] = test.fn;
  if (!no_origin) {
    test = make_test('allow same origin', path, false, true);
    module.exports.tests[test.name] = test.fn;
  }
});
