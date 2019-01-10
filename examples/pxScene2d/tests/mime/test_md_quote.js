/**
 * unit tests for block quote
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
     * test block quote
     */
    test_quote: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/blockquote.md', function(payload) {
        var results = [];
        var rectangle = payload.root.children[0].children[0];
        var textNode = payload.root.children[0].children[1];
        results.push(assert(rectangle.description() === "pxRectangle", 'first node should be a pxRectangle'));
        results.push(assert(textNode.description() === "pxObject", 'second node should be a pxObject'));
        results.push(assert(textNode.children[0].text === "blockquote", 'render text should be blockquote'));
        return results;
      }); 
    },

    /**
     * test negative quote
     */
    test_quote_negative: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/blockquote-negative.md', function(payload) {
        var results = [];
        var textNode = payload.root.children[0].children[0];
        results.push(assert(textNode.description() === "pxText", 'second node should be a pxObject'));
        results.push(assert(textNode.text === "->blockquote", 'render text should be ->blockquote'));
        return results;
      }); 
    },
  }
  
  module.exports.tests = tests;
  if(manualTest === true) {
    manual.runTestsManually(tests);
  }

}).catch( function importFailed(err){
  console.error("Import failed for test_md_quote.js: " + err)
});
