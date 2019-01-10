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

var goodImageUrl = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/ball.png";
var goodImageUrl2 = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/rgb.jpg";
var goodFontUrl = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTTCond-Medium.ttf";
var goodFontUrl2 = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-Medium.ttf";

var fontRes = scene.create({t:"fontResource", url: goodFontUrl2});
var imageRes = scene.create({t:"imageResource", url: goodImageUrl2});
var imageARes = scene.create({t:"imageAResource", url: goodImageUrl2});

// Use a bogus url to cause promise rejection
var image = scene.create({t:"image",parent:root, x:0,y:100, url:goodImageUrl});

var imageA = scene.create({t:"imageA",parent:root, x:0,y:200, url:goodImageUrl});

var image9 = scene.create({t:"image9",parent:root, x:0,y:300, url:goodImageUrl});

var rect = scene.create({t:"rect", parent:root, x:300,y:300, h:50, w:200, fillColor: 0x005454ff});

var text = scene.create({t:"text", parent:root, x:300,y:400, h:50, w:200, text:"I'm just a text; I'm only a text."});

var imageReadySaved = image.ready;
var imageAReadySaved = imageA.ready;
var image9ReadySaved = image9.ready;
var rectReadySaved = rect.ready;
var textReadySaved = text.ready;

var beforeStart = function() {
  // Ensure rejected promise on first, invalid url 
  // before beginning the rest of the test
  console.log("test_promiseCreation start.....");

    return new Promise(function(resolve, reject) {
      var results = []; 
      Promise.all([image.ready, imageA.ready, image9.ready, rect.ready, text.ready]).then( function() {
        results.push(assert((image.ready === imageReadySaved), "image promise is not equal"));
        results.push(assert((imageA.ready === imageAReadySaved), "imageA promise is not equal"));
        results.push(assert((image9.ready === image9ReadySaved), "image9 promise is not equal"));
        results.push(assert((rect.ready === rectReadySaved), "rect promise is not equal"));
        results.push(assert((text.ready === textReadySaved), "text promise is not equal"));

      }, function rejection(o) {
        console.log("Promise.all rejection received");
        results.push(assert(true, "Promise.all received"));
      }).then( function(obj) {
        resolve(results);
      });
    });
 
}

