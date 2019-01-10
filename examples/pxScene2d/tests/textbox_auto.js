"use strict";


/**
 * This test runs a series of steps over a set of real news stories in var 'textStories'.
 * 
 * The test will display the text in three locations on screen with various pixelSizes.
 * 
 * Usage: 
 * - Run this test with no parameters to cycle through all the titles/text of the stories (there are 4).
 * - Pass in '?index={number}' to run the test over just a single selection of text. This is helpful
 *   in debugging when a particular behavior is only seen with certain text.
 * 
 * */
var textStories = {
    "stories": [ {
      "fullDescription": "This would be some kind of random news story print.  It should wrap within the detail text so that we can test.",
      "imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription": "Wrap This Title Then Give Use Ellipsis for Larger Font",
      "startTimeMillis": 1470757861652,
      "articleText": "This would be some kind of random news story print.  It should wrap within the detail text so that we can test.",
      "type": "Baseball",
      "articleUrl": "http://my.xfinity.com/articles/sports-mlb/20160808/BBN--Ichiro-3000/"
    }, {
      "fullDescription": "The title for this article should be a single line for the first font size, then wrapt to two lines for the larger font size.",
      "imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription": "This Should Be Single Line",
      "startTimeMillis": 1470758277089,
      "articleText": "The title for this article should be a single line for the first font size, then wrapt to two lines for the larger font size.",
      "type": "U.S. News",
      "articleUrl": "http://www.nbcnews.com/politics/2016-election/poll-clinton-opens-double-digit-lead-over-trump-n625676"
    }, {
      "fullDescription" : "The title for this story wraps okay for smaller font size, but truncates last two words with larger font.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Longish Title That Causes Two Truncation Words",
      "startTimeMillis" : 1464712401156,
      "articleText" : "The title for this story wraps okay for smaller font size, but truncates last two words with larger font.",
      "type" : "Sports",
      "articleUrl" : "http://www.nbcnews.com/news/us-news/jack-jakubek-suny-cortland-swim-star-dies-during-lifeguard-test-n582901"
    },{      
       "fullDescription" : "The \"Quotes\" and tick marks were causing trouble because they are double-byte utc characters, so we're testing them here.",
       "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",      
       "shortDescription" : "Test Possessive’s and ‘Tick Marks’ to Validate",      
       "startTimeMillis" : 1473818039656,      
       "articleText" : "The \"Qutoes\" and tick marks were causing trouble because they are double-byte utc characters, so we're testing them here.",      
       "type" : "U.S. News",      
       "articleUrl" : "http://www.nbcnews.com/business/consumer/olive-garden-s-unlimited-pasta-pass-lets-you-gorge-100-n647416"    
  }  
 ]

};


