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

var container = scene.create({parent:root, t:"object",x:825, y:100,w:800, h:400, clip:false});
var textBoxInContainer = scene.create({parent:container, t:"textBox", font:fontXfinityMed, 
              w:800, h:300, textColor:0xFFFFFFFF, pixelSize:25,
              text:"Test Upper and Low-gery characters",
              clip:false, wordWrap:true});
var textBox = scene.create({parent:root, t:"textBox", font:fontXfinityMed, 
              x:20, y:100, w:800, h:300, textColor:0xFFFFFFFF, pixelSize:25,
              text:"Test Upper and Low-gery characters",
              clip:false, wordWrap:true});
 //with newline...             
textBox.ready.then(function(obj) {
  console.log("text is ready");
  var measurements = obj.measureText();
  console.log("bounds.x2 is "+measurements.bounds.x2);
  console.log("bounds.y1 is "+measurements.bounds.y1);
  console.log("bounds.y2 is "+measurements.bounds.y2);
  console.log("charFirst.y is "+measurements.charFirst.y);
  console.log("charFirst.x is "+measurements.charFirst.x);
  console.log("charLast.y is "+measurements.charLast.y);
  console.log("charLast.x is "+measurements.charLast.x);
  
  var metrics = fontXfinityMed.getFontMetrics(obj.pixelSize);
  console.log("metrics baseline is "+metrics.baseline);
  
  var fontMeasurements = fontXfinityMed.measureText(obj.pixelSize,obj.text);
  console.log("font measure gives width = "+fontMeasurements.w);
  
  console.log("textBox width is "+obj.w);
  // Draw a new rect on baseline
  var rect2 = scene.create({t:"rect",parent:root, h:1, w:1200,x:0,y:(100+metrics.baseline),lineWidth:0.5,lineColor:0xFFFFFFFF});
});

 //with newline...             
textBoxInContainer.ready.then(function(obj) {
  console.log("text is ready");
  var measurements = obj.measureText();
  console.log("textBoxInContainer bounds.x2 is "+measurements.bounds.x2);
  console.log(" textBoxInContainerbounds.y1 is "+measurements.bounds.y1);
  console.log(" textBoxInContainerbounds.y2 is "+measurements.bounds.y2);
  console.log("charFirst.y is "+measurements.charFirst.y);
  console.log("charFirst.x is "+measurements.charFirst.x);
  console.log("charLast.y is "+measurements.charLast.y);
  console.log("charLast.x is "+measurements.charLast.x);
  
  var metrics = fontXfinityMed.getFontMetrics(obj.pixelSize);
  console.log(" textBoxInContainermetrics baseline is "+metrics.baseline);
  
  var fontMeasurements = fontXfinityMed.measureText(obj.pixelSize,obj.text);
  console.log(" textBoxInContainerfont measure gives width = "+fontMeasurements.w);
  
  console.log(" textBoxInContainertextBox width is "+obj.w);

});
              
var text = scene.create({parent:root, t:"text",font:fontXfinityMed, x:425,y:100,w:800,h:300,textColor:0xFFFFFFFF, pixelSize:25,
              text:"Test Upper and Low-gery characters",
              clip:false}); 

// Draw a line at y=100 which should align with the top of the tallest letters              
var rect = scene.create({t:"rect",parent:root, h:1, w:1200,x:0,y:100,lineWidth:0.5,lineColor:0xFFFFFFFF});
            
var finish = function(resolve, results) 
{
  if( doScreenshot) 
  {
    setTimeout( function() {
      shots.validateScreenshot(basePackageUri+"/images/screenshot_results/"+testPlatform+"/test_y_coords.png",false)
        .then(function(match){
          console.log("test result is match: "+match);
          results.push(assert(match == true, "screenshot comparison for test_y_coords failed"));
          resolve(results);
      }).catch(function(err) {
          results.push(assert(false, "screenshot comparison for test_y_coords failed due to error: "+err));
          resolve(results);
      });

    },timeoutForScreenshot);
  }
  else
    resolve(results);
}

var beforeStart = function() {
  // Nothing to do here...
  console.log("test_y_coords start.....");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}

