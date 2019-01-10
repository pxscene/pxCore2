/** 
 * This test will test that a text or textBox that is using an invalid font will still 
 * get a rejected promise when it changes other text properties, but the font still
 * was unable to be downloaded.
 */

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
var basePackageUri = px.getPackageBaseFilePath();

// Use fontUrls to load from web
var fontUrlStart = "http://www.pxscene.org/examples/px-reference/fonts/";
var IndieFlower = "IndieFlower.ttf";
var DejaVu = "DejaVuSans.ttf";
var DejaVuSerif = "DejaVuSerif.ttf";
var DancingScript = "DancingScript-Regular.ttf";
var DancingScriptBold = "DancingScript-Bold.ttf";
// Invalid names for testing
var IndieFlower_notThere = "IndieFlower_notThere.ttf";
var DejaVu_notThere = "DejaVuSans_notThere.ttf";
var DejaVuSerif_notThere = "DejaVuSerif_notThere.ttf";
var DancingScript_notThere = "DancingScript-Regular_notThere.ttf";
var DancingScriptBold_notThere = "DancingScript-Bold_notThere.ttf";

// The two test widgets
var myText = scene.create({t:'text',parent:root,x:15, y: 25, text:"I am a text!"});

var myTextBox = scene.create({t:'textBox',parent:root,x:15, y: 70, text:"I am a textBox!"});

// Load some fontResources
var IndieFlower_font = scene.create({t:'fontResource', url:fontUrlStart+IndieFlower});
var DejaVu_font = scene.create({t:'fontResource', url:fontUrlStart+DejaVu});

var DejaVuSerif_fontBad = scene.create({t:'fontResource', url:fontUrlStart+DejaVuSerif_notThere});
var DancingScript_fontBad = scene.create({t:'fontResource', url:fontUrlStart+DancingScript_notThere});

// beforeStart will verify we have the correct resolutions for the fontResources that were preloaded
var beforeStart = function() {

  return new Promise(function(resolve, reject) {
    
    var results = [];
    Promise.all([IndieFlower_font.ready,DejaVu_font.ready]).then(function () {
      results.push(assert(true, "promise resolved received for IndieFlower_font and DejaVu_font"));
    }, function () {
      results.push(assert(false, "rejection not expected for IndieFlower_font or DejaVu_font"));

    }).then( function () {
      DejaVuSerif_fontBad.ready.then(function resolve() {
        results.push(assert(false, "rejection expected for DejaVuSerif_fontBad, but promise resolved"));
      }, function () {
        results.push(assert(true, "rejection expected and received for DejaVuSerif_fontBad"));

    }).then( function() {
      DancingScript_fontBad.ready.then(function resolve() {
        results.push(assert(false, "rejection expected for DancingScript_fontBad, but promise resolved"));
      }, function () {
        results.push(assert(true, "rejection expected and received for DancingScript_fontBad"));

    }).then(function() {
      resolve(results);
    });
  }, function () {
    results.push(assert(false, "unexpected rejection in beforeStart!"));
    resolve(results);
  }).catch( function(error)  {
    results.push(assert(false, "unexpected exception in beforeStart! "+error));
    resolve(results);
  });
  })});
}

