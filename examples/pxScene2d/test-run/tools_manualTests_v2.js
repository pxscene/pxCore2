/* Use file.io to capture a screenshot during test, then send link back to caller for verification */
px.import({scene:"px:scene.1.js"}).then(function(imports) {

var scene = imports.scene;
var root = imports.scene.root;

var basePackageUri = px.getPackageBaseFilePath();

var testResults = {};

var runManualTest = function(index, tests) {
  
  var testsToRun = Object.keys(tests);
  var currIndex = index;
  if( currIndex < testsToRun.length) {
    console.log("Running test "+currIndex);
    tests[testsToRun[currIndex]]().then(function(results) {
      testResults[testsToRun[currIndex]] = results;
      runManualTest(currIndex+1, tests);
    },function( results) {
      console.log("promise failure for test "+currIndex);
      testResults[testsToRun[currIndex]] = results;
      runManualTest(currIndex+1, tests);
    }).catch(function (err) {
      console.log("Error on test currIndex: "+currIndex+": "+err);
    });
  }
  else {
    // All done with tests; output the results
    var resultsPageUrl = basePackageUri+"/../test-run/results.js";
    var resultsDisplay = scene.create({t:"scene",parent:root,url:resultsPageUrl,a:0});
    resultsDisplay.ready.then(function() {
      resultsDisplay.api.setResults(testResults);
      //console.log(testResults);
    });   
  }
}

var runTestsManually = function(tests, beforeStart) {

  var beforeStartPromise;
  // Call beforeStart
  if( beforeStart !== undefined) {
    beforeStartPromise = beforeStart();
  } 
  else {
    beforeStartPromise = new Promise(function( resolve, reject) { 
      resolve();
    });
  }

  beforeStartPromise.then(function() {
    var testsToRun = Object.keys(tests);

    if( testsToRun.length > 0) {
      runManualTest(0, tests);
    }
  });
}

var getManualTestValue = function() {
  
  var manualTest = true;
  if( px.appQueryParams.manualTest !== undefined) {
    manualTest = (px.appQueryParams.manualTest==0)?false:true;
  }
  return manualTest;
}
module.exports.getManualTestValue = getManualTestValue;
module.exports.runTestsManually = runTestsManually;

})
.catch(function(err){
  console.error("Imports failed for tools_manualTests: " + err)
});
