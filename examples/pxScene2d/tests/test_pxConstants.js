"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var basePackageUri = px.getPackageBaseFilePath();

var animationInterpolatorNames = [
  ["EASE_OUT_BOUNCE",10],
  ["EASE_OUT_ELASTIC",9],
  ["EASE_IN_ELASTIC",8],
  ["EASE_IN_BACK",7],
  ["EASE_IN_CUBIC",6],
  ["EASE_IN_QUAD",5],
  ["TWEEN_STOP",4],
  ["TWEEN_EXP3",3],
  ["TWEEN_EXP2",2],
  ["TWEEN_EXP1",1],
  ["TWEEN_LINEAR",0]
];

var enumTests = {

  stretch: [ ["NONE", 0],
             ["STRETCH", 1],
             ["REPEAT", 2]
           ],
  alignVertical: [ 
             ["TOP", 0],
             ["CENTER", 1],
             ["BOTTOM", 2]
           ],
  alignHorizontal: [ 
             ["LEFT", 0],
             ["CENTER", 1],
             ["RIGHT", 2]
           ],
  truncation: [ 
             ["NONE", 0],
             ["TRUNCATE", 1],
             ["TRUNCATE_AT_WORD", 2]
           ], 
  animation: [ 
             ["OPTION_OSCILLATE", 1],
             ["OPTION_LOOP", 2],
             ["OPTION_FASTFORWARD", 8],
             ["OPTION_REWIND", 16],
             ["COUNT_FOREVER", -1],
             ["STATUS_IDLE", 0,],
             ["STATUS_INPROGRESS", 1],
             ["STATUS_CANCELLED", 2],
             ["STATUS_ENDED", 3]
           ],    
};


var testConstant = function( property) 
{
      var values = enumTests[property];
      var len = values.length;

      var results = [];

      for( var i = 0; i < len; i++) {
        results.push(assert(scene[property][values[i][0]] == values[i][1], values[i][0]+" is not "+ values[i][1]));
      }

      return results;
}

var beforeStart = function() {
  // Nothing to do here...
  console.log("test_pxConstants start.....");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}

var tests = {

  testInterpolators: function() {
  
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      var interpolators = scene.animation.interpolators;

      var interpLen = interpolators.length;
      results.push(assert(interpLen == animationInterpolatorNames.length, "Number of interpolators returned is not as expected: "+interpLen));

      for (var i = 0; i < interpLen; i++) {
          var interpolatorName = interpolators[i];
          // Test name is as expected
          results.push(assert(interpolators[i] == animationInterpolatorNames[i][0], "Interpolator name \""+interpolators[i]+"\" does not match \""+animationInterpolatorNames[i][0]+"\""));
          // Test constant value/enumerator is as expected
          results.push(assert(scene.animation[interpolators[i]] == animationInterpolatorNames[i][1], "Interpolator enumerator \""+scene.animation.interpolators[i]+"\" does not match \""+animationInterpolatorNames[i][1]+"\""));
          // Test calling animateTo for every interpolator
          var image = scene.create({t:"image",url:basePackageUri+"/images/ball.png",a:0.5,y:-40,parent:root});
          promises.push(image.ready);
          promises.push( image.animateTo({x:550},1,scene.animation[interpolators[i]],scene.animation.OPTION_OSCILLATE,scene.animation.COUNT_FOREVER));        
      }
      // Test default for interpolator
      var image = scene.create({t:"image",url:basePackageUri+"/images/ball.png",a:0.5,y:-40,parent:root});
      promises.push(image.ready);
      promises.push( image.animateTo({x:550},1));

      Promise.all(promises).then(function() {
        results.push(assert(true, "animations started okay"));
        root.removeAll();
        resolve(results);
      }).catch(function(exception) { 
        results.push(assert(false, "Failure in promise for image or animation: "+exception));
        root.removeAll();
        resolve(results);
      });
 
    });
  },

  testStretch: function() {

    return new Promise(function(resolve, reject) {

      resolve( testConstant("stretch"));

    });
  },

  testAlignVertical: function() {

    return new Promise(function(resolve, reject) {

      resolve( testConstant("alignVertical"));
 
    });
  },

  testAlignHorizontal: function() {

    return new Promise(function(resolve, reject) {

      resolve( testConstant("alignHorizontal"));
 
    });
  },

  testTruncation: function() {

    return new Promise(function(resolve, reject) {

      resolve(testConstant("truncation"));

    });
  },

  testAnimation: function() {

    return new Promise(function(resolve, reject) {

      resolve(testConstant("animation"));

    });
  }

}
module.exports.tests = tests;
module.exports.beforeStart = beforeStart;

if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}

}).catch( function importFailed(err){
  console.error("Import for test_pxConstants.js failed: " + err)
});
