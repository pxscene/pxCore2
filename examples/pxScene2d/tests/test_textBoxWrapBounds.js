"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var string = "The trick will be to find a string that, when wrapping, has a longer line in the middle than at the end or beginning";

var expectedResults = {

  test1: { 
    bounds_x1: 40,
    bounds_y1: 80,
    bounds_x2: 200,
    bounds_y2: 188,
    charFirst_x: 40,
    charFirst_y: 99,
    charLast_x: 155,
    charLast_y: 100
  },
  
  test2: {
    bounds_x1: 220,
    bounds_y1: 80,
    bounds_x2: 380,
    bounds_y2: 188,
    charFirst_x: 220,
    charFirst_y: 99,
    charLast_x: 157,
    charLast_y: 100
  },

  test3: {
    bounds_x1: 40,
    bounds_y1: 225,
    bounds_x2: 200,
    bounds_y2: 333,
    charFirst_x: 40,
    charFirst_y: 244,
    charLast_x: 138,
    charLast_y: 100
  },

  test4: {
    bounds_x1: 220,
    bounds_y1: 225,
    bounds_x2: 380,
    bounds_y2: 333,
    charFirst_x: 220,
    charFirst_y: 244,
    charLast_x: 122,
    charLast_y: 100
  }
  ,

  test5: {
    bounds_x1: 40,
    bounds_y1: 375,
    bounds_x2: 197,
    bounds_y2: 429,
    charFirst_x: 40,
    charFirst_y: 394,
    charLast_x: 73,
    charLast_y: 46
  }
 }

var tests = {

  test1: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      var textBox = scene.create({t:"textBox",parent:root, text:string,
                    x:40, y:80, w:160, h:125, wordWrap:true, ellipsis:true,truncation:scene.truncation.TRUNCATE
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
      var textBox = scene.create({t:"textBox",parent:root, text:string,
                    x:220, y:80, w:160, h:125, wordWrap:true, ellipsis:false,truncation:scene.truncation.TRUNCATE
                    });

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
      var textBox = scene.create({t:"textBox",parent:root, text:string,
                    x:40, y:225, w:160, h:125, wordWrap:true, ellipsis:true,truncation:scene.truncation.TRUNCATE_AT_WORD
                    });

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
  },
  test4: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      var textBox = scene.create({t:"textBox",parent:root, text:string,
                    x:220, y:225, w:160, h:125, wordWrap:true, ellipsis:false,truncation:scene.truncation.TRUNCATE_AT_WORD
                    });

      textBox.ready.then( function(obj) {
        var measure = textBox.measureText();
        results.push(assert(measure.bounds.x1 == expectedResults.test4.bounds_x1, "Bounds x1 "+measure.bounds.x1+" does not match expected value "+ expectedResults.test4.bounds_x1));
        results.push(assert(measure.bounds.y1 == expectedResults.test4.bounds_y1, "Bounds y1 "+measure.bounds.y1+" does not match expected value "+ expectedResults.test4.bounds_y1));
        results.push(assert(measure.bounds.x2 == expectedResults.test4.bounds_x2, "Bounds x2 "+measure.bounds.x2+" does not match expected value "+ expectedResults.test4.bounds_x2));
        results.push(assert(measure.bounds.y2 == expectedResults.test4.bounds_y2, "Bounds y2 "+measure.bounds.y2+" does not match expected value "+ expectedResults.test4.bounds_y2));
        results.push(assert(measure.charFirst.x == expectedResults.test4.charFirst_x, "CharFirst x "+measure.charFirst.x+" does not match expected value "+ expectedResults.test4.charFirst_x));
        results.push(assert(measure.charFirst.y == expectedResults.test4.charFirst_y, "CharFirst y "+measure.charFirst.y+" does not match expected value "+ expectedResults.test4.charFirst_y));
        results.push(assert(measure.charLast.x == expectedResults.test4.charLast_x, "CharLast x "+measure.charLast.x+" does not match expected value "+ expectedResults.test4.charLast_x));
        results.push(assert(measure.charLast.y == expectedResults.test4.charLast_y, "CharLast y "+measure.charLast.y+" does not match expected value "+ expectedResults.test4.charLast_y));

      }, function failure() {
        results.push(assert(false,"TextBox got rejected promise but expected success."));
      }).then(function(error) {
        resolve(results);
      });
    });
  },
  test5: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      var textBox = scene.create({t:"textBox",parent:root, text:'A one-off string to test charLast y',
                    x:40, y:375, w:160, h:125, wordWrap:true, ellipsis:false,truncation:scene.truncation.TRUNCATE_AT_WORD
                    });

      textBox.ready.then( function(obj) {
        var measure = textBox.measureText();
        results.push(assert(measure.bounds.x1 == expectedResults.test5.bounds_x1, "Bounds x1 "+measure.bounds.x1+" does not match expected value "+ expectedResults.test5.bounds_x1));
        results.push(assert(measure.bounds.y1 == expectedResults.test5.bounds_y1, "Bounds y1 "+measure.bounds.y1+" does not match expected value "+ expectedResults.test5.bounds_y1));
        results.push(assert(measure.bounds.x2 == expectedResults.test5.bounds_x2, "Bounds x2 "+measure.bounds.x2+" does not match expected value "+ expectedResults.test5.bounds_x2));
        results.push(assert(measure.bounds.y2 == expectedResults.test5.bounds_y2, "Bounds y2 "+measure.bounds.y2+" does not match expected value "+ expectedResults.test5.bounds_y2));
        results.push(assert(measure.charFirst.x == expectedResults.test5.charFirst_x, "CharFirst x "+measure.charFirst.x+" does not match expected value "+ expectedResults.test5.charFirst_x));
        results.push(assert(measure.charFirst.y == expectedResults.test5.charFirst_y, "CharFirst y "+measure.charFirst.y+" does not match expected value "+ expectedResults.test5.charFirst_y));
        results.push(assert(measure.charLast.x == expectedResults.test5.charLast_x, "CharLast x "+measure.charLast.x+" does not match expected value "+ expectedResults.test5.charLast_x));
        results.push(assert(measure.charLast.y == expectedResults.test5.charLast_y, "CharLast y "+measure.charLast.y+" does not match expected value "+ expectedResults.test5.charLast_y));

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
  console.error("Import for test_textBoxWrapBounds.js failed: " + err)
});
