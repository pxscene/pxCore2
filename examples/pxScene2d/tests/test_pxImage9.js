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
  "http://www.pxscene.org/examples/px-reference/gallery/images/apng/cube.png",     // apng
  "http://www.pxscene.org/examples/px-reference/gallery/images/star.png",          // supports plain old pngs
  "http://www.pxscene.org/examples/px-reference/gallery/images/ajpeg.jpg",         // and single frame jpegs too!!
];



var beforeStart = function() {
  // Nothing to do here...
  console.log("test_pxImage9 beforeStart.....");
  var promise = new Promise(function(resolve,reject) {
    resolve(assert(true,"beforeStart succeeded"));
  });
  return promise;
}

var tests = {

  testLoad: function() {
    console.log("Running image9 testLoad");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"image9",url:urls[i],parent:root}).ready);
      }

 
      Promise.all(promises).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testLoad: promise all returned undefined objs"));
        }
        else {
          for(var i = 0; i < objs.length; i++) {
            if( objs[i] == undefined) {
              results.push(assert(false, "testLoad: promise all returned an undefined obj at index "+i));
            }
            else {
              results.push(assert(objs[i].url === urls[i], "image9 url is not correct when queried"));
            }
          }
        }
      }, function(obj) {//rejected
        results.push(assert(false, "image9 load failed : "+obj));
      }).catch(function(exception) {
          results.push(assert(false, "image9 load failed : "+exception));       
      }).then(function() {
        root.removeAll();
        resolve(results);
      });
    });
  },

  testReload: function() {
    console.log("Running image9 testReload");
    return new Promise(function(resolve, reject) {

      var results = [];
      
      var image9 = scene.create({t:"image9",url:urls[0],parent:root});

      image9.ready.then( function(obj) {
        if( obj == undefined) {
          results.push(assert(false, "testLoad: promise all returned undefined obj"));
        } 
        else {
          results.push(assert(obj.url === urls[0], "image9 url is not correct when queried"));
          obj.url = urls[1];
          obj.ready.then(function(newObj) {
            if( newObj != undefined) {
              results.push(assert(newObj.url === urls[1], "image9 url is not correct when queried"));
            } else {
              results.push(assert(false, "testReload received undefined object in promise resolution")); 
            }  
          }, function(o) {
            results.push(assert(false, "testReload received undefined object in promise resolution")); 
          })
        }
      }, function(obj) {//rejected
        results.push(assert(false, "image9 url set failed : "+obj));
      }).catch(function(exception) {
          results.push(assert(false, "image url set failed : "+exception));       
      }).then(function() {
        root.removeAll();
        resolve(results);
      });

    });   
  },

  testInsetsInteger: function() {
    console.log("Running image9 testInsetsInteger");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"image9",url:urls[i],parent:root,insetLeft:5,insetRight:5,insetTop:5,insetBottom:5}).ready);
      }


      Promise.all(promises).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testInsetsInteger: promise all returned undefined objs"));
        } 
        else {
          for(var i = 0; i < objs.length; i++) {
            if( objs[i] != undefined) {
              results.push(assert(objs[i].insetLeft === 5, "image9 insetLeft is not 5: "+objs[i].insetLeft));
              results.push(assert(objs[i].insetRight === 5, "image9 insetRight is not 5: "+objs[i].insetRight));
              results.push(assert(objs[i].insetTop === 5, "image9 insetTop is not 5: "+objs[i].insetTop));
              results.push(assert(objs[i].insetBottom === 5, "image9 insetBottom is not 5: "+objs[i].insetBottom));
            } else {
                results.push(assert(false, "testInsetsInteger: promise all returned an undefined obj at index "+i));
              }
          }
        }
      }, function rejection(obj) {
          results.push(assert(false, "image9 inset properties failed for positive value: "+obj));       
      }).then(function() {
        root.removeAll();
        resolve(results);
      });
    });
  },

  testInsetsNegative: function() {
    console.log("Running image9 testInsetsNegative");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"image9",url:urls[i],parent:root,insetLeft:-5,insetRight:-5,insetTop:-5,insetBottom:-5}).ready);
      }


      Promise.all(promises).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testInsetsNegative: promise all returned undefined objs"));
        } 
        else {
          for(var i = 0; i < objs.length; i++) {
            if( objs[i] != undefined) {
              results.push(assert(objs[i].insetLeft === -5, "image9 insetLeft is not -5: "+objs[i].insetLeft));
              results.push(assert(objs[i].insetRight === -5, "image9 insetRight is not -5: "+objs[i].insetRight));
              results.push(assert(objs[i].insetTop === -5, "image9 insetTop is not -5: "+objs[i].insetTop));
              results.push(assert(objs[i].insetBottom === -5, "image9 insetBottom is not -5: "+objs[i].insetBottom));
            }
            else {
              results.push(assert(false, "testInsetsNegative: promise all returned an undefined obj at index "+i));
            }
          }
        }
      }, function rejection(obj) {
          results.push(assert(false, "image9 inset properties failed for negative value: "+obj));       
      }).then(function() {
        root.removeAll();
        resolve(results);
      });
    });
  },

  testInsetsZeroValue: function() {
    console.log("Running image9 testInsetsZeroValue");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      var value = 0;
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"image9",url:urls[i],parent:root,insetLeft:value,insetRight:value,insetTop:value,insetBottom:-value}).ready);
      }


      Promise.all(promises).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testStretchNone: promise all returned undefined objs"));
        } 
        else {
          for(var i = 0; i < objs.length; i++) {
            if( objs[i] != undefined) {
              results.push(assert(objs[i].insetLeft === value, "image9 insetLeft is not "+value+": "+objs[i].insetLeft));
              results.push(assert(objs[i].insetRight === value, "image9 insetRight is not "+value+": "+objs[i].insetRight));
              results.push(assert(objs[i].insetTop === value, "image9 insetTop is not "+value+": "+objs[i].insetTop));
              results.push(assert(objs[i].insetBottom === value, "image9 insetBottom is not "+value+": "+objs[i].insetBottom));
            }
            else {
              results.push(assert(false, "testStretchNone: promise all returned an undefined obj at index "+i));
            }
          }
        }
      }, function rejection(obj) {
          results.push(assert(false, "image9 inset properties failed for zero value: "+obj));       
      }).then(function() {
        root.removeAll();
        resolve(results);
      });
    });
  },

 testImage9Resource: function() {
   console.log("Running image9 testImage9Resource");
    return new Promise(function(resolve, reject) {

      var results = [];

      var imageResource = scene.create({t:"imageResource", url:urls[0]});
      var image = scene.create({t:"image9",resource:imageResource,parent:root});

      Promise.all([imageResource.ready,image.ready]).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testImage9Resource: promise all returned undefined objs"));
        } 
        else { 
          if( objs[0] != undefined) {
            results.push(assert(objs[0].url == urls[0], "image url is not correct when created with resource"));
          }
          else {
            results.push(assert(false, "testImage9Resource: promise all returned an undefined obj at index 0"));
          }
          if( objs[1] != undefined) {
            results.push(assert(objs[1].url == urls[0], "imageResource url is not correct: "+objs[1].url ));
          }
          else {
            results.push(assert(false, "testImage9Resource: promise all returned an undefined obj at index 1"));
          }
        }
      }, function rejection(o) {
        results.push(assert(false, "imageResource for image9 failed: "+o));      
      }).then(function() {
        root.removeAll();
        resolve(results);
      });

    });
  }
  ,
  // Trying to load an imageAResource should fail for pxImage
 testImageResourceFailure: function() {
   console.log("Running image9 testImageResourceFailure");
    return new Promise(function(resolve, reject) {

      var results = [];

      var imageAResource = scene.create({t:"imageAResource", url:urls[0]});
      var image = scene.create({t:"image9",resource:imageAResource,parent:root});

      Promise.all([imageAResource.ready,image.ready]).then(function(objs) {
        if( objs == undefined) {
          results.push(assert(false, "testImageResourceFailure: promise all returned undefined objs"));
        } 
        else { 
          if( objs[0] != undefined) {
            results.push(assert(objs[0].url == urls[0], "image url is not correct when created with resource"));
          }
          else {
            results.push(assert(false, "testImageResourceFailure: promise all returned an undefined obj at index 0"));
          }
          if( objs[1] != undefined) {
            results.push(assert(objs[1].url == urls[0], "imageAResource url is not correct"));
          }
          else {
            results.push(assert(false, "testImageResourceFailure: promise all returned an undefined obj at index 1"));
          }
        }

      }, function rejection(obj) {
        results.push(assert(true, "imageAResource for image9 correctly caused pxImage promise rejection : "));      
      }).then(function() {
        root.removeAll();
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
  console.error("Import for test_pxImage9.js failed: " + err)
});
