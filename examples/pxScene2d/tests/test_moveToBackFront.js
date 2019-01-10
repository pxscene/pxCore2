"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           shots:"../test-run/tools_screenshot.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var shots = imports.shots;
var manual = imports.manual;

var doScreenshot = shots.getScreenshotEnabledValue();
var testPlatform=scene.info.build.os;

var manualTest = manual.getManualTestValue();

var basePackageUri = px.getPackageBaseFilePath();

    var urls = [
      basePackageUri+"/images/ball.png",
      basePackageUri+"/../../../images/tiles/001.jpg",
      basePackageUri+"/../../../images/tiles/002.jpg",
      basePackageUri+"/../../../images/tiles/003.jpg",
      basePackageUri+"/../../../images/dolphin.jpg",
    ];
    
var parent = scene.create({t:'object', parent:root, y: 50, clip:false});

var text = scene.create({t:'text', parent:root, clip:false, text:"Dolphin should be on top"});

var ball = scene.create({t:'image', parent:parent,url:urls[0]});
var tile1 = scene.create({t:'image', parent:parent,url:urls[1]});
var tile2 = scene.create({t:'image', parent:parent,url:urls[2]});
var tile3 = scene.create({t:'image', parent:parent,url:urls[3]});
var dolphin = scene.create({t:'image', parent:parent,url:urls[4]});

var noParent = scene.create({t:'image', url:basePackageUri+"/../../../images/tiles/004.jpg"});

var doScreenshotComparison = function(name, expectedScreenshotImage, results, printScreenshotToConsole,resolve, reject) 
{

      shots.validateScreenshot(expectedScreenshotImage, printScreenshotToConsole).then(function(match){
        console.log("test result is match: "+match);
        results.push(assert(match == true, "screenshot comparison for"+name+" failed"));
        resolve(results);
      }).catch(function(err) {
        results.push(assert(false, "screenshot comparison for "+name+" failed due to error: "+err));
        resolve(results);
    });

 
}

var tests = {

  moveToBack: function() {
    
    return new Promise(function(resolve, reject) {

      setTimeout(function() {
        dolphin.moveToBack(); 
        text.text = "Dolphin is on bottom; Cabin Fever should be on top"; 
        var expectedScreenshotImage = basePackageUri+"/images/screenshot_results/"+testPlatform+"/test_moveToBackFront_moveToBack.png";
        var results = [];
        results.push(assert(parent.getChild(1) === ball, "Ball is not in expected position."));
        results.push(assert(parent.getChild(2) === tile1, "Tile1 is not in expected position."))
        results.push(assert(parent.getChild(3) === tile2, "Tile2 is not in expected position."));
        results.push(assert(parent.getChild(4) === tile3, "Tile3 is not in expected position."));
        // Give time for visual verification before resolving promise
        setTimeout(function() {
          results[4] = assert(parent.getChild(0) === dolphin, "Dolphin is not the first child/is not at back.");
          if( doScreenshot) {
            doScreenshotComparison("moveToBack", expectedScreenshotImage, results, false, resolve, reject);
/*            shots.takeScreenshot(false).then(function(link){
              shots.validateScreenshot(expectedScreenshotImage, link).then(function(match){
                console.log("test result is match: "+match);
                results.push(assert(match == true, "screenshot comparison for moveToBack failed"));
                resolve(results);
              });
            });*/
          } else {
            resolve(results);
          }
          }, 3000);
        },2000);
      });

  },
  moveToFront: function() {
      
      return new Promise(function(resolve, reject) {
        ball.moveToFront(); 
        text.text = "Ball should be on top"; 
        var expectedScreenshotImage = basePackageUri+"/images/screenshot_results/"+testPlatform+"/test_moveToBackFront_moveToFront.png";
        var results = [];
        results[0] = assert(parent.getChild(0) === dolphin, "Dolphin is not in expected position.");
        results[1] = assert(parent.getChild(1) === tile1, "Tile1 is not in expected position.");
        results[2] = assert(parent.getChild(2) === tile2, "Tile2 is not in expected position.");
        results[3] = assert(parent.getChild(3) === tile3, "Tile3 is not in expected position.");
        setTimeout(function() {
          results[4] = assert(parent.getChild(parent.numChildren-1) === ball, "Ball is not the last child/is not on top.");
          if( doScreenshot) {
            doScreenshotComparison("moveToFront", expectedScreenshotImage, results, false, resolve, reject);
/*            shots.takeScreenshot(false).then(function(link){
              shots.validateScreenshot(expectedScreenshotImage, link).then(function(match){
                console.log("test result is match: "+match);
                results[5] = assert(match == true, "screenshot comparison for moveToFront failed");
                resolve(results);
              });
            });*/
          } else {
            resolve(results);
          }
        },
        3000);
      });

  },
  
  moveToBackNoParent: function() {
    return new Promise(function(resolve, reject) {
      
        noParent.moveToBack(); 
        text.text = "noParent moveToBack test should not change what's on screen"; 
        var results = [];
        var expectedScreenshotImage = basePackageUri+"/images/screenshot_results/"+testPlatform+"/test_moveToBackFront_moveToBackNoParent.png";
        results[0] = assert(parent.getChild(0) === dolphin, "Dolphin is not in expected position.");
        results[1] = assert(parent.getChild(1) === tile1, "Tile1 is not in expected position.");
        results[2] = assert(parent.getChild(2) === tile2, "Tile2 is not in expected position.");
        results[3] = assert(parent.getChild(3) === tile3, "Tile3 is not in expected position.");
        results[4] = assert(parent.getChild(4) === ball, "Ball is not in expected position.");
        // Give time for visual verification before resolving promise
        setTimeout(function() {
          results[5] = assert(parent.getChild(0) === dolphin, "noParent moveToBack affected children");
          if( doScreenshot) {
            doScreenshotComparison("moveToBackNoParent", expectedScreenshotImage, results, false, resolve, reject);
/*            shots.takeScreenshot(false).then(function(link){
              shots.validateScreenshot(expectedScreenshotImage, link).then(function(match){
                console.log("test result is match: "+match);
                results.push(assert(match == true, "screenshot comparison for moveToBackNoParent failed"));
                resolve(results);
              });
            });*/
          } else {
            resolve(results);
          }
        }, 3000);
      });
  },
  
  moveToFrontNoParent: function() {
    
    return new Promise(function(resolve, reject) {
        noParent.moveToFront(); 
        text.text = "noParent moveToFront test should not change what's on screen";
        var results = [];
        var expectedScreenshotImage = basePackageUri+"/images/screenshot_results/"+testPlatform+"/test_moveToBackFront_moveToFrontNoParent.png";
        results[0] = assert(parent.getChild(0) === dolphin, "Dolphin is not in expected position.");
        results[1] = assert(parent.getChild(1) === tile1, "Tile1 is not in expected position.");
        results[2] = assert(parent.getChild(2) === tile2, "Tile2 is not in expected position.");
        results[3] = assert(parent.getChild(3) === tile3, "Tile3 is not in expected position.");
        results[4] = assert(parent.getChild(4) === ball, "Ball is not in expected position.");
          setTimeout(function() {
            results[5] = assert(parent.getChild(parent.numChildren-1) === ball, "noParent moveToFront affected children");
            if( doScreenshot) {
              doScreenshotComparison("moveToFrontNoParent", expectedScreenshotImage, results, false, resolve, reject);
/*              shots.takeScreenshot(false).then(function(link){
                shots.validateScreenshot(expectedScreenshotImage, link).then(function(match){
                  console.log("test result is match: "+match);
                  results[6] = assert(match == true, "screenshot comparison for moveToFrontNoParent failed");
                  resolve(results);
                });
              });*/
            } else {
            resolve(results);
          }
            },
            3000);
        });

  },
  
}
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests);

}

}).catch( function importFailed(err){
  console.error("Import for test_moveToBack.js failed: " + err)
});
