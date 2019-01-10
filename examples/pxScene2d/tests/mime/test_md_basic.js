/**
 * unit tests for Emphasis (bold, italics, underline)
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
     * test bold element
     */
    test_bold: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/bold.md', function(payload) {
        var results = [];
        var textNode = payload.root.children[0].children[0];
        var FONT_STYLE = payload.options.FONT_STYLE;
        results.push(assert(textNode.text === 'Hello', "render text should be Hello"));
        results.push(assert(textNode.font === FONT_STYLE.BOLD, "render font should be BOLD"));
        return results;
      }); 
    },

    /**
     * test negative bold element
     */
    test_bold_negative: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/bold-negative-01.md', function(payload) {
        var results = [];
        var textNode = payload.root.children[0].children[1];
        var FONT_STYLE = payload.options.FONT_STYLE;
        results.push(assert(textNode.text === '*Hello', "render text should be *Hello"));
        results.push(assert(textNode.font === FONT_STYLE.REGULAR, "render font should be REGULAR"));
        return results;
      }); 
    },

    /**
     * test italic element
     */
    test_italics: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/italics.md', function(payload) {
        var results = [];
        var textNode = payload.root.children[0].children[0];
        var FONT_STYLE = payload.options.FONT_STYLE;
        results.push(assert(textNode.text === 'italics', "render text should be italics"));
        results.push(assert(textNode.font === FONT_STYLE.ITALIC, "render font should be ITALIC"));
        return results;
      }); 
    },

    /**
     * test negative italic element
     */
    test_italics_negative: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/italics-nagative.md', function(payload) {
        var results = [];
        var textNode = payload.root.children[0].children[0];
        var FONT_STYLE = payload.options.FONT_STYLE;
        results.push(assert(textNode.text === '*italics', "render text should be *italics"));
        results.push(assert(textNode.font === FONT_STYLE.REGULAR, "render font should be REGULAR"));
        return results;
      }); 
    },

    /**
     * test underline element
     */
    test_underline: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/underline.md', function(payload) {
        var results = [];
        var textNode = payload.root.children[0].children[0];
        results.push(assert(textNode.text === 'underline', "render text should be underline"));
        results.push(assert(textNode.children[0].description() === "pxRectangle", "underline should be had a child type for pxRectangle"));
        return results;
      }); 
    },

    /**
     * test negative element
     */
    test_underline_negative: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/underline-negative.md', function(payload) {
        var results = [];
        var textNode = payload.root.children[0].children[1];
        results.push(assert(textNode.text === '_underline', "render text should be _underline"));
        results.push(assert(textNode.numChildren === 0, "text should be 0 children"));
        return results;
      }); 
    },
  }
  
  module.exports.tests = tests;
  if(manualTest === true) {
    manual.runTestsManually(tests);
  }

}).catch( function importFailed(err){
  console.error("Import failed for test_md_basic.js: " + err)
});
