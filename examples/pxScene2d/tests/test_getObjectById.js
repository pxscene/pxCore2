px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var rect = scene.create({t:"rect", parent:scene.root, w:400, h:400, lineColor:0xFF0000FF, lineWidth:1});
var container = scene.create({t:"object", parent:scene.root, w:400, h:400});
var text2 = scene.create({t:"textBox", parent:container,h:400,w:400,textColor:0xFFDDFFFF,pixelSize:20,fontUrl:"FreeSans.ttf",alignHorizontal:1,alignVertical:1,
            text:"I'm a giraffe!",
            id:'giraffe'});

 var text2 = scene.create({t:"textBox", parent:root,x:400, y:400,h:200,w:200,textColor:0xFFDDFFFF,pixelSize:20,fontUrl:"FreeSans.ttf",alignHorizontal:1,alignVertical:1,
            text:"I'm an elephant!",
            id:'elephant'});             


var tests = 
{
  // Test that null is returned if an object by Id is not within the parent being searched
  testNull: function() {
    console.log("testNull");
    return new Promise(function(resolve, reject) {
      var elephant = container.getObjectById("elephant");

      resolve(assert(elephant==null, "Object returned was not null."));

    },function(o) {
      results.push(assert(false,"Received unexpected promise rejection in testNull"))
      resolve(results);
    });
  },
  // Test that an object can be retrieved by ID from a parent object
  testObj: function() {
    console.log("testObj");
    return new Promise(function(resolve, reject) {
      var giraffe = container.getObjectById("giraffe");
      var results = [];
      results.push(assert(giraffe != null, "object was not found by id"));
      if(giraffe != null){
        results.push(assert(giraffe.text === "I'm a giraffe!", "Correct object was not returned for id"));
      }
      resolve(results);

    },function(o) {
      results.push(assert(false,"Received unexpected promise rejection in testObj"))
      resolve(results);
    });
  },
  // Test that an object can be retrieved by ID from root
  testObj2: function() {
    console.log("testObj2");
    return new Promise(function(resolve, reject) {
      var elephant = root.getObjectById("elephant");
      var results = [];
      results.push(assert(elephant != null, "object was not found by id"));
      if( elephant != null) {
        results.push(assert(elephant.text === "I'm an elephant!", "Correct object was not returned for id"));
      }
      resolve(results);

    },function(o) {
      results.push(assert(false,"Received unexpected promise rejection in testObj2"))
      resolve(results);
    });
  }
}
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests);

}

}).catch( function importFailed(err){
  console.error("Import failed for test_xre2-71.js: " + err)
});