px.import({scene:"px:scene.1.js",
           keys:'px:tools.keys.js',
           assert:"../test-run/assert.js",
           shots:"../test-run/tools_screenshot.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

  var scene = imports.scene;
  var root = scene.root;
  var keys = imports.keys;
  var assert = imports.assert.assert;
  var shots = imports.shots;
  var manual = imports.manual;

  var doScreenshot = shots.getScreenshotEnabledValue();
  var testPlatform=scene.info.build.os;

  var manualTest = manual.getManualTestValue();
  var timeoutForScreenshot = 40;  

  var basePackageUri = px.getPackageBaseFilePath();
  
  // Allow parm to indicate a single story to show in specific position
  console.log("index "+px.appQueryParams.index);

  var titleFont = scene.create({t:"fontResource", url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-Medium.ttf"});
  var bodyFont = scene.create({t:"fontResource", url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTTCond-Medium.ttf"});
  
  var stories = textStories.stories;
  var storyStartIndex = 0;
  var storiesLen = stories.length;
  var storyIndex = storyStartIndex;
  var loopIndex = 0;

  var promise = null;
  var measurements;
  var results = [];

  // Container and widgets for news stories
  var overlayContainerNews = scene.create({t:"object",parent:root,a:1});
  var title = scene.create({t:"textBox",parent:overlayContainerNews,y:10,x:25,h:90,w:400,pixelSize:30,textColor:0xffffffff,
                            wordWrap:true,truncation:scene.truncation.TRUNCATE_AT_WORD, ellipsis:true,
                            font:titleFont});
  var body = scene.create({t:"textBox",parent:overlayContainerNews,x:25,y:95,h:300,w:400,pixelSize:20,wordWrap:true,
                          textColor:0xffffffff, clip:true, truncation:scene.truncation.TRUNCATE_AT_WORD,
                          font:bodyFont});

  var expectedTextDesc = [
    ["bounds", "x1"], 
    ["bounds", "y1"], 
    ["bounds", "x2"], 
    ["bounds", "y2"], 
    ["charFirst", "x"], 
    ["charFirst", "y"], 
    ["charLast", "x"], 
    ["charLast", "y"]
    
  ];
  var expectedScreenshotImages = [
    [basePackageUri+"/images/screenshot_results/"+testPlatform+"/textbox_auto_0_0.png",basePackageUri+"/images/screenshot_results/"+testPlatform+"/textbox_auto_0_1.png"],
    [basePackageUri+"/images/screenshot_results/"+testPlatform+"/textbox_auto_1_0.png",basePackageUri+"/images/screenshot_results/"+testPlatform+"/textbox_auto_1_1.png"],
    [basePackageUri+"/images/screenshot_results/"+testPlatform+"/textbox_auto_2_0.png",basePackageUri+"/images/screenshot_results/"+testPlatform+"/textbox_auto_2_1.png"],
    [basePackageUri+"/images/screenshot_results/"+testPlatform+"/textbox_auto_3_0.png",basePackageUri+"/images/screenshot_results/"+testPlatform+"/textbox_auto_3_1.png"]
  ];
  var expectedValuesMeasure = [
    // bounds.x1, bounds.y1, bounds.x2, bounds.y2, charFirst.x, charFirst.y, charLast.x, charLast.y
    [[25,10,391,82,25,39,366,65],[25,10,367,94,25,44,364,76]], // test0
    [[25,10,385,46,25,39,360,29],[25,10,385,94,25,44,70,76]], // test1
    [[25,10,382,82,25,39,307,65],[25,10,325,94,25,44,233,76]], // test2
    [[25,10,342,82,25,39,317,65],[25,10,400,94,25,44,375,76]] // test3


  ];
  
  var textMeasurementResults = function(values) {
    var results = [];
    var numResults = values.length;
    for( var i = 0; i < numResults; i++) {
      
      console.log("in loop: "+expectedTextDesc[i][0]);
      results[i] = assert(measurements[expectedTextDesc[i][0]][expectedTextDesc[i][1]] === values[i], "measurements "+expectedTextDesc[i][0]+"."+expectedTextDesc[i][1]+" should be "+values[i]+" but is "+measurements[expectedTextDesc[i][0]][expectedTextDesc[i][1]]);
    }
    return results;
  }


var walkStories = function(resolve) {
      console.log( "stories.length="+storiesLen);
      if(storyIndex < storiesLen) {
        title.text = stories[storyIndex].shortDescription;
        body.text = stories[storyIndex].fullDescription;
        overlayContainerNews.animateTo({a:1},0.5,scene.animation.TWEEN_LINEAR,scene.animation.LOOP,1).
          then(function(c) {
                  measurements = title.measureText();
                  console.log("storyIndex "+storyIndex+" loopIndex "+loopIndex);
                  results.push(textMeasurementResults(expectedValuesMeasure[storyIndex][loopIndex]));
                  console.log("************************ position "+loopIndex+" ********************");
                    console.log("measurements boundsX1="+measurements.bounds.x1);
                    console.log("measurements boundsY1="+measurements.bounds.y1);
                    console.log("measurements boundsX2="+measurements.bounds.x2);
                    console.log("measurements boundsY2="+measurements.bounds.y2);
                    console.log("measurements charFirstX="+measurements.charFirst.x);
                    console.log("measurements charFirstY="+measurements.charFirst.y);
                    console.log("measurements charLastX="+measurements.charLast.x);
                    console.log("measurements charLastY="+measurements.charLast.y);
 
            setTimeout( function() {
              if(doScreenshot) {
                shots.validateScreenshot(expectedScreenshotImages[storyIndex][loopIndex],false)
                  .then(function(match){
                    console.log("test result is match: "+match);
                    results.push(assert(match == true, "screenshot comparison for test_y_coords failed"));
                    storyIndex++;
                    overlayContainerNews.animateTo({a:0},0.5,scene.animation.TWEEN_LINEAR,scene.animation.LOOP,1).
                      then(function(d) {
                        walkStories(resolve);});
                          //resolve(results);
                }).catch(function(err) {
                    results.push(assert(false, "screenshot comparison for test_y_coords failed due to error: "+err));
                    //resolve(results);
                    storyIndex++;
                    overlayContainerNews.animateTo({a:0},0.5,scene.animation.TWEEN_LINEAR,scene.animation.LOOP,1).
                      then(function(d) {
                        walkStories(resolve); });
                });                 
              }
              else {
               storyIndex++;
               overlayContainerNews.animateTo({a:0},0.5,scene.animation.TWEEN_LINEAR,scene.animation.LOOP,1).
                then(function(d) {
                  walkStories(resolve);
                });               
              }

            }, timeoutForScreenshot);
/*            setTimeout(function() {
              overlayContainerNews.animateTo({a:0},0.5,scene.animation.TWEEN_LINEAR,scene.animation.LOOP,1).
                then(function(d) {

                  walkStories(resolve);
                });
              },500);*/
          });
      } else if( loopIndex === 0) {
        console.log("loopIndex is 0");
        loopIndex = 1;
        storyIndex = storyStartIndex;
        overlayContainerNews.x = 100;
        overlayContainerNews.y = 200;
        title.pixelSize = 35;
        body.pixelSize = 25;
        body.clip = false;
        walkStories(resolve);
      } else {
        console.log("DONE");
        resolve();

        console.log("resolved promise");
        
      }
}
  

  var doIt = function(startPromise) {
    console.log("inside doIt");
    if( promise == null) {
      console.log("promise is null");
      var promise = new Promise(function(resolve,reject) {
        walkStories(resolve);
      });
      return promise;
    }
  } 


var resetValues = function() {
    results = [];
    loopIndex = 0;
    title.pixelSize = 30;
    body.pixelSize = 20;
    body.clip = true;
    overlayContainerNews.x =0; 
    overlayContainerNews.y = 0;
}

var tests = {
  
  test0: function() {
    resetValues();
    console.log("textbox.js test0");
    var testPromise = new Promise(function(resolve, reject) {
      
      storyStartIndex = 0;
      storyIndex = storyStartIndex;
      storiesLen = storyStartIndex+1;
console.log("calling doIt!");
      doIt(testPromise).then(function(promise) {
        console.log("doIt promise has resolved");
        resolve(results);
      },function() {
        results.push(assert(false,"Promise rejection occurred"))
      });
    
  });
  return testPromise;
},
  test1:function() {
    resetValues();
  console.log("textbox.js test1");
    var testPromise = new Promise(function(resolve, reject) {
      
      storyStartIndex = 1;
      storyIndex = storyStartIndex;
      storiesLen = storyStartIndex+1;
console.log("calling doIt!");
      doIt(testPromise).then(function(promise) {
        console.log("doIt promise has resolved");
        resolve(results);
      });
    
  });
  return testPromise;
},
  test2:function() {
    resetValues();
  console.log("textbox.js test2");
    var testPromise = new Promise(function(resolve, reject) {
      
      storyStartIndex = 2;
      storyIndex = storyStartIndex;
      storiesLen = storyStartIndex+1;
console.log("calling doIt!");
      doIt(testPromise).then(function(promise) {
        console.log("doIt promise has resolved");
        resolve(results);
      });
    
  });
  return testPromise;
},
  test3:function() {
    resetValues();
  console.log("textbox.js test3");
    var testPromise = new Promise(function(resolve, reject) {
      
      storyStartIndex = 3;
      storyIndex = storyStartIndex;
      storiesLen = storyStartIndex+1;
console.log("calling doIt!");
      doIt(testPromise).then(function(promise) {
        console.log("doIt promise has resolved");
        resolve(results);
      });
    
  });
  return testPromise;
} 
}



var beforeStart = function() {
  console.log("textbox.js beforeStart()!");

  var startPromise = new Promise(function(resolve,reject) {
 
    var results = [];
    // Make sure both fonts are loaded before beginning
    titleFont.ready.then(function(a) {
      console.log("titleFont is ready");
      results.push(assert(true, "titleFont is ready"))
    }, function() { 
      console.log("titleFont download FAILED ");
      results.push(assert(false,"Font load failure occurred for titleFont"));
    }).then(function() {
      bodyFont.ready.then(function(b) {
        console.log("bodyFont is ready");
        results.push(assert(true, "bodyFont is ready"))

      }, function() {
        console.log("bodyFont download FAILED ");
        resolve(assert(false,"Font load failure occurred for bodyFont"));
      });
    }).then(function() {
      resolve(results);
    });
  });
  
  return startPromise;
 }

module.exports.tests = tests;       
module.exports.beforeStart = beforeStart;

if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}             

}).catch( function importFailed(err){
  console.error("Imports failed for screensaver.js: " + err)
});