var tests = {

  testyPos1: function() {

  
  return new Promise(function(resolve, reject) {
    
    var results = [];
    var other = false;
    textBox.ready.then(function(obj) {
      
      // Measure text in textbox
      var measurements = obj.measureText();
      results.push(assert(measurements.bounds.x1 == 0, "textBox bounds.x1 is "+measurements.bounds.y1+" but should be 0"));
      results.push(assert(measurements.bounds.x2 == 412, "textBox bounds.x2 is "+measurements.bounds.y1+" but should be 412"));
      results.push(assert(measurements.bounds.y1 == 0, "textBox bounds.y1 is "+measurements.bounds.y1+" but should be 0"));
      results.push(assert(measurements.bounds.y2 == 30, "textBox bounds.y2 is "+measurements.bounds.y1+" but should be 30"));
      results.push(assert(measurements.charFirst.y == 19 || measurements.charFirst.y == 20, "textBox charFirst.y is "+measurements.charFirst.y+" but should be 19 or 20"));
      results.push(assert(measurements.charLast.y == 19 || measurements.charLast.y == 20, "textBox charLast.y is "+measurements.charLast.y+" but should be 19 or 20"));
      results.push(assert(measurements.charFirst.x == 0, "textBox charFirst.y is "+measurements.charFirst.x+" but should be 0"));
      results.push(assert(measurements.charLast.x == 412, "textBox charLast.y is "+measurements.charLast.x+" but should be 412"));
            
      // Test font metrics
      results.push( assert(obj.pixelSize === 25,"textBox.pixelSize does not match expected value of 25: value is "+ obj.pixelSize));
      var metrics = fontXfinityMed.getFontMetrics(obj.pixelSize);
      console.log("metrics baseline is "+metrics.baseline);
      results.push( assert(metrics.baseline === 19 || metrics.baseline === 20,"textBox baseline does not match expected value of 19"));
      
      // Measure text via font
      var fontMeasurements = fontXfinityMed.measureText(obj.pixelSize,obj.text);
      console.log("font measure gives width = "+fontMeasurements.w);
      
      var fontMeasurements = fontXfinityMed.measureText(obj.pixelSize,obj.text);
      results.push( assert(measurements.bounds.x2 === fontMeasurements.w, "Text width from textBox does not match font measurement width"));
      
      // Check textBox width 
      results.push(assert(obj.w === 800, "textBox does not match defined width of 800"));
      if( other === true) 
        finish(resolve, results);
      other = true;
    });
    textBoxInContainer.ready.then(function(obj) {
      
      // Measure text in textbox
      var measurements = obj.measureText();
      results.push(assert(measurements.bounds.x1 == 0, "textboxInContainer bounds.x1 is "+measurements.bounds.y1+" but should be 0"));
      results.push(assert(measurements.bounds.x2 == 412, "textboxInContainer bounds.x2 is "+measurements.bounds.y1+" but should be 412"));
      results.push(assert(measurements.bounds.y1 == 0, "textboxInContainer bounds.y1 is "+measurements.bounds.y1+" but should be 0"));
      results.push(assert(measurements.bounds.y2 == 30, "textboxInContainer bounds.y2 is "+measurements.bounds.y1+" but should be 30"));
      results.push(assert(measurements.charFirst.y == 19 || measurements.charFirst.y == 20, "textboxInContainer charFirst.y is "+measurements.charFirst.y+" but should be 19 or 20"));
      results.push(assert(measurements.charLast.y == 19 || measurements.charLast.y == 20, "textboxInContainer charLast.y is "+measurements.charLast.y+" but should be 20"));
      results.push(assert(measurements.charFirst.x == 0, "textboxInContainer charFirst.y is "+measurements.charFirst.x+" but should be 0"));
      results.push(assert(measurements.charLast.x == 412, "textboxInContainer charLast.y is "+measurements.charLast.x+" but should be 412"));
      
      // Test font metrics
      results.push( assert(obj.pixelSize === 25,"textBoxInContainer.pixelSize does not match expected value of 25: value is "+ obj.pixelSize));
      var metrics = fontXfinityMed.getFontMetrics(obj.pixelSize);
      console.log("metrics baseline is "+metrics.baseline);
      results.push( assert(metrics.baseline === 19 || metrics.baseline === 20,"textboxInContainer baseline does not match expected value of 19 or 20: value is "+ metrics.baseline));
      
      // Measure text via font
      var fontMeasurements = fontXfinityMed.measureText(obj.pixelSize,obj.text);
      console.log("font measure gives width = "+fontMeasurements.w);
      
      var fontMeasurements = fontXfinityMed.measureText(obj.pixelSize,obj.text);
      results.push( assert(measurements.bounds.x2 === fontMeasurements.w, "Text width from textboxInContainer does not match font measurement width: measurements.bounds.x2:"+measurements.bounds.x2+" != fontMeasurements.w:"+fontMeasurements.w));
      
      // Check textBox width 
      results.push(assert(obj.w === 800, "textboxInContainer does not match defined width of 800: value is "+obj.w));
      
      if( other === true) 
        finish(resolve, results);
      other = true;
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
  console.error("Import for test_y_coords.js failed: " + err)
});
