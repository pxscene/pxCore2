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

var urls = [
  "http://www.pxscene.org/examples/px-reference/gallery/images/apng/elephant.png", // apng
  "http://www.pxscene.org/examples/px-reference/gallery/images/apng/cube.png",     // apng
  "http://www.pxscene.org/examples/px-reference/gallery/images/apng/spinfox.png",  // apng
  "http://www.pxscene.org/examples/px-reference/gallery/images/star.png",          // supports plain old pngs
  "http://www.pxscene.org/examples/px-reference/gallery/images/ajpeg.jpg",         // and single frame jpegs too!!
];

var container = scene.create({t:'object',parent:root});

var beforeStart = function() {
  // Nothing to do here...
  console.log("test_pxImageA beforeStart.....");
  var promise = new Promise(function(resolve,reject) {
    resolve(assert(true,"beforeStart succeeded"));
  });
  return promise;
}

var tests = {

  testLoad: function() {
    console.log("Running imageA testLoad");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"imageA",url:urls[i],parent:container}).ready);
      }

 
      Promise.all(promises).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testLoad: promise all returned undefined objs"));
        } 
        else {
          for(var i = 0; i < objs.length; i++) {
            if( objs[i] != undefined) {
              results.push(assert(objs[i].url === urls[i], "imageA url is not correct when queried"));
            }
            else {
              results.push(assert(false, "testLoad: promise all returned an undefined obj at index "+i));
            }
          }
        }
      }, function(obj) {//rejected
        results.push(assert(false, "imageA load failed : "+obj));
      }).catch(function(exception) {
          results.push(assert(false, "imageA load failed : "+exception));       
      }).then(function() {
        container.removeAll();
        resolve(results);
      });
    });
  },

  testReload: function() {
    console.log("Running imageA testReload");
    return new Promise(function(resolve, reject) {

      var results = [];
      
      var imageA = scene.create({t:"imageA",url:urls[0],parent:container});

      imageA.ready.then( function(obj) {
        if( obj == undefined) {
          results.push(assert(false, "testLoad: promise all returned undefined obj"));
          container.removeAll();
          resolve(results);
        } 
        else {
          results.push(assert(obj.url === urls[0], "imageA url is not correct when queried"));
          obj.url = urls[1];
          obj.ready.then(function(newObj) {
            if( newObj != undefined) {
              results.push(assert(newObj.url === urls[1], "imageA url is not correct when queried " + newObj.url));
              if (newObj.url != urls[1])
              {
                console.log(newObj);
              }
            } else {
              results.push(assert(false, "testReload received undefined object in promies resolution")); 
            }
          }, function() {
               results.push(assert(false, "testReload received rejection on reassignment"));
          }).catch(function(exception) {
              results.push(assert(false, "imageA reload failed : "+exception));       
          }).then(function() {
            container.removeAll();
            resolve(results);
          });
        }
      });
    });   
  },

  testStretchNone: function() {
    console.log("Running imageA testStretchNone");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"imageA",url:urls[i],parent:container,stretchX:scene.stretch.NONE,stretchY:scene.stretch.NONE}).ready);
      }


      Promise.all(promises).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testStretchNone: promise all returned undefined objs"));
        } 
        else {
          for(var i = 0; i < objs.length; i++) {
            if( objs[i] != undefined) {
              results.push(assert(objs[i].stretchX === scene.stretch.NONE, "imageA stretchX is not NONE"));
              results.push(assert(objs[i].stretchY=== scene.stretch.NONE, "imageA stretchY is not NONE"));
            }
            else {
              results.push(assert(false, "testStretchNone: promise all returned an undefined obj at index "+i));
            }
          }
          console.log("got all promises");
        }
       
      }, function(obj) {//rejected
        results.push(assert(false, "imageA stretch NONE failed : "+obj));
      }).catch(function(exception) {

          results.push(assert(false, "imageA stretch NONE failed: "+exception));       
      }).then(function() {
        container.removeAll();
        resolve(results);
      });
    });
  },

  testStretchStretch: function() {
    console.log("Running imageA testStretchStretch");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"imageA",url:urls[i],parent:container,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH}).ready);
      }

      Promise.all(promises).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testStretchNone: promise all returned undefined objs"));
        } 
        else {
          for(var i = 0; i < objs.length; i++) {
            if( objs[i] != undefined) {
              results.push(assert(objs[i].stretchX === scene.stretch.STRETCH, "imageA stretchX is not STRETCH"));
              results.push(assert(objs[i].stretchY === scene.stretch.STRETCH, "imageA stretchY is not STRETCH"));
            }
            else {
              results.push(assert(false, "testStretchStretch: promise all returned an undefined obj at index "+i));
            }
          }
        }
      }, function(obj) {//rejected
        results.push(assert(false, "imageA stretch STRETCH failed : "+obj));
      }).catch(function(exception) {
          results.push(assert(false, "imageA stretch STRETCH failed: "+exception));       
      }).then(function() {
        container.removeAll();
        resolve(results);
      });
    });
  },

  testStretchRepeat: function() {
    console.log("Running imageA testStretchRepeat");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"imageA",url:urls[i],parent:container,stretchX:scene.stretch.REPEAT,stretchY:scene.stretch.REPEAT}).ready);
      }

      Promise.all(promises).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testStretchNone: promise all returned undefined objs"));
        } 
        else {
          for(var i = 0; i < objs.length; i++) {
            if( objs[i] != undefined) {
              results.push(assert(objs[i].stretchX === scene.stretch.REPEAT, "imageA stretchX is not REPEAT"));
              results.push(assert(objs[i].stretchY === scene.stretch.REPEAT, "imageA stretchY is not REPEAT"));
            }
            else {
              results.push(assert(false, "testStretchStretch: promise all returned an undefined obj at index "+i));
            }
          }
        }
      }, function(obj) {//rejected
        results.push(assert(false, "imageA stretch REPEAT failed : "+obj));
      }).catch(function(exception) {

          results.push(assert(false, "imageA stretch REPEAT failed : "+exception));       
      }).then(function() {
        container.removeAll();
        resolve(results);
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
  console.error("Import for test_pxImageA.js failed: " + err)
});
