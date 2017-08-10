// testrunner file without main testrunner tests to minimize time
"use strict";

px.import("px:scene.1.js").then( function ready(scene) {

var root = scene.root;

console.log("Starting testRunner_memcheck.js...");

// Info about tests to be run
var testUrls = ["http://pxscene.org/examples/px-reference/gallery/fancy.js",
                "http://pxscene.org/examples/px-reference/gallery/picturepile.js",
                "http://pxscene.org/examples/px-reference/gallery/gallery.js"
                ];

//added different timeout for different pages, as travis server is bit slower and so getting more timeout errors for complex js pages
var testTimeouts = [5000, 5000, 20000];
var numUrls = testUrls.length;
var lastSceneIndex = numUrls-1;
var savedIndexForTimeout;

var prevTest;

// Widgets for displaying "Running tests... " on screen
var mainPage = scene.create({t:"textBox", parent:root, text:"Running tests...",pixelSize:25,w:400,h:50,textColor:0x3090ccff,x: 25, y: 25});
var mainPageProgress1 = scene.create({t:"text", parent:mainPage,pixelSize:25,text:".", a:0,textColor:0x3090ccff});
var mainPageProgress2 = scene.create({t:"text", parent:mainPage,pixelSize:25,text:".", a:0,textColor:0x3090ccff});
var mainPageProgress3 = scene.create({t:"text", parent:mainPage,pixelSize:25,text:".", a:0,textColor:0x3090ccff});
var mainPageProgress4 = scene.create({t:"text", parent:mainPage,pixelSize:25,text:".", a:0,textColor:0x3090ccff});

var runTests = function( i) {  
  //  for( var i = 0; i < numUrls; i ++) {
    console.log("runTests() for index "+i);
  if( i < numUrls) {
      var url = testUrls[i];
      if( prevTest != undefined) {
        scene.logDebugMetrics(); 
        prevTest.a = 0;
        prevTest.url = "";
        prevTest = null;
        scene.logDebugMetrics(); 
      }
      var test  = scene.create({t:'scene',parent:mainPage,url:url,x:100, y:100, w:scene.w, h:scene.h});
      
      var sceneReady = new Promise(
        function(fulfill,reject) {
          var myIndex = i;
          var testScene = test;
          Promise.all([testScene.ready,sceneReady]).then(function() {
            console.log("test "+ myIndex +" is ready");
            testScene.focus = true;
              console.log(">>>>>>>>>>>>STARTING TIMEOUT");
              savedIndexForTimeout = i;
              setTimeout(function() {
                console.log(">>>>>>>>>>>>INSIDE TIMEOUT");
                if (savedIndexForTimeout+1 == numUrls)
                {
                  test.a = 0;
                  test.url = "";
                  stopProgressAnimation();
                  mainPage.a = 0;
                  console.log("RUN COMPLETED");
                }
                else
                {
                  runTests(savedIndexForTimeout+1);
                }
              },testTimeouts[i]);
          },function() {
            console.log("promise failure for test "+myIndex);
          });
      });
      prevTest = test;
    }
}

/** Animate the "..." to indicate that tests are running */
var progressAnimation = function() {
  
  //console.log("-----------------> Entering progressAnimation");
  mainPageProgress1.animateTo({a:1},0.8,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,3).then(function() {
    //console.log("-----------------> Ready after first");
    mainPageProgress2.animateTo({a:1},0.8,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,3).then(function() {
      //console.log("-----------------> Ready after second");
        mainPageProgress3.animateTo({a:1},0.8,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,3).then(function() {
        //  console.log("-----------------> Ready after third");
            mainPageProgress4.animateTo({a:1},0.8,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,3).then(function() {
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
var stopProgressAnimation = function() {
    mainPageProgress1.a = 0;
    mainPageProgress2.a = 0;
    mainPageProgress3.a = 0;
    mainPageProgress4.a = 0;
}

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

// Now, run the tests by starting with index 0
runTests(0);

function updateSize(w,h) {

  console.log(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> updateSize w="+w+" h="+h);

}

scene.on("onResize", function(e) { updateSize(e.w,e.h); });

  }).catch( function importFailed(err){
  console.error("Import for tests.js failed: " + err)
});
