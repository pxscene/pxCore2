"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           shots:"../test-run/tools_screenshot.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;
var shots = imports.shots;

var doScreenshot = shots.getScreenshotEnabledValue();
var testPlatform=scene.info.build.os;

var manualTest = manual.getManualTestValue();

var basePackageUri = px.getPackageBaseFilePath();

    var urls = [
      basePackageUri+"/images/ball.png",
      basePackageUri+"/../../../images/tiles/001.jpg", // Lego Movie
      basePackageUri+"/../../../images/tiles/002.jpg", // Kung Fu Panda
      basePackageUri+"/../../../images/tiles/003.jpg", // Cabin Fever
      basePackageUri+"/../../../images/tiles/008.jpg", // Inside Out
    ];
    
var parent = scene.create({t:'object', parent:root, y: 50, clip:false});

var text = scene.create({t:'text', parent:root, clip:false, text:"insideOut should be on top"});

var ball = scene.create({t:'image', parent:parent,url:urls[0], x:0});
var lego = scene.create({t:'image', parent:parent,url:urls[1], x:25});
var panda = scene.create({t:'image', parent:parent,url:urls[2],x:50});
var fever = scene.create({t:'image', parent:parent,url:urls[3],x:75});
var insideOut = scene.create({t:'image', parent:parent,url:urls[4]});
var textObj = scene.create({t:'text', text:'testing text testing text testing text', x:100, textColor:0xCC3030FF}); // give it no parent for now

var noParent = scene.create({t:'image', url:basePackageUri+"/../../../images/tiles/004.jpg"});

