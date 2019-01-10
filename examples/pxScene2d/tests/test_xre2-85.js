"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           shots:"../test-run/tools_screenshot.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var shots = imports.shots;
var manual = imports.manual;

var doScreenshot = shots.getScreenshotEnabledValue();
var testPlatform=scene.info.build.os;

var manualTest = manual.getManualTestValue();
var timeoutForScreenshot = 40;

var basePackageUri = px.getPackageBaseFilePath();

var fontUrlStart = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/";
var XFinityMed = "XFINITYSansTT-New-Med.ttf";

var fontXfinityMed = scene.create({t:"fontResource",url:fontUrlStart+XFinityMed});

var container = scene.create({parent:root, t:"object",w:800, h:400, clip:false});
var textBox = scene.create({parent:container, t:"textBox", font:fontXfinityMed, 
              x:20, y:20, w:800, h:300, textColor:0xFFFFFFFF, pixelSize:25,
              text:"Text here and there and there and there",
              clip:false, wordWrap:true});
 //with newline...             
textBox.ready.then(function(obj) {
  console.log("text is ready");
  var measurements = obj.measureText();
  console.log("bounds is "+measurements.bounds.x2);
  
  var metrics = fontXfinityMed.getFontMetrics(obj.pixelSize);
  console.log("metrics width is "+metrics.w);
  
  var fontMeasurements = fontXfinityMed.measureText(obj.pixelSize,obj.text);
  console.log("font measure gives width = "+fontMeasurements.w);
  
  console.log("textBox width is "+obj.w);
});
              
var text = scene.create({parent:container, t:"text",font:fontXfinityMed, x:20,y:100,w:800,h:300,textColor:0xFFFFFFFF, pixelSize:25,
              text:"Text here and there and there and there",
              clip:false}); 

var beforeStart = function() {
  // Nothing to do here...
  console.log("XRE2-85 beforeStart.....");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}

var doScreenshotComparison = function(name, results, printScreenshotToConsole,resolve, reject) 
{

      shots.validateScreenshot(basePackageUri+"/images/screenshot_results/"+testPlatform+"/test_xre2-85_"+name+".png", printScreenshotToConsole).then(function(match){
        console.log("test result is match: "+match);
        results.push(assert(match == true, "screenshot comparison for "+name+" failed"));
        resolve(results);
      }).catch(function(err) {
        results.push(assert(false, "screenshot comparison for "+name+" failed due to error: "+err));
        resolve(results);
    });

 
}
var tests = {
// Test clip false and wordWrap false
  clipWrapFalse: function() {
  textBox.clip = false;
  textBox.wordWrap = false;
  
  return new Promise(function(resolve, reject) {

    textBox.ready.then(function() {
      var results = [];
      var measurements = textBox.measureText();
      
      results.push(assert(measurements.bounds.x2 === 459,"Text bounds "+measurements.bounds.x2+" does not match expected value of 459"));
      
      var fontMeasurements = fontXfinityMed.measureText(textBox.pixelSize,textBox.text);
      results.push(assert(measurements.bounds.x2 === fontMeasurements.w, "Text width from TextBox does not match font measurement width"));
      if( doScreenshot) 
      {
          setTimeout( function() {
            doScreenshotComparison("clipWrapFalse", results, false, resolve, reject);
          }, timeoutForScreenshot);
      } 
      else
        resolve(results);
    });
  });
},

// Test clip false and wordWrap false
  clipFalseWrapTrue: function() {
  
  textBox.clip = false;
  textBox.wordWrap = true;
  
  return new Promise(function(resolve, reject) {

    textBox.ready.then(function() {
      var results = [];
      var measurements = textBox.measureText();
      
      results.push(assert(measurements.bounds.x2 === 459,"Text bounds "+measurements.bounds.x2+" does not match expected value of 459"));
      var fontMeasurements = fontXfinityMed.measureText(textBox.pixelSize,textBox.text);
      results.push(assert(measurements.bounds.x2 === fontMeasurements.w, "Text width from TextBox does not match font measurement width"));
      if( doScreenshot) 
      {
          setTimeout( function() {
            doScreenshotComparison("clipFalseWrapTrue", results, false, resolve, reject);
          }, timeoutForScreenshot);
      } 
      else
        resolve(results);
    });
  });
}
}
module.exports.tests = tests;
module.exports.beforeStart = beforeStart;


if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}

}).catch( function importFailed(err){
  console.error("Import for test_XRE2-85.js failed: " + err)
});
