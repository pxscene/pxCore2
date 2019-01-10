/**
 * unit tests for link
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
     * test for link
     */
    test_link: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/link.md', function(payload) {
        var results = [];
        var link1 = payload.root.children[0];
        var link2 = payload.root.children[1];

        results.push(assert(link1.children[0].description() === "pxText", 'first node should be a pxText'));
        results.push(assert(link1.children[0].text === "google", 'render text should be google'));
        results.push(assert(link1.children[0].type === "link", 'node type should be link'));
        results.push(assert(link1.children[0].children[0].description() === "pxObject", 'second node should be a pxObject'));
 
        results.push(assert(link2.children[0].description() === "pxText", 'first node should be a pxText'));
        results.push(assert(link2.children[0].text === "markdown file", 'render text should be google'));
        results.push(assert(link2.children[0].type === "link", 'node type should be link'));
        results.push(assert(link2.children[0].children[0].description() === "pxObject", 'second node should be a pxObject'));
        
        return results;
      }); 
    },
    
    /**
     * test for negative link
     */
    test_link_negative: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/link-negative.md', function(payload) {
        var results = [];
        var link1 = payload.root.children[0];
        var link2 = payload.root.children[1];
        
        results.push(assert(link1.children[0].description() === "pxText", 'first node should be a pxText'));
        results.push(assert(link1.children[0].text === "google](", 'render text should be google]('));
        results.push(assert(link1.children[0].type !== "link", 'node type should not be link'));
        results.push(assert(link1.children[1].text === "https://www.google.com)", 'render text should be https://www.google.com)'));

        results.push(assert(link2.children[0].description() === "pxText", 'first node should be a pxText'));
        results.push(assert(link2.children[0].text === "[markdown file(", 'render text should be [markdown file('));
        results.push(assert(link2.children[0].type !== "link", 'node type should not be link'));
        results.push(assert(link2.children[1].text === "https://raw.githubusercontent.com/topcoderinc/pxCore/master/README.md)", 'render text should be https://raw.githubusercontent.com/topcoderinc/pxCore/master/README.md)'));

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