var tests = {

  moveBackward_1: function() {
    
    return new Promise(function(resolve, reject) {

        insideOut.moveBackward(); 
        text.text = "insideOut is behind cabin fever; Cabin Fever should be on top"; 
        var results = [];
        results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
        results.push(assert(parent.getChild(1) === lego, "lego is not in expected position."))
        results.push(assert(parent.getChild(2) === panda, "panda is not in expected position."));
        results.push(assert(parent.getChild(3) === insideOut, "insideOut is not in expected position."));
        results.push(assert(parent.getChild(4) === fever, "fever is not in expected position."));
        // Give time for manual visual verification before resolving promise
        setTimeout(function() {
            resolve(results);
          }, 1500);
      });

  },
  moveBackward_2: function() {
      
      return new Promise(function(resolve, reject) {
        insideOut.moveBackward(); 
        text.text = "insideOut is behind Kung Fu Panda"; 
        var results = [];
        results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
        results.push(assert(parent.getChild(1) === lego, "lego is not in expected position."));
        results.push(assert(parent.getChild(2) === insideOut, "insideOut is not in expected position."));
        results.push(assert(parent.getChild(3) === panda, "panda is not in expected position."));
        results.push(assert(parent.getChild(4) === fever, "fever is not in expected position."));
        setTimeout(function() {
            resolve(results);
        },
        3000);
      });

  },
  moveBackward_3: function() {
    
    return new Promise(function(resolve, reject) {
      insideOut.moveBackward(); 
      text.text = "insideOut is behind Lego Movie"; 
      var results = [];
      results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
      results.push(assert(parent.getChild(1) === insideOut, "insideOut is not in expected position."));
      results.push(assert(parent.getChild(2) === lego, "lego is not in expected position."));
      results.push(assert(parent.getChild(3) === panda, "panda is not in expected position."));
      results.push(assert(parent.getChild(4) === fever, "fever is not in expected position."));
      setTimeout(function() {
          resolve(results);
      },
      3000);
    });

  }, 
  moveBackward_4: function() {
    
    return new Promise(function(resolve, reject) {
      insideOut.moveBackward(); 
      text.text = "insideOut is behind ball and on bottom"; 
      var results = [];
      results.push(assert(parent.getChild(0) === insideOut, "insideOut is not in expected position."));
      results.push(assert(parent.getChild(1) === ball, "Ball is not in expected position."));
      results.push(assert(parent.getChild(2) === lego, "lego is not in expected position."));
      results.push(assert(parent.getChild(3) === panda, "panda is not in expected position."));
      results.push(assert(parent.getChild(4) === fever, "fever is not in expected position."));
      setTimeout(function() {
          resolve(results);
      },
      3000);
    });

  },
  moveBackward_noop: function() {
    
    return new Promise(function(resolve, reject) {
      insideOut.moveBackward(); 
      text.text = "what is on screen should not change"; 
      var results = [];
      results.push(assert(parent.getChild(0) === insideOut, "insideOut is not in expected position."));
      results.push(assert(parent.getChild(1) === ball, "Ball is not in expected position."));
      results.push(assert(parent.getChild(2) === lego, "lego is not in expected position."));
      results.push(assert(parent.getChild(3) === panda, "panda is not in expected position."));
      results.push(assert(parent.getChild(4) === fever, "fever is not in expected position."));
      setTimeout(function() {
          resolve(results);
      },
      3000);
    });

  },

  moveForward_1: function() {
    
    return new Promise(function(resolve, reject) {

        insideOut.moveForward(); 
        text.text = "insideOut is in front of ball; Cabin Fever should be on top"; 
        var results = [];
        results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
        results.push(assert(parent.getChild(1) === insideOut, "insideOut is not in expected position."));
        results.push(assert(parent.getChild(2) === lego, "lego is not in expected position."));
        results.push(assert(parent.getChild(3) === panda, "panda is not in expected position."));
        results.push(assert(parent.getChild(4) === fever, "fever is not in expected position."));
        // Give time for manual visual verification before resolving promise
        setTimeout(function() {
            resolve(results);
          }, 1500);
      });

  },
  moveForward_2: function() {
      
      return new Promise(function(resolve, reject) {
        insideOut.moveForward(); 
        text.text = "insideOut is in front of ball and lego"; 
        var results = [];
        results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
        results.push(assert(parent.getChild(1) === lego, "lego is not in expected position."));
        results.push(assert(parent.getChild(2) === insideOut, "insideOut is not in expected position."));
        results.push(assert(parent.getChild(3) === panda, "panda is not in expected position."));
        results.push(assert(parent.getChild(4) === fever, "fever is not in expected position."));
        setTimeout(function() {
            resolve(results);
        },
        3000);
      });

  },
  moveForward_3: function() {
    
    return new Promise(function(resolve, reject) {
      insideOut.moveForward(); 
      text.text = "insideOut is in front of ball, lego and panda; cabin fever is on top"; 
      var results = [];
      results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
      results.push(assert(parent.getChild(1) === lego, "lego is not in expected position."));
      results.push(assert(parent.getChild(2) === panda, "panda is not in expected position."));
      results.push(assert(parent.getChild(3) === insideOut, "insideOut is not in expected position."));
      results.push(assert(parent.getChild(4) === fever, "fever is not in expected position."));
      setTimeout(function() {
          resolve(results);
      },
      3000);
    });

  }, 
  moveForward_4: function() {

  return new Promise(function(resolve, reject) {
    insideOut.moveForward(); 
    text.text = "insideOut is on top"; 
    var results = [];
    results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
    results.push(assert(parent.getChild(1) === lego, "lego is not in expected position."));
    results.push(assert(parent.getChild(2) === panda, "panda is not in expected position."));
    results.push(assert(parent.getChild(3) === fever, "fever is not in expected position."));
    results.push(assert(parent.getChild(4) === insideOut, "insideOut is not in expected position."));
    setTimeout(function() {
        resolve(results);
    },
    3000);
  });

  },
  moveForward_noop: function() {

  return new Promise(function(resolve, reject) {
    insideOut.moveForward(); 
    text.text = "what is on screen should not change"; 
    var results = [];
    results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
    results.push(assert(parent.getChild(1) === lego, "lego is not in expected position."));
    results.push(assert(parent.getChild(2) === panda, "panda is not in expected position."));
    results.push(assert(parent.getChild(3) === fever, "fever is not in expected position."));
    results.push(assert(parent.getChild(4) === insideOut, "insideOut is not in expected position."));
    setTimeout(function() {
        resolve(results);
    },
    3000);
  });

  },

  moveBackward_noParent: function() {
    return new Promise(function(resolve, reject) {
      
        noParent.moveBackward(); 
        text.text = "noParent moveBackward test should not change what's on screen"; 
        var results = [];
        results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
        results.push(assert(parent.getChild(1) === lego, "lego is not in expected position."));
        results.push(assert(parent.getChild(2) === panda, "panda is not in expected position."));
        results.push(assert(parent.getChild(3) === fever, "fever is not in expected position."));
        results.push(assert(parent.getChild(4) === insideOut, "insideOut is not in expected position."));
        // Give time for visual verification before resolving promise
        setTimeout(function() {
            resolve(results);
        }, 3000);
      });
  },
  
  moveForward_noParent: function() {
    
    return new Promise(function(resolve, reject) {
        noParent.moveForward(); 
        text.text = "noParent moveForward test should not change what's on screen";
        var results = [];
        results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
        results.push(assert(parent.getChild(1) === lego, "lego is not in expected position."));
        results.push(assert(parent.getChild(2) === panda, "panda is not in expected position."));
        results.push(assert(parent.getChild(3) === fever, "fever is not in expected position."));
        results.push(assert(parent.getChild(4) === insideOut, "insideOut is not in expected position."));
          setTimeout(function() {
            resolve(results);
          },
          3000);
        });

  },

  moveBackward_differentType: function() {

    return new Promise(function(resolve, reject) {
      textObj.parent = parent;
      textObj.moveBackward(); 
      text.text = "Test Text should be underneath Inside Out, but on top of Cabin Fever";
      var results = [];
      results.push(assert(parent.getChild(0) === ball, "Ball is not in expected position."));
      results.push(assert(parent.getChild(1) === lego, "lego is not in expected position."));
      results.push(assert(parent.getChild(2) === panda, "panda is not in expected position."));
      results.push(assert(parent.getChild(3) === fever, "fever is not in expected position."));
      results.push(assert(parent.getChild(4) === textObj, "textObj is not in expected position."));
      results.push(assert(parent.getChild(5) === insideOut, "insideOut is not in expected position."));
        setTimeout(function() {
          resolve(results);
        },
        3000);
      });    
  }
}
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests);

}

}).catch( function importFailed(err){
  console.error("Import for test_moveToBack.js failed: " + err)
});
