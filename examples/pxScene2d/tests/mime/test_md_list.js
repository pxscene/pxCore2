/**
 * unit tests for list elements
 */
px.import({    scene: "px:scene.1.js",
            testUtil: "test_md_util.js",
              assert: "../../test-run/assert.js",
              manual: "../../test-run/tools_manualTests.js"
}).then( function ready(imports)
{
  var assert = imports.assert.assert;
  var manual = imports.manual;
  var scene = imports.scene;
  var testUtil = imports.testUtil;
  var manualTest = manual.getManualTestValue();
  var childScene = scene.create({t:"scene",  w:scene.root.w, h:scene.root.h, parent:scene.root});
  var basePackageUri = px.getPackageBaseFilePath();

  var tests = {

    /**
     * test list
     */
    test_list: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/list.md', function(payload) {
        var results = [];
        var listRoot = payload.root.children[0];
        results.push(assert(listRoot.numChildren * 0.5 === 3, "list item number should be 3"));
        for(var i = 0; i < listRoot.numChildren; i += 1) {
          var node = listRoot.children[i];
          if(i%2 === 0) {
            var index = (i*0.5 + 1) + '.';
            results.push(assert(node.description() === "pxText", "this item node type should be pxText"));
            results.push(assert(node.text === index, "this item node type should be " + index));
          } else {
            results.push(assert(node.description() === "pxObject", "this item node type should be pxObject"));
          }
        }
        return results;
      }); 
    },

    /**
     * test negative list
     */
    test_list_nagative: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/list-negative.md', function(payload) {
        var results = [];
        var negativeNode = payload.root.children[0];
        var listRoot = payload.root.children[1];
        results.push(assert(negativeNode.numChildren === 2, "this node children number should be 2"));
        results.push(assert(listRoot.numChildren === 2, "this node children number should be 2"));
        results.push(assert(listRoot.children[0].description() === "pxText", "this item node type should be pxText"));
        results.push(assert(listRoot.children[1].description() === "pxObject", "this item node type should be pxObject"));
        return results;
      }); 
    },

    /**
     * test unordered list elements
     */
    test_unordered_list: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/unordered-list.md', function(payload) {
        var results = [];
        var listRoot = payload.root.children[0];
        testUtil.dumpRtObject(payload.root.children[0])
        results.push(assert(listRoot.numChildren * 0.5 === 3, "list item number should be 3"));
        for(var i = 0; i < listRoot.numChildren; i += 1) {
          var node = listRoot.children[i];
          if(i%2 === 0) {
            results.push(assert(node.description() === "pxImage", "this item node type should be pxImage"));
          } else {
            results.push(assert(node.description() === "pxObject", "this item node type should be pxObject"));
          }
        }
        return results;
      }); 
    },
    
    /**
     * test negative unordered list
     */
    test_unordered_list_negative: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/unordered-list-negative.md', function(payload) {
        var results = [];
        var negativeRoot = payload.root.children[0];
        var listRoot = payload.root.children[1];
        results.push(assert(negativeRoot.children[0].text === 'Unordered-list-item-01', "render text should be Unordered-list-item-01"));
        results.push(assert(listRoot.numChildren === 2, "this node children number should be 2"));
        results.push(assert(listRoot.children[0].description() === "pxImage", "this item node type should be pxImage"));
        results.push(assert(listRoot.children[1].description() === "pxObject", "this item node type should be pxObject"));
        return results;
      }); 
    },
  }
  
  module.exports.tests = tests;
  if(manualTest === true) {
    manual.runTestsManually(tests);
  }

}).catch( function importFailed(err){
  console.error("Import failed for test_md_list.js: " + err)
});
