px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

  createTxtBox = function(x, h, wordWrap) {
      var txtBoxBg = scene.create({ t:'rect', parent: scene.root, fillColor: 0x555555FF, x: x, y: 50, w: 50, h: h });
      var txtBox = scene.create({ t: 'textBox', parent: txtBoxBg, w: txtBoxBg.w, h: txtBoxBg.h, text: "Watch me truncate", textColor: 0xFFFFFFFF, ellipsis: true, wordWrap: wordWrap, truncation: 1});
      return txtBox;
  };

  var expectedResults = {

    test1: { 
      bounds_x1: 0,
      bounds_y1: 0,
      bounds_x2: 44,
      bounds_y2: 27,
      charFirst_x: 0,
      charFirst_y: 19,
      charLast_x: 44,
      charLast_y: 19
    },
    
    test2: {
      bounds_x1: 0,
      bounds_y1: 0,
      bounds_x2: 44,
      bounds_y2: 25,
      charFirst_x: 0,
      charFirst_y: 19,
      charLast_x: 44,
      charLast_y: 19
    },
 
    test3: {
      bounds_x1: 0,
      bounds_y1: 0,
      bounds_x2: 49,
      bounds_y2: 50,
      charFirst_x: 0,
      charFirst_y: 19,
      charLast_x: 46,
      charLast_y: 46
    }
   }

  var tests = {

    test1: function() {
      return new Promise(function(resolve, reject) {
        var results = [];
        var textBox = createTxtBox(100, 25, false); // truncates with ellipsis
  
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
        var textBox = createTxtBox(200, 25, true); // truncates but no ellipsis
  
        textBox.ready.then( function(obj) {
          var measure = textBox.measureText();
          results.push(assert(measure.bounds.x1 == expectedResults.test2.bounds_x1, "Bounds x1 "+measure.bounds.x1+" does not match expected value "+ expectedResults.test2.bounds_x1));
          results.push(assert(measure.bounds.y1 == expectedResults.test2.bounds_y1, "Bounds y1 "+measure.bounds.y1+" does not match expected value "+ expectedResults.test2.bounds_y1));
          results.push(assert(measure.bounds.x2 == expectedResults.test2.bounds_x2, "Bounds x2 "+measure.bounds.x2+" does not match expected value "+ expectedResults.test2.bounds_x2));
          results.push(assert(measure.bounds.y2 == expectedResults.test2.bounds_y2, "Bounds y2 "+measure.bounds.y2+" does not match expected value "+ expectedResults.test2.bounds_y2));
          results.push(assert(measure.charFirst.x == expectedResults.test2.charFirst_x, "CharFirst x "+measure.charFirst.x+" does not match expected value "+ expectedResults.test2.charFirst_x));
          results.push(assert(measure.charFirst.y == expectedResults.test2.charFirst_y, "CharFirst y "+measure.charFirst.y+" does not match expected value "+ expectedResults.test2.charFirst_y));
          results.push(assert(measure.charLast.x == expectedResults.test2.charLast_x, "CharLast x "+measure.charLast.x+" does not match expected value "+ expectedResults.test2.charLast_x));
          results.push(assert(measure.charLast.y == expectedResults.test2.charLast_y, "CharLast y "+measure.charLast.y+" does not match expected value "+ expectedResults.test2.charLast_y));

        }, function failure() {
          results.push(assert(false,"TextBox got rejected promise but expected success."));
        }).then(function(error) {
          resolve(results);
        });
      });
    },
    test3: function() {
      return new Promise(function(resolve, reject) {
        var results = [];
        var textBox = createTxtBox(300, 50, true); // truncates with ellipsis
  
        textBox.ready.then( function(obj) {
          var measure = textBox.measureText();
          results.push(assert(measure.bounds.x1 == expectedResults.test3.bounds_x1, "Bounds x1 "+measure.bounds.x1+" does not match expected value "+ expectedResults.test3.bounds_x1));
          results.push(assert(measure.bounds.y1 == expectedResults.test3.bounds_y1, "Bounds y1 "+measure.bounds.y1+" does not match expected value "+ expectedResults.test3.bounds_y1));
          results.push(assert(measure.bounds.x2 == expectedResults.test3.bounds_x2, "Bounds x2 "+measure.bounds.x2+" does not match expected value "+ expectedResults.test3.bounds_x2));
          results.push(assert(measure.bounds.y2 == expectedResults.test3.bounds_y2, "Bounds y2 "+measure.bounds.y2+" does not match expected value "+ expectedResults.test3.bounds_y2));
          results.push(assert(measure.charFirst.x == expectedResults.test3.charFirst_x, "CharFirst x "+measure.charFirst.x+" does not match expected value "+ expectedResults.test3.charFirst_x));
          results.push(assert(measure.charFirst.y == expectedResults.test3.charFirst_y, "CharFirst y "+measure.charFirst.y+" does not match expected value "+ expectedResults.test3.charFirst_y));
          results.push(assert(measure.charLast.x == expectedResults.test3.charLast_x, "CharLast x "+measure.charLast.x+" does not match expected value "+ expectedResults.test3.charLast_x));
          results.push(assert(measure.charLast.y == expectedResults.test3.charLast_y, "CharLast y "+measure.charLast.y+" does not match expected value "+ expectedResults.test3.charLast_y));

        }, function failure() {
          results.push(assert(false,"TextBox got rejected promise but expected success."));
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
  console.error("Import for test_textBoxEllipsis_oneLine.js failed: " + err)
});