var tests = {

  test_imageSetDim: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      image.w = 300;
      image.h = 300;
      // No new promise should be created
      image.ready.then(function(o) {
        results.push(assert((image.ready === imageReadySaved), "test_imageSetDim: New image Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_imageSetDim: Promise rejection received"));
      }).then( function(obj) {
        imageReadySaved = image.ready;
        resolve(results);
      });
    });
  },

  test_imageASetDim: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      imageA.w = 300;
      imageA.h = 300;
      // No new promise should be created
      imageA.ready.then(function(o) {
        results.push(assert((imageA.ready === imageAReadySaved), "test_imageASetDim: New image Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_imageASetDim: Promise rejection received"));
      }).then( function(obj) {
        imageAReadySaved = imageA.ready;
        resolve(results);
      });
    });
  },

  test_image9SetDim: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      image9.w = 300;
      image9.h = 300;
      // No new promise should be created
      image9.ready.then(function(o) {
        results.push(assert((image9.ready === image9ReadySaved), "test_image9SetDim: New image Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_image9SetDim: Promise rejection received"));
      }).then( function(obj) {
        image9ReadySaved = image9.ready;
        resolve(results);
      });
    });
  },

  test_rectSetDim: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      rect.w = 50;
      // No new promise should be created
      rect.ready.then(function(o) {
        results.push(assert((rect.ready === rectReadySaved), "test_rectSetDim: New rect Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_rectSetDim: Promise rejection received"));
      }).then( function(obj) {
        rectReadySaved = rect.ready;
        resolve(results);
      });
    });
  },

  test_textSetDim: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      text.w = 50;
      // No new promise should be created
      text.ready.then(function(o) {
        results.push(assert((text.ready === textReadySaved), "test_textSetDim: New text Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_textSetDim: Promise rejection received"));
      }).then( function(obj) {
        textReadySaved = text.ready;
        resolve(results);
      });
    });
  },

  test_imageSetStretch: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      image.stretchX = 1;
      image.stretchY = 1;
      // No new promise should be created
      image.ready.then(function(o) {
        results.push(assert((image.ready === imageReadySaved), "test_imageSetStretch: New image Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_imageSetStretch: Promise rejection received"));
      }).then( function(obj) {
        imageReadySaved = image.ready;
        resolve(results);
      });
    });
  },

  test_imageASetStretch: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      imageA.stretchX = 1;
      imageA.stretchY = 1;
      // No new promise should be created
      imageA.ready.then(function(o) {
        results.push(assert((imageA.ready === imageAReadySaved), "test_imageASetStretch: New image Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_imageASetStretch: Promise rejection received"));
      }).then( function(obj) {
        imageAReadySaved = imageA.ready;
        resolve(results);
      });
    });
  },

  test_image9SetStretch: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      image9.stretchX = 1;
      image9.stretchY = 1;
      // No new promise should be created
      image9.ready.then(function(o) {
        results.push(assert((image9.ready === image9ReadySaved), "test_image9SetStretch: New image Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_image9SetStretch: Promise rejection received"));
      }).then( function(obj) {
        image9ReadySaved = image9.ready;
        resolve(results);
      });
    });
  },

  test_rectSetFillColor: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      rect.fillColor = 0xFFFFFFFF;
      // No new promise should be created
      rect.ready.then(function(o) {
        results.push(assert((rect.ready === rectReadySaved), "test_rectSetFillColor: New rect Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_rectSetFillColor: Promise rejection received"));
      }).then( function(obj) {
        rectReadySaved = rect.ready;
        resolve(results);
      });
    });
  },

  test_rectAnimateDim: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      rect.animateTo({w:500}, 0.2, scene.animation.TWEEN_LINEAR).then( function() { 
        // No new promise should be created
        rect.ready.then(function(o) {
          results.push(assert((rect.ready === rectReadySaved), "test_rectAnimateDim: New rect Promise was created"));
        }, function rejection(o) {
          results.push(assert(false, "test_rectAnimateDim: Promise rejection received"));
        }).then( function(obj) {
          rectReadySaved = rect.ready;
          resolve(results);
        });
      });
    });
  },

  test_textAnimateDim: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      text.animateTo({w:500}, 0.2, scene.animation.TWEEN_LINEAR).then( function() { 
        // No new promise should be created
        text.ready.then(function(o) {
          results.push(assert((text.ready === textReadySaved), "test_textAnimateDim: New rect Promise was created"));
        }, function rejection(o) {
          results.push(assert(false, "test_textAnimateDim: Promise rejection received"));
        }).then( function(obj) {
          textReadySaved = text.ready;
          resolve(results);
        });
      });
    });
  },

  test_textSetText: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      text.text = text.text + " Yes I am.";
      // No new promise should be created
      text.ready.then(function(o) {
        results.push(assert((text.ready === textReadySaved), "test_textSetText: New rect Promise was created"));
      }, function rejection(o) {
        results.push(assert(false, "test_textSetText: Promise rejection received"));
      }).then( function(obj) {
        textReadySaved = text.ready;
        resolve(results);
      });
    });
  }, 

  test_textSetFontUrl: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      text.fontUrl = goodFontUrl;
      // New promise should be created
      text.ready.then(function(o) {
        results.push(assert(!(text.ready === textReadySaved), "test_textSetFontUrl: text Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textSetFontUrl: Promise rejection received"));
      }).then( function(obj) {
        textReadySaved = text.ready;
        resolve(results);
      });
    });
  },

  test_textSetFont: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      text.font = fontRes;
      // New promise should be created
      text.ready.then(function(o) {
        results.push(assert(!(text.ready === textReadySaved), "test_textSetFont: text Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_textSetFont: Promise rejection received"));
      }).then( function(obj) {
        textReadySaved = text.ready;
        resolve(results);
      });
    });
  },


  test_imageSetImageUrl: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      image.url = goodImageUrl;
      // No new promise should be created because url is the same
      image.ready.then(function(o) {
        results.push(assert((image.ready === imageReadySaved), "test_imageSetImageUrl: text Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_imageSetImageUrl: Promise rejection received"));
      }).then( function(obj) {
        imageReadySaved = image.ready;
        resolve(results);
      });
    });
  },

  test_imageSetResource: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      image.resource = imageRes;
      // No new promise should be created
      image.ready.then(function(o) {
        results.push(assert(!(image.ready === imageReadySaved), "test_imageSetResource: text Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_imageSetResource: Promise rejection received"));
      }).then( function(obj) {
        imageReadySaved = image.ready;
        resolve(results);
      });
    });
  },

  test_imageSetImageAUrl: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      imageA.url = goodImageUrl;
      // No new promise should be created because url is the same
      imageA.ready.then(function(o) {
        results.push(assert((imageA.ready === imageAReadySaved), "test_imageASetImageUrl: text Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_imageASetImageUrl: Promise rejection received"));
      }).then( function(obj) {
        imageAReadySaved = imageA.ready;
        resolve(results);
      });
    });
  },

  test_imageASetResource: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      imageA.resource = imageARes;
      // No new promise should be created
      imageA.ready.then(function(o) {
        results.push(assert(!(imageA.ready === imageAReadySaved), "test_imageASetResource: text Promise was old"));
      }, function rejection(o) {
        results.push(assert(false, "test_imageASetResource: Promise rejection received"));
      }).then( function(obj) {
        imageAReadySaved = imageA.ready;
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
  console.error("Import failed for test_promiseNewCreation.js: " + err)
});
