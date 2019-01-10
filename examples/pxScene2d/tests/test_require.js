"use strict";
px.import({scene:"px:scene.1.js"
          }).then( function ready(imports) {

/** To test require api */
function test_require() {
    var testHttp = require('http');
    var testHttps = require('https');
    var testHtmlParser = require('htmlparser');
}

test_require();

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for test_require.js failed: " + err)
});
