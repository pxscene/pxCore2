"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene  = imports.scene;
var root   = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var container = scene.create({t:'object',parent:root});

var beforeStart = function() {
  // Nothing to do here...
  console.log("test_animationOscillate.js ... beforeStart ...");
  var promise = new Promise(function(resolve,reject) {
    resolve(assert(true,"beforeStart succeeded"));
  });
  return promise;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

var tests = {

  test_animationOscillate1: function()
  {
    return new Promise(function(resolve, reject)
    {
      var box = scene.create({t:"rect", parent: container, x: 10, y: 10,fillColor: "#0F0", w: 50, h: 50 });

      var results = [];
      box.ready.then(function()
      {
        box.animateTo({ x: 200 }, 1.0, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, 1)
        .then(function(o)
        {
          // Count = 1 ... final position is x: 200
          //
          results.push(assert(o.x ===  200, "test_animationOscillate1 >> 'box.x' is not correct when queried - expected 'x == 200' "));

          console.log("test_animationOscillate1: Result > x: " + o.x);

          o.remove();
          resolve(results);
        });
      });
     });// test promise()
  },

  test_animationOscillate2: function()
  {
    return new Promise(function(resolve, reject)
    {
      var box = scene.create({t:"rect", parent: container, x: 10, y: 10,fillColor: "#0F0", w: 50, h: 50 });

      var results = [];
      box.ready.then(function()
      {
        box.animateTo({ x: 200 }, 1.0, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, 2)
        .then(function(o)
        {
          // Count = 2 ... final position is x: 10
          //
          results.push(assert(o.x === 10, "test_animationOscillate2 >> 'box.x' is not correct when queried - expected 'x == 10' "));

          console.log("test_animationOscillate2: Result > x: " + o.x);

          o.remove();
          resolve(results);
        });
      });
     });// test promise()
  },

  test_animationOscillate3: function()
  {
    return new Promise(function(resolve, reject)
    {
      var box = scene.create({t:"rect", parent: container, x: 10, y: 10,fillColor: "#0F0", w: 50, h: 50 });

      var results = [];
      box.ready.then(function()
      {
        box.animateTo({ x: 200 }, 1.0, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_OSCILLATE, 3)
        .then(function(o)
        {
          // Count = 3 ... final position is x: 10
          //
          results.push(assert(o.x ===  200, "test_animationOscillate3 >> 'box.x' is not correct when queried - expected 'x == 200' "));

          console.log("test_animationOscillate3: Result > x: " + o.x);

          o.remove();
          resolve(results);
        });
      });
     });// test promise()
  },
}

module.exports.tests = tests;
module.exports.beforeStart = beforeStart;

if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}

}).catch( function importFailed(err){
  console.error("Import for test_animationOscillate.js failed: " + err)
});
