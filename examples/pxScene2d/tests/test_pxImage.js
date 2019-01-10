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
  "http://www.pxscene.org/examples/px-reference/gallery/images/gold_star.png", 
  "http://www.pxscene.org/examples/px-reference/gallery/images/banana.png",     
  "http://www.pxscene.org/examples/px-reference/gallery/images/grapes.png",  
  "http://www.pxscene.org/examples/px-reference/gallery/images/star.png",          
  "http://www.pxscene.org/examples/px-reference/gallery/images/flower1.jpg",         
];

var container = scene.create({t:'object',parent:root});

var beforeStart = function() {
  // Nothing to do here...
  console.log("test_pxImage beforeStart.....");
  var promise = new Promise(function(resolve,reject) {
    resolve(assert(true,"beforeStart succeeded"));
  });
  return promise;
}

var tests = {

  testLoad: function() {
    console.log("Running testLoad");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"image",url:urls[i],parent:container}).ready);
      }

 
      Promise.all(promises).then(function(objs) {
        for(var i = 0; i < objs.length; i++) {
          results.push(assert(objs[i].url === urls[i], "image url is not correct when queried"));
        }
      }, function(obj) {//rejected
        results.push(assert(false, "image load failed : "+obj));
      }).catch(function(obj) {
          results.push(assert(false, "image load failed : "+obj));       
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testLoad");
      });
    });
  },

  testReload: function() {
    console.log("Running testReload");
    return new Promise(function(resolve, reject) {

      var results = [];
      
      var image = scene.create({t:"image",url:urls[0],parent:container});

      image.ready.then( function(obj) {
        results.push(assert(obj.url === urls[0], "image url is not correct when queried"));
        obj.url = urls[1];
        obj.ready.then(function(newObj) {
          results.push(assert(newObj.url === urls[1], "image url is not correct when queried"));
        })
      }, function(obj) {//rejected
        results.push(assert(false, "image url set failed : "+obj));
      }).catch(function(exception) {
          results.push(assert(false, "image url set failed : "+exception));       
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testReload");
      });

    });   
  },

  testStretchNone: function() {
    console.log("Running testStretchNone");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"image",url:urls[i],parent:container,stretchX:scene.stretch.NONE,stretchY:scene.stretch.NONE}).ready);
      }


      Promise.all(promises).then(function(objs) {
        for(var i = 0; i < objs.length; i++) {
          results.push(assert(objs[i].stretchX === scene.stretch.NONE, "image stretchX is not NONE"));
          results.push(assert(objs[i].stretchY=== scene.stretch.NONE, "image stretchY is not NONE"));
        }
       
      }, function(obj) {//rejected
        results.push(assert(false, "image stretch NONE failed : "+obj));
      }).catch(function(exception) {

          results.push(assert(false, "image stretch NONE failed: "+exception));       
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testStretchNone");
      });
    });
  },

  testStretchStretch: function() {
    console.log("Running testStretchStretch");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"image",url:urls[i],parent:container,stretchX:scene.stretch.STRETCH,stretchY:scene.stretch.STRETCH}).ready);
      }

      Promise.all(promises).then(function(objs) {
        for(var i = 0; i < objs.length; i++) {
          results.push(assert(objs[i].stretchX === scene.stretch.STRETCH, "image stretchX is not STRETCH"));
          results.push(assert(objs[i].stretchY === scene.stretch.STRETCH, "image stretchY is not STRETCH"));
        }
      }, function(obj) {//rejected
        results.push(assert(false, "image stretch STRETCH failed : "+obj));
      }).catch(function(exception) {
          results.push(assert(false, "image stretch STRETCH failed: "+exception));       
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testStretchStretch");
      });
    });
  },

  testStretchRepeat: function() {
    console.log("Running testStretchRepeat");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"image",url:urls[i],parent:container,stretchX:scene.stretch.REPEAT,stretchY:scene.stretch.REPEAT}).ready);
      }

      Promise.all(promises).then(function(objs) {
        for(var i = 0; i < objs.length; i++) {
          results.push(assert(objs[i].stretchX === scene.stretch.REPEAT, "image stretchX is not REPEAT"));
          results.push(assert(objs[i].stretchY === scene.stretch.REPEAT, "image stretchY is not REPEAT"));
        }
      }, function(obj) {//rejected
        results.push(assert(false, "image stretch REPEAT failed : "+obj));
      }).catch(function(exception) {

          results.push(assert(false, "image stretch REPEAT failed : "+exception));       
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testStretchRepeat");
      });
    });
  }
 ,
 testImageResource: function() {
   console.log("Running testImageResource");
    return new Promise(function(resolve, reject) {

      var results = [];

      var imageResource = scene.create({t:"imageResource", url:urls[0]});
      var image = scene.create({t:"image",resource:imageResource,parent:container});

      Promise.all([imageResource.ready,image.ready]).then(function(objs) {
        results.push(assert(objs[0].url == urls[0], "image url is not correct when created with resource"));
        results.push(assert(objs[1].url == urls[0], "imageResource url is not correct: "+objs[1].url ));

      }, function rejection() {
        results.push(assert(false, "imageResource failed  "));      
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testImageResource");
      });

    });
  }
  , 
  
  // Trying to load an imageAResource should fail for pxImage
 testImageResourceFailure: function() {
   console.log("Running testImageResourceFailure");
    return new Promise(function(resolve, reject) {

      var results = [];

      var imageAResource = scene.create({t:"imageAResource", url:urls[0]});
      var image = scene.create({t:"image",resource:imageAResource,parent:container});

      Promise.all([imageAResource.ready,image.ready]).then(function(objs) {
        results.push(assert(objs[0].url == urls[0], "image url is not correct when created with resource"));
        results.push(assert(objs[1].url == urls[0], "imageAResource url is not correct"));

      }, function rejection() {
        results.push(assert(true, "imageAResource correctly caused pxImage promise rejection"));      
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testImageResourceFailure");
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
  console.error("Import for test_pxImage.js failed: " + err)
});