var tests = {

  setTextToInvalidFontUrlAlreadyLoaded: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      myText.fontUrl = fontUrlStart+DejaVuSerif_notThere;
      myText.ready.then(function() {
        results = assert(false, "setTextToInvalidFontUrlAlreadyLoaded: expected rejection but received resolution");
      }, function() {
        results = assert(true, "setTextToInvalidFontUrlAlreadyLoaded: expected and received rejection");
      }).then(function() {
        resolve(results);
      })
    });
  },
  setTextToInvalidFont: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      myText.font = DejaVuSerif_fontBad;
      myText.ready.then(function() {
        results = assert(false, "setTextToInvalidFont: expected rejection but received resolution");
      }, function() {
        results = assert(true, "setTextToInvalidFont: expected and received rejection");
      }).then(function() {
        resolve(results);
      })
    });
  },
  setTextToValidFontUrlAlreadyLoaded: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      myText.fontUrl = fontUrlStart+DejaVu;
      myText.ready.then(function() {
        results = assert(true, "setTextToValidFontUrlAlreadyLoaded: expected and received resolution");
      }, function() {
        results = assert(false, "setTextToValidFontUrlAlreadyLoaded: expected resolution but received rejection");
      }).then(function() {
        resolve(results);
      })
    });
  },
  setTextToValidFont: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      myText.font = IndieFlower_font;
      myText.ready.then(function() {
        results = assert(true, "setTextToValidFont: expected and received resolution");
      }, function() {
        results = assert(false, "setTextToValidFont: expected resolution but received rejection");
      }).then(function() {
        resolve(results);
      })
    });
  },
  setTextPropWhileInvalidFont: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      myText.font = DejaVuSerif_fontBad;
      myText.ready.then(function() {
        results.push(assert(false, "setTextPropWhileInvalidFont: expected rejection but received resolution"));
      }, function() {
        results.push(assert(true, "setTextPropWhileInvalidFont: expected and received rejection"));
      }).then(function() {
        myText.x = 25;
        myText.ready.then(function() {
          results.push(assert(false, "setTextPropWhileInvalidFont: expected rejection but received resolution"));
        }, function() {
          results.push(assert(true, "setTextPropWhileInvalidFont: expected and received rejection"));
        }).then(function() {
          resolve(results);
        });
      })
    });   
  },
  // TEXT BOX
  setTextBoxToInvalidFontUrlAlreadyLoaded: function() {
    return new Promise(function(resolve, reject) {
      var results;
      myTextBox.fontUrl = fontUrlStart+DejaVuSerif_notThere;
      myTextBox.ready.then(function() {
        results = assert(false, "setTextBoxToInvalidFontUrlAlreadyLoaded: expected rejection but received resolution");
      }, function() {
        results = assert(true, "setTextBoxToInvalidFontUrlAlreadyLoaded: expected and received rejection");
      }).then(function() {
        resolve(results);
      })
    });
  },
  setTextBoxToInvalidFont: function() {
    return new Promise(function(resolve, reject) {
      var results;
      myTextBox.font = DejaVuSerif_fontBad;
      myTextBox.ready.then(function() {
        results = assert(false, "setTextBoxToInvalidFont: expected rejection but received resolution");
      }, function() {
        results = assert(true, "setTextBoxToInvalidFont: expected and received rejection");
      }).then(function() {
        resolve(results);
      })
    });
  },
  setTextBoxToValidFontUrlAlreadyLoaded: function() {
    return new Promise(function(resolve, reject) {
      var results;
      myTextBox.fontUrl = fontUrlStart+DejaVu;
      myTextBox.ready.then(function() {
        results = assert(true, "setTextBoxToValidFontUrlAlreadyLoaded: expected and received resolution");
      }, function() {
        results = assert(false, "setTextBoxToValidFontUrlAlreadyLoaded: expected resolution but received rejection");
      }).then(function() {
        resolve(results);
      })
    });
  },
  setTextBoxToValidFont: function() {
    return new Promise(function(resolve, reject) {
      var results;
      myTextBox.font = IndieFlower_font;
      myTextBox.ready.then(function() {
        results = assert(true, "setTextBoxToValidFont: expected and received resolution");
      }, function() {
        results = assert(false, "setTextBoxToValidFont: expected resolution but received rejection");
      }).then(function() {
        resolve(results);
      })
    });
  },
  setTextBoxPropWhileInvalidFont: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      myTextBox.font = DejaVuSerif_fontBad;
      myTextBox.ready.then(function() {
        results.push(assert(false, "setTextBoxPropWhileInvalidFont: expected rejection but received resolution"));
      }, function() {
        results.push(assert(true, "setTextBoxPropWhileInvalidFont: expected and received rejection"));
      }).then(function() {
        myTextBox.x = 25;
        myTextBox.ready.then(function() {
          results.push(assert(false, "setTextBoxPropWhileInvalidFont: expected rejection but received resolution"));
        }, function() {
          results.push(assert(true, "setTextBoxPropWhileInvalidFont: expected and received rejection"));
        }).then(function() {
          resolve(results);
        });
      })
    });   
  }
}

module.exports.beforeStart = beforeStart;
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}

}).catch( function importFailed(err){
  console.error("Import for test_textFontRejection.js failed: " + err)
});