/**
 * Simultaneous import of the same module.
 * If you don't see "TypeError: Cannot read property '1' of undefined" or
 * "TypeError: _this.appSandbox.importTracking[bPath].push is not a function" in log then everything is fine
 */

let numImports = 0;

px.import({ https: 'https' }).then((imports) => {
  if (typeof imports.https.request === 'function') ++numImports;
});

px.import({ https: 'https', http: 'http' }).then((imports) => {
  if (typeof imports.https.request === 'function') ++numImports;
  if (typeof imports.http.request === 'function') ++numImports;
});

px.import({scene:"px:scene.1.js", https:"https"}).then( function ready(imports) {
  if (typeof imports.https.request === 'function') ++numImports;
  px.import({https:'https'}).then((imports) => {
    if (typeof imports.https.request === 'function') ++numImports;
  });
  px.import({https:'https'}).then((imports) => {
    if (typeof imports.https.request === 'function') ++numImports;
  });
});

px.import('http').then( function ready(imports) {
  if (typeof imports.request === 'function') ++numImports;
  px.import('http').then((imports) => {
    if (typeof imports.request === 'function') ++numImports;
  });
  px.import('http').then((imports) => {
    if (typeof imports.request === 'function') ++numImports;
  });
});

px.import({assert: '../test-run/assert.js'}).then((imports) => {
  if (typeof imports.assert.assert === 'function') ++numImports;
});
px.import({manual: '../test-run/tools_manualTests.js', assert: '../test-run/assert.js'}).then((imports) => {
  if (typeof imports.manual.getManualTestValue === 'function') ++numImports;
  if (typeof imports.assert.assert === 'function') ++numImports;
});

px.import({assert: '../test-run/assert.js', manual: '../test-run/tools_manualTests.js'})
.then((imports) => {
  module.exports.tests = {
    test1: () => {
      return new Promise((resolve) => {
        setTimeout(() => {
          resolve(imports.assert.assert(numImports === 12, 'simultaneous imports failed'));
        }, 1000);
      });
    }
  };
  if (imports.manual.getManualTestValue() === true) {
    imports.manual.runTestsManually(module.exports.tests, module.exports.beforeStart);
  }
});

