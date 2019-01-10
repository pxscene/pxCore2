"use strict";
/** This test is for XRE2-597 - test reassigning url to image that had rejected promise, then url="", then url= valid url */

px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var goodFontUrl = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTTCond-Medium.ttf";
var goodFontUrl2 = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-Medium.ttf";

var fontRes = scene.create({t:"fontResource", url: goodFontUrl2});

var textBox = scene.create({t:"textBox", parent:root, x:300,y:100, h:50, w:200, wordWrap:true, text: "some longish text for testing in order to see wrapping change as properties change"});


var textBoxReadySaved = textBox.ready;

var beforeStart = function() {
  // Ensure rejected promise on first, invalid url 
  // before beginning the rest of the test
  console.log("test_promiseCreationTextBox start.....");

    return new Promise(function(resolve, reject) {
      var results = []; 
      Promise.all([textBox.ready]).then( function() {
        results.push(assert((textBox.ready === textBoxReadySaved), "textBox promise is not equal"));

      }, function rejection(o) {
        console.log("Promise.all rejection received");
        results.push(assert(true, "Promise.all received"));
      }).then( function(obj) {
        resolve(results);
      });
    });
 
}

var tests = {

  test_textBoxSetW: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.w = 50;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetW: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetW: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetText: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.text = textBox.text + " and add a bit more text for fun.";
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetDim: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetDim: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxAnimateW: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.animateTo({w:500}, 0.2, scene.animation.TWEEN_LINEAR).then( function() { 
        // New promise should be created
        textBox.ready.then(function(o) {
          results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxAnimateW: textBox Promise was old"));
        }, function rejection(o) {
          results.push(assert(false, "test_textBoxAnimateW: Promise rejection received"));
        }).then( function(obj) {
          textBoxReadySaved = textBox.ready;
          resolve(results);
        });
      });
    });
  },
  test_textBoxAnimateH: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.animateTo({h:500}, 0.2, scene.animation.TWEEN_LINEAR).then( function() { 
        // New promise should be created
        textBox.ready.then(function(o) {
          results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxAnimateH: textBox Promise was old"));
        }, function rejection(o) {
          results.push(assert(false, "test_textBoxAnimateH: Promise rejection received"));
        }).then( function(obj) {
          textBoxReadySaved = textBox.ready;
          resolve(results);
        });
      });
    });
  },

  test_textBoxSetFontUrl: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.fontUrl = goodFontUrl;
      // No new promise should be created
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetFontUrl: text Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetFontUrl: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },
  test_textBoxSetFont: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.font = fontRes;
      // No new promise should be created
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetFont: text Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetFont: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetH: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.h = 50;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetH: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetH: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetWordWrap: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.wordWrap = false;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetWordWrap: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetWordWrap: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetClip: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.clip = true;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetClip: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetClip: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetEllipsis: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.ellipsis = !textBox.ellipsis;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetEllipsis: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetEllipsis: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetTruncation: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.truncation = 2;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetTruncation: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetTruncation: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetXStartPos: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.xStartPos = 5;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetXStartPos: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetXStartPos: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetXStopPos: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.xStopPos = 5;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetXStopPos: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetXStopPos: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetAlignVertical: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.alignVertical = 2;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetAlignVertical: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetAlignVertical: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetAlignHorizontal: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.alignHorizontal = 2;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetAlignHorizontal: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetAlignHorizontal: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetLeading: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.leading = 2;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetLeading: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetLeading: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetPixelSize: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.pixelSize = 18;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetPixelSize: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetPixelSize: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetSx: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.sx = 1.5;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetSx: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetSx: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxSetSy: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.sy = 1.5;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxSetSy: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxSetSy: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },

  test_textBoxReset: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      textBox.h = 50;
      textBox.w = 200;
      textBox.wordWrap = true;
      textBox.truncation = 0;
      textBox.clip = false;
      textBox.alignVertical = 0;
      textBox.alignHorizontal = 0;
      textBox.xStartPos = 0;
      textBox.xStopPos = 0;
      textBox.sx = 1.0;
      textBox.sy = 1.0;
      // New promise should be resolved or rejected
      textBox.ready.then(function(o) {
        results.push(assert(!(textBox.ready === textBoxReadySaved), "test_textBoxReset: textBox Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textBoxReset: Promise rejection received"));
      }).then( function(obj) {
        textBoxReadySaved = textBox.ready;
        resolve(results);
      });
    });
  },
}
module.exports.beforeStart = beforeStart;
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}

}).catch( function importFailed(err){
  console.error("Import failed for test_promiseNewCreationTextBox.js: " + err)
});
