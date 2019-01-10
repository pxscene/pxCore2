/**
 * unit tests for inline code and code elements
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
     * test inline code and code element
     */
    test_code: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/code.md', function(payload) {
        var results = [];
        var inlineCodeRoot = payload.root.children[0];
        var codeRoot = payload.root.children[1];

        // inline code
        results.push(assert(inlineCodeRoot.numChildren === 1, "this node children number should be 1"));
        results.push(assert(inlineCodeRoot.children[0].text === "inline-code", "render text should be inline-code"));
        
        // code
        results.push(assert(codeRoot.numChildren === 2, "this node children number should be 2"));
        results.push(assert(codeRoot.children[0].description() === "pxRectangle", "this item node type should be pxRectangle"));
        results.push(assert(codeRoot.children[1].description() === "pxTextBox", "this item node type should be pxTextBox"));
        results.push(assert(codeRoot.children[1].text === "var manualTest = manual.getManualTestValue();", "render text should be var manualTest = manual.getManualTestValue();"));
        return results;
      }); 
    },
    
    /**
     * test negative code element
     */
    test_code_negative: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/code-negative.md', function(payload) {
        var results = [];
        var textNode1 = payload.root.children[0]; 
        var textNode2 = payload.root.children[1]; 
        results.push(assert(textNode1.children[0].text === "`code-03-02", "render text should be `"));
        results.push(assert(textNode2.children[0].text === "`", "render text should be `"));
        results.push(assert(textNode2.children[1].text === "`", "render text should be `"));
        results.push(assert(textNode2.children[2].text === "`what's your name", "render text should be `what's your name"));
        return results;
      }); 
    },
  }
  
  module.exports.tests = tests;
  if(manualTest === true) {
    manual.runTestsManually(tests);
  }

}).catch( function importFailed(err){
  console.error("Import failed for test_md_code.js: " + err)
});
