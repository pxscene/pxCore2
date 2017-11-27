# testRunner

We use [testRunner](https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js) to run JavaScript tests that exercise pxObjects and the JavaScript wrappers. By default, testRunner will run the tests located here: [tests.json](https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/tests.json).  For our Travis CI builds, however, the tests.json from pxCore/tests/pxScene2d/tests/testRunner/tests.json will be used.

*testRunner* will run all tests defined in tests.json and will report success and failure from those tests.  When tests have completed, it will display a Results page indicating how many tests succeeded and how many failed. If there are failures, the details of those failures will be displayed as a list on screen.  These results will also be output to the console/log.

## Adding a testRunner Test:
1. Create a test JavaScript file according to instructions in [Defining Tests](#defining-test-functions-to-be-run). We suggest creating a pull request to add the test to [pxscene](https://github.com/pxscene/pxscene/tree/gh-pages) for open source availability.
2. Test your test:  Use the manual test mechanism described in [Defining Tests](#defining-test-functions-to-be-run) to run your test and verify the results.
3. Add that test file name and other properties to pxCore/tests/pxScene2d/tests/testRunner/test.json according to the instructions in [Adding a Test Page in tests.json](#Adding-a-test-page-in-tests.json).  Note that the "timeToRun" mechanism should only be used to run pages that are not explicitly authored to be tests. Otherwise, any tests added should export the tests variable and run tests and return results, as described in [Defining Tests](#defining-test-functions-to-be-run).

### Adding a Test Page in tests.json:
A test is defined in tests.json as an object with the following properties: 
{"url":"../tests/simpleTestApi.js", "title":"api_test","preTest":"start"} 
* url: the path to the test js file
  * if the test url is relative to the testRunner.js, specify the relative path as the url property, as in the example
  * if the test url is not relative to the testRunner.js, specify the full url to the js file to test, then set the "useBaseURI" property to false, eg, "useBaseURI":"false"
* title: the title that you would like to appear when testRunner reports results for these tests
Alternatively, you can add a js file to the test run without adding any tests or apis to the js page itself, for instance if you want to ensure that a particular page or app always successfully runs.  You can do this by specifying a different set of properties in the tests.json: 
{"url":"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/gallery/coverflowtest_v2.js", "title":"coverflow", "useBaseURI":"false", "timelimit":"60000"}
* timeToRun: number of milliseconds for this test scene to be left to run.  
* useBaseURI: whether or not the url specified uses a path relative to testRunner.js; if value is "false", the url is expected to be the full path to the js file to load.  Defaults to "false".
TBD:  Key injection mechanisms.

### Defining Test Functions To Be Run:
1. There are several JavaScript files that can be imported into any test that is meant to run in *testRunner*. Their names and functionality are as follows: 
   1. __asserts.js__: Use *assert.js* to properly format the results returned to *testRunner*.  Asserts' assert() function will prepend data with SUCCESS or FAILURE strings that testRunner uses to determine the tests' final status.  If you have a single test that tests multiple conditions for success/failure, return the results as an array of strings accumulated during the test.  The assert function is defined as function(condition, message), where the message will be output along with a prepended "FAILURE" string when the condition is false.
_assert function definition_
```module.exports.assert = function(condition, message) {
  //console.log("Inside px assert with condition: " +condition);
  if( condition === false)
  {
    //console.log("FAILURE : "+message);
    return "FAILURE: "+message;
  } else {
    return "SUCCESS";
  }
}
```
   1. __tools\_manualTests.js__: Use tools\_manualTests.js to allow for easy one-off testing of tests without having to use *testRunner* itself.  With a few additional lines of code, you can run the js test directly and still get the equivalent test results output in the log.

* Example tools\_manualTest usage *
```px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {
 
var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;
 
var manualTest = manual.getManualTestValue(); // <!--- Variable that can be used to turn on manual tests; Call manual.getManualTestValue()
                                              // and its return value will default to "true" if this js test is called directly and not passed a queryParam called "manualTest". -->
  
var tests = {
    test1: function() {...},
    test2: function() {...}
  
}
  
module.exports.tests = tests;
 
if(manualTest === true) {
 
  manual.runTestsManually(tests); // <!--- This is usage of imported manual function to run tests manually. -->
 
}
}).catch( function importFailed(err){
  console.error("Import for test_sample.js failed: " + err)
});
```

   1. __tools\_screenshot.js__: Use tools\_screenshot.js to include screenshot comparisons in your test.  tools\_screenshot.js implements and exports a function called "validateScreenshot" that makes it easy to do a screenshot comparison.  The first parameter to the function is a string to indicate an url to the saved, expected image for the comparision.  The second parameter is a boolean that, if true, will cause the function to output the base64 value of the realtime screen.screenshot function.  This can then be copied and pasted into a base64 decoder (like https://opinionatedgeek.com/Codecs/Base64Decoder) and decoded to the resultant png binary. That can then be saved and used as the expected image for the comparison.

* Example tools\_screenshot usage *
```px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           shots:"../test-run/tools_screenshot.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {
 
var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var shots = imports.shots;
var manual = imports.manual;
 
var doScreenshot = true;
var manualTest = false;
 
var tests = {
    test1: function() {
        return new Promise(function(resolve, reject) {
          // Invoke the imported shots.validateScreenshot function to fetch the expected image from expectedScreenshotImageUrl
          // and compare it to the one captured now while running the test
          shots.validateScreenshot(expectedScreenshotImageUrl, printScreenshotToConsole).then(function(match){
            console.log("test result is match: "+match);
            results.push(assert(match == true, "screenshot comparison for"+name+" failed"));
            resolve(results);
            }).catch(function(err) {
               results.push(assert(false, "screenshot comparison for "+name+" failed due to error: "+err));
               resolve(results);
            });
 
        }
 
    }
}
}).catch( function importFailed(err){
  console.error("Import for test_sample.js failed: " + err)
});
```
1. __beforeStart__: In your test page, if there is any code that needs to be run prior to starting the tests, for instance for setup or downloads, you can implement and export a function called "beforeStart".  This function will be called once before ** begins iterating over and running the exported tests (see step 2).
```var beforeStart = function() {
  // Do startup/pre-test work here ...
  console.log("test_pxConstants start.....");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}
module.exports.beforeStart = beforeStart;
```

1. __tests__ object: In your test page, define and export a "tests" object. Each member of that object will be considered to be a test function and will be run by testRunner. Each test must return a promise.  When that promise is resolved, the data should be a string or array of strings indicating the test results. 
For example:
```var tests = {
   
  myFirstTest: function() {
    // do any setup necessary
    // Use results as an array
    var results  =[];
    // return a promise that is fulfilled when the test is complete
    return new Promise(function(resolve, reject) {
       // do test stuff
        ....
       // Populate results using assert
       results.push(assert(val1 === val2, "Val1 is not equal to Val2!"));
       results.push(assert(widget.x = 45, "widget is not at correct x location"));
       resolve(results);
    });
  },
   
  mySecondTest: function() {
    // do any setup necessary
    // return a promise that is fulfilled when the test is complete
    return new Promise(function(resolve, reject) {
       // do test stuff
       // Use result as just a simple string
       var result = assert(val1 !== val2, "Val1 and Val2 are equivalent"));
       resolve(result);
    });
  }
}
module.exports.tests = tests; // export the tests
```

## Running Tests
Tests can be run either using the checked-in/auto-deployed version of *testRunner* and tests, or by running local versions of *testRunner* and tests: 
__Run deployed versions of *testRunner* and *tests.json*__:
``` ./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js
```

*or* 
__Run local versions of *testRunner* and *tests.json*__:
``` ./pxscene.sh {local_path_to_pxscene-samples}/examples/px-reference/test-run/testRunner.js
```

*or*
__Run deployed *testRunner* with local *tests.json* file via queryParam "tests"__:
``` ./pxscene.sh https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js?tests=file://{local_path_to_a_tests.json_file}
```

