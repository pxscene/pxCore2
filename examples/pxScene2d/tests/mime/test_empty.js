/**
 * empty unit test file
 */
px.import({    scene: "px:scene.1.js",
              assert: "../../test-run/assert.js",
              manual: "../../test-run/tools_manualTests.js"
}).then( function ready(imports)
{
  var assert = imports.assert.assert;
  var manual = imports.manual;
  var manualTest = manual.getManualTestValue();
  
  var tests = {
    empty_test: function() {
      return new Promise(function(resolve, reject) {
        var results = [];
        results.push(assert(true, 'empty test passed!'));
        resolve(results);
      });
    }
  }
  
  module.exports.tests = tests;
  if(manualTest === true) {
    manual.runTestsManually(tests);
  }

}).catch( function importFailed(err){
  console.error("Import failed for test_empty.js: " + err)
});
