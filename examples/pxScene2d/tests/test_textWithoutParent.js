"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

const PIVOT_HEIGHT = 48;
const PIVOT_TEXT_OFFSETY = -2;
const PIVOT_TEXT_SIZE = 20;
const PIVOT_TEXT_PADDING = 15;
const PIVOT_TEXT_COLOR = 0xfbfcfcfa; // rgba(251, 252, 252, 0.98)
var padding = PIVOT_TEXT_PADDING;

var expectedResults = {
  test1: { 
    bounds_x1: 6,
    bounds_y1: 7,
    bounds_x2: 94,
    bounds_y2: 41,
    charFirst_x: 6,
    charFirst_y: 30,
    charLast_x: 94,
    charLast_y: 30
  }
 }

var tests = {

  test1: function() {
    return new Promise(function(resolve, reject) {
      var results = [];

      var textBox = scene.create({t:'textBox',
          y: PIVOT_TEXT_OFFSETY,
          h: PIVOT_HEIGHT,
          alignHorizontal: scene.alignHorizontal.CENTER,
          alignVertical: scene.alignVertical.CENTER,
          text: 'some text',
          pixelSize: PIVOT_TEXT_SIZE,
          textColor: PIVOT_TEXT_COLOR,
          w: 100,
          x: padding
      });

      textBox.ready.then( function(obj) {
        var measure = textBox.measureText();
        results.push(assert(measure.bounds.x1 == expectedResults.test1.bounds_x1, "Bounds x1 "+measure.bounds.x1+" does not match expected value "+ expectedResults.test1.bounds_x1));
        results.push(assert(measure.bounds.y1 == expectedResults.test1.bounds_y1, "Bounds y1 "+measure.bounds.y1+" does not match expected value "+ expectedResults.test1.bounds_y1));
        results.push(assert(measure.bounds.x2 == expectedResults.test1.bounds_x2, "Bounds x2 "+measure.bounds.x2+" does not match expected value "+ expectedResults.test1.bounds_x2));
        results.push(assert(measure.bounds.y2 == expectedResults.test1.bounds_y2, "Bounds y2 "+measure.bounds.y2+" does not match expected value "+ expectedResults.test1.bounds_y2));
        results.push(assert(measure.charFirst.x == expectedResults.test1.charFirst_x, "CharFirst x "+measure.charFirst.x+" does not match expected value "+ expectedResults.test1.charFirst_x));
        results.push(assert(measure.charFirst.y == expectedResults.test1.charFirst_y, "CharFirst y "+measure.charFirst.y+" does not match expected value "+ expectedResults.test1.charFirst_y));
        results.push(assert(measure.charLast.x == expectedResults.test1.charLast_x, "CharLast x "+measure.charLast.x+" does not match expected value "+ expectedResults.test1.charLast_x));
        results.push(assert(measure.charLast.y == expectedResults.test1.charLast_y, "CharLast y "+measure.charLast.y+" does not match expected value "+ expectedResults.test1.charLast_y));
      }, function failure() {
        results.push(assert(false,"TextBox got rejected promise but expected success."));
      }).then(function(error) {
        resolve(results);
      });
    });
  },
  test2: function() {
    return new Promise(function(resolve, reject) {
      var results = [];

      var textElem = scene.create({t:'text',
          y: PIVOT_TEXT_OFFSETY,
          h: PIVOT_HEIGHT,
          text: 'some text',
          pixelSize: PIVOT_TEXT_SIZE,
          w: 100,
          x: padding
      });

      textElem.ready.then( function(obj) {
        results.push(assert(true,"Text element got resolved promise"));
      }, function failure() {
        results.push(assert(false,"Text element got rejected promise but expected success."));
      }).then(function(error) {
        resolve(results);
      });
    });
  }
}
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests);

}

}).catch( function importFailed(err){
  console.error("Import for test_textWithoutParent.js failed: " + err)
});
