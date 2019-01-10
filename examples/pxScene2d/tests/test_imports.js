/**
 * Imports test.
 * Verifies which modules can be imported (ws, url, querystring, htmlparser, http, https, crypto, oauth, http2)
 * and which cannot be (events, fs, os, net).
 */

px.import({
  scene:"px:scene.1.js",
  assert:"../test-run/assert.js",
  manual:"../test-run/tools_manualTests.js"
})
.then(function(imports) {

  var scene = imports.scene;
  var root = imports.scene.root;
  var assert = imports.assert.assert;
  var manual = imports.manual;
  var manualTest = manual.getManualTestValue();
  var basePath = px.getPackageBaseFilePath();

  module.exports.beforeStart = function() {
    return new Promise(function(resolve, reject) {
      if (scene && root && assert && basePath) {
        resolve();
      } else {
        reject();
      }
    });
  };

  function make_import_test(module, allow) {
    return function () {
      return new Promise(function (resolve) {
        try {
          function onPromiseRejected() {
            resolve(assert(!allow, "module should be enabled"));
          }
          var url = basePath+"/test_import.js?module="+module;
          scene.create({t:"scene",parent:root, url:url}).ready.then(function(js)  {
            if (!js || !js.api || !js.api.ready || !js.api.ready.then) {
              resolve(assert(false, "returned object should be a promise"));
              return;
            }
            js.api.ready.then(function (module) {
              if (!module) {
                resolve(assert(false, "module is null"));
                return;
              }
              resolve(assert(allow, "module should be disabled"));
            }, onPromiseRejected);
          }, onPromiseRejected);
        } catch (e) {
          resolve(assert(false, "exception occurred: " + e));
        }
      })
    };
  }

  module.exports.tests = {
    test_events: make_import_test("events", false),
    test_fs: make_import_test("fs", false),
    test_os: make_import_test("os", false),
    test_net: make_import_test("net", false),
    test_ws: make_import_test("ws", true),
    test_url: make_import_test("url", true),
    test_querystring: make_import_test("querystring", true),
    test_htmlparser: make_import_test("htmlparser", true),
    test_http: make_import_test("http", true),
    test_https: make_import_test("https", true),
    test_crypto: make_import_test("crypto", true),
    test_oauth: make_import_test("oauth", true),
    test_http2: make_import_test("http2", true)
  };

  if (manualTest === true) {
    manual.runTestsManually(module.exports.tests, module.exports.beforeStart);
  }

}).catch(function(err){
  console.error("Import failed: " + err);
});
