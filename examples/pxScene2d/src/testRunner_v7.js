"use strict";

px.import("px:scene.1.js").then( function ready(scene) {

var root = scene.root;

var basePackageUri = px.getPackageBaseFilePath();

// Info about tests to be run
var testUrls = []; // See tests.json for test urls to be run

var testsFileNameParam = basePackageUri+"/tests.json"; // Change name to run custom set of tests
if( px.appQueryParams.tests !== undefined) {
  testsFileNameParam = px.appQueryParams.tests;
}
// Note that test urls should be relative to basePackageUri.  If a test
// url is using full url and not relative to basePackagerUri, json should
// have  "useBaseURI":"false" for that test row.
// If no test api are present for the url being tested, a timeout value 
// can be used, instead... or in addition to.  Add  "timeToRun":"120000",
// where the value should be the timeout in milliseconds for the scene 
// to be displayed. In the example, it is set to 2 minutes.

// resultsDisplay is for displaying results on screen when all tests have completed.
var resultsDisplay;
var resultsPageUrl = basePackageUri+"/results.js";

// numUrls and lastSceneIndex are set after the testUrls file is loaded
var numUrls;
var lastSceneIndex;
var savedIndexForTimeout;

var prevTest;
var runnerResults = {};

var timeouts = [];
var continueProgressAnimation = true;

// Widgets for displaying "Running tests... " on screen
var mainPage = scene.create({t:"textBox", parent:root, text:"Running tests...",pixelSize:25,w:400,h:50,textColor:0x3090ccff,x: 25, y: 25});
var mainPageProgress1 = scene.create({t:"text", parent:mainPage,pixelSize:25,text:".", a:0,textColor:0x3090ccff});
var mainPageProgress2 = scene.create({t:"text", parent:mainPage,pixelSize:25,text:".", a:0,textColor:0x3090ccff});
var mainPageProgress3 = scene.create({t:"text", parent:mainPage,pixelSize:25,text:".", a:0,textColor:0x3090ccff});
var mainPageProgress4 = scene.create({t:"text", parent:mainPage,pixelSize:25,text:".", a:0,textColor:0x3090ccff});

/** This function is called for each test in testUrls "tests" 
 * 
 *  The test function is expected to return a promise with text of either
 *  SUCCESS of FAILURE for logging.
 */
var runSceneTests = function (testScene, sceneIndex, testNum, tests, cancelToken) {
  return new Promise(function(fulfill, reject) {
    var tmpCancelToken = cancelToken;
    if( testNum < tests.length) {
      var testName = testUrls[sceneIndex].title + "." + tests[testNum];
      console.log("[" + new Date() + "] "+testName);
      try {
        testScene.api.tests[tests[testNum]]().then(function (message) {
          runnerResults[testName] = message;
          if(tmpCancelToken == undefined || tmpCancelToken.cancel != true)
            runSceneTests(testScene, sceneIndex, ++testNum, tests, tmpCancelToken).then(fulfill, reject);
          else 
            fulfill();

        }, function (error) {
          // test rejection
          error = "" + error;
          if (error.indexOf("FAILURE") !== 0) error = "FAILURE: " + error;
          runnerResults[testName] = error;
          if(tmpCancelToken == undefined || tmpCancelToken.cancel != true)
            runSceneTests(testScene, sceneIndex, ++testNum, tests, tmpCancelToken).then(fulfill, reject);
          else 
            fulfill();
        }).catch(reject); // just to be sure
      } catch (error) {
        // test throws
        error = "" + error;
        if (error.indexOf("FAILURE") !== 0) error = "FAILURE: " + error;
        runnerResults[testName] = error;
        if(tmpCancelToken == undefined || tmpCancelToken.cancel != true)
            runSceneTests(testScene, sceneIndex, ++testNum, tests, tmpCancelToken).then(fulfill, reject);
        else 
            fulfill();
      }
    } else {
      fulfill();
    }
  });
};

var iterateTests = function( sceneToBeTested, sceneIndex, cancelToken) {
    // get the names of all exported tests for this scene
    if( sceneToBeTested.api !== undefined && sceneToBeTested.api.tests !== undefined) {
      var tests = Object.keys(sceneToBeTested.api.tests);
      //console.log("TESTS ARE "+tests.length);
      var numTests = tests.length;
      //console.log("numTests for scene "+sceneIndex+" is "+numTests);
      // Run the individual tests for this scene....
      if( numTests > 0) {
          runSceneTests(sceneToBeTested, sceneIndex, 0, tests, cancelToken).then(function(o) {
            if(cancelToken == undefined || cancelToken.cancel != true) {
              //console.log("Returned from runSceneTests!");
              runTests(sceneIndex+1);
              }
          });
          // not handling errors here as runSceneTests is not expected to reject
      }
    }

  
}


/** This is the main work horse for tests.  It is called recursively for
 *  each row in testUrls. It will call the preTest function for the 
 *  url under test, then once the promise is received from the preTest
 *  function, it will invoke runSceneTests.
 */
 
//var timeoutScene = null;

var runTests = function( i) {  

  if( i-1 >=0 && timeouts[i-1] != undefined) {
    console.log("[" + new Date() + "] >>>>>>>>>>>CLEARING HANG TIMEOUT FOR TESTS "+testUrls[i-1].url);
    clearTimeout(timeouts[i-1]);
    timeouts[i-1] = undefined;
  }
    
  //  for( var i = 0; i < numUrls; i ++) {
  if( i < numUrls) {
      //console.log("In loop for i="+i);
      var useBaseURI = testUrls[i].useBaseURI;
      //console.log("baseURI is "+useBaseURI);
      var url = (useBaseURI === undefined || useBaseURI == "false")? (testUrls[i].url) : (basePackageUri+"/"+testUrls[i].url) ;
      var testName = testUrls[i].title;
      console.log("[" + new Date() + "] Creating new scene for test "+testName+" url=\'"+url+"\'");

      // Create the test page - always pass in manualTest=0 to override default of manual test
      var manualString = (url.indexOf("?") == -1) ? "?manualTest=0"  : "&manualTest=0";
      
      var test  = scene.create({t:'scene',parent:root,url:url+manualString,x:0, y:0, w:scene.w, h:scene.h});

          var myIndex = i;
          var testScene = test;
          var prevTestCopy = prevTest;

          //set a timeout to fire in case we never get the scene promise back
          console.log("[" + new Date() + "] test \""+ testName +"\" >>>> SET TIMEOUT FOR SCENE LOAD FOR "+url);
          timeouts[myIndex] = setTimeout( function() {
            console.log("[" + new Date() + "] test \""+ testName +"\" >>>> LOAD FOR "+url+" TIMED OUT WITH NO PROMISE!");
            runnerResults[testUrls[myIndex].title+".HANGING"]= "FAILURE AND CANCEL TESTS";
            testScene = null;
            // Kill test Runner and put up failure results by setting index too high
            runTests(lastSceneIndex);
          },600000);

          testScene.ready.then(function() {
            console.log("Get ready for testScene "+url);
            console.log("[" + new Date() + "] test \""+ testName +"\" is ready. url="+url);
            console.log(">>>> CLEAR TIMEOUT FOR SCENE LOAD FOR "+url);
            clearTimeout(timeouts[myIndex]);
            timeouts[myIndex] = undefined;
            if( prevTestCopy !== undefined && prevTestCopy != null) {
              //prevTestCopy.a = 0;
              console.log("[" + new Date() + "] About to remove() scene for url='"+prevTestCopy.url+"'");

              prevTestCopy.remove();
              //prevTestCopy.url = "";
              prevTestCopy = null;
            }
            console.log("set testScene focus for testName "+testName);
            var cancelToken = {};
            testScene.focus = true;
            //console.log("beforeStart is "+testUrls[myIndex].preTest);
            // Run pre-test if well-known name "beforeStart" is present in API
            if(testScene.api["beforeStart"] !== undefined) {
            // call the preTest for this scene
            //if( testUrls[myIndex].preTest !== undefined) {
             // testScene.api[testUrls[myIndex].preTest]().then( function() {
                testScene.api["beforeStart"]().then( function() {
                console.log("Done executing beforeStart for scene "+testName);
                iterateTests(testScene, myIndex,cancelToken);
                
              }, function(){
                console.log("Problem in 'beforeStart' for test "+myIndex);
                runnerResults[testUrls[myIndex].title+".beforeStart.rejection"]= "FAILURE";
                iterateTests(testScene, myIndex,cancelToken);
              });
            } else {
              // Still try to iterateTests even if no PreTests were set
                //console.log("starting tests without preTests for scene "+myIndex);
                iterateTests(testScene, myIndex,cancelToken);
            }

            if(testUrls[myIndex].timeToRun == undefined) {
              console.log("[" + new Date() + "] >>>>>>>>>>>STARTING HANG PREVENTION TIMEOUT for url:"+testUrls[myIndex].url);
              timeouts[myIndex] = setTimeout( function() {
                    if(cancelToken != undefined && cancelToken.done == undefined && cancelToken.cancel == undefined){
                      cancelToken.cancel = true;
                    }
                    console.log("[" + new Date() + "] >>>>>>>>>>>CANCELLING TESTS for "+testUrls[myIndex].url+" due to timeout!<<<<<<<<<")
                    runnerResults[testUrls[myIndex].title+".HANGING"]= "FAILURE AND CANCEL TESTS";
                    testScene = null;
                    // start running next set of tests
                    //runTests(myIndex+1);
                    // TEMPORARILY Kill test Runner and put up failure results by setting index too high
                    runTests(lastSceneIndex);
                }, 300000);
            }
            // Now run for set timeToRun if one is present
            //console.log("myIndex is now "+myIndex);
            if( testUrls[myIndex].timeToRun !== undefined) {
              console.log("[" + new Date() + "] >>>>>>>>>>>>STARTING TIMEOUT for timeToRun test url="+testUrls[myIndex].url);
              savedIndexForTimeout = i;
              //timeoutScene = testScene;
              setTimeout(function() {
                console.log("[" + new Date() + "] >>>>>>>>>>>>INSIDE TIMEOUT  for timeToRun test url="+testUrls[savedIndexForTimeout].url);
                //console.log("savedIndexForTimeout: "+savedIndexForTimeout);
                // Update results to show successful timeout for this scene
                runnerResults[testUrls[savedIndexForTimeout].title+".timeout"]= "SUCCESS";
//                testScene.remove();
                //testScene = null;
                //timeoutScene = null;
                // Kick off loading of the next url and its tests after timeout
                //runSceneTests(timeoutScene, savedIndexForTimeout, 0, []).then(function(o) {
                  runTests(savedIndexForTimeout+1);
                //});
                // not handling errors here as runSceneTests is not expected to reject
                },
                (testUrls[savedIndexForTimeout].timeToRun));
            }
          },function() {
            console.log("promise failure for test "+myIndex);
            // skip the test that failed to load after logging the error
            runnerResults[testUrls[myIndex].title+".LOAD"]= "FAILURE";
            runTests(myIndex+1);
          }).catch(function(error)
          {
            runnerResults[testUrls[myIndex].title+".FAILURE"]= "FAILURE";
            runTests(myIndex+1);
          });;

      // if(prevTest != null && prevTest != undefined) {
      //   prevTest.remove();
      //   prevTest = null;
      // }
      prevTest = test;

      
    }
    else if(i >= lastSceneIndex) {
      //prevTest.remove();
      prevTest.a = 0;
      //prevTest = null;  
      console.log("Results are \n");
      for(var propertyName in runnerResults) {
        console.log(propertyName+": "+runnerResults[propertyName]+"\n");
         // propertyName is what you want
         // you can get the value like this: myObject[propertyName]
      }
      for( var propertyName in timeouts) {
        if( timeouts[propertyName] != undefined)
          clearTimeout(timeouts[propertyName]);
      }
      // Cleanup
      //testScene.a = 0;
      //testScene = null;
      // Display final results on screen
      resultsDisplay = scene.create({t:"scene",parent:root,url:resultsPageUrl});
      stopProgressAnimation();
      mainPage.a = 0;
      resultsDisplay.ready.then(function() {
        resultsDisplay.api.setResults(runnerResults);
      }, function(error) {
        console.log(">>>>>>> FAILURE to load the testRunner RESULTS page");
        // Print out results detail that CI is looking for
        // TO DO: Externalize this so as not to duplicate this text
        console.log("TEST RESULTS: ");
        console.log("Successes: 0");
        console.log("Failures: 1");
        console.log("Failure Details: \n"+">>>> FAILURE TO LOAD testRunner RESULTS PAGE!  INTERNET CONNECTIVITY ISSUE? <<<<<<<<<<");
      }); 
    }   
}

/** Animate the "..." to indicate that tests are running */
var progressAnimation = function() {
  console.log("-----------------> progressAnimation");
  if( continueProgressAnimation == true) {
    console.log("-----------------> Entering progressAnimation");
    mainPageProgress1.animateTo({a:1},0.8,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,3).then(function() {
      if( continueProgressAnimation != true) {console.log("-----------------> Canceling progressAnimation"); return; }
      console.log("-----------------> Ready after first");
      mainPageProgress2.animateTo({a:1},0.8,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,3).then(function() {
          if( continueProgressAnimation != true) {console.log("-----------------> Canceling progressAnimation"); return; }
          console.log("-----------------> Ready after second");
          mainPageProgress3.animateTo({a:1},0.8,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,3).then(function() {
              if( continueProgressAnimation != true) {console.log("-----------------> Canceling progressAnimation"); return; }
              console.log("-----------------> Ready after third");
              mainPageProgress4.animateTo({a:1},0.8,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,3).then(function() {
                if( continueProgressAnimation != true) {console.log("-----------------> Canceling progressAnimation"); return; }
                console.log("-----------------> Ready after fourth");
                mainPageProgress1.a = 0;
                mainPageProgress2.a = 0;
                mainPageProgress3.a = 0;
                mainPageProgress4.a = 0;
                progressAnimation();
              });
            });
        });
    });
  }
}
var stopProgressAnimation = function() {
    continueProgressAnimation = false;
    mainPageProgress1.a = 0;
    mainPageProgress2.a = 0;
    mainPageProgress3.a = 0;
    mainPageProgress4.a = 0;
}



/** Load the tests.json file to get urls and test info to be run */
  var testsFileName = [];
  testsFileName = testsFileNameParam.split(","); 
  var testsFilePromises = [];

  for( var i in testsFileName) {
    testsFilePromises.push(px.getFile(testsFileName[i]));
  }

  //console.log("loading programsFile at "+programsFileName);
//  var testsToRun = px.getFile(testsFileName);//scene.loadArchive(testsFileName);  
  //console.log("Done with call to programsFile");
//  testsToRun.then(function(data) {
    Promise.all(testsFilePromises).then(function(data) {
    mainPage.ready.then(function(o) {
      var measurement = mainPage.measureText();

      mainPageProgress1.x = measurement.bounds.x2;
      //mainPageProgress1.y = mainPage.y;
      
      var startX = mainPageProgress1.x;
      var ms = mainPage.font.measureText(mainPage.pixelSize, ".");
      
      mainPageProgress2.x = startX+ms.w
      //mainPageProgress2.y = mainPage.y;
      
      mainPageProgress3.x = startX+(ms.w*2);
      //mainPageProgress3.y = mainPage.y;
      
      mainPageProgress4.x = startX+(ms.w*3);
      //mainPageProgress4.y = mainPage.y;
      
      progressAnimation();

      
    });


    var testsToRunText;
    console.log("> In promise from loading programs file <");
    for( var i in data) {
      testUrls = testUrls.concat(JSON.parse(data[i]));
    }
    //var testsToRunText = data;
    //console.log("urls are "+testsToRunText);
    //testUrls = JSON.parse(testsToRunText);
    //console.log("urls are "+testUrls);
    numUrls = testUrls.length;
    lastSceneIndex = numUrls-1;
    // Now, run the tests by starting with index 0
    // Use a timeout to try to put "Running Tests... " on screen briefly
    setTimeout(function() {
        mainPage.a = 0;
        runTests(0);
      }
      ,300);

}, function() {
  console.log("Failed to load tests file");
  mainPage.text = "Failed to load tests file!";
}).catch(function(error) {
  console.log("Error occurred while loading tests file: "+error);
});


  function updateSize(w,h) {

    console.log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> updateSize w="+w+" h="+h);

  }

scene.on("onResize", function(e) { updateSize(e.w,e.h); });

  }).catch( function importFailed(err){
  console.error("Import for testRunner_v5.js failed: " + err)
});
