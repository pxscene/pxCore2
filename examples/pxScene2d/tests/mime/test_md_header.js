/**
 * the unit tests for header
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
     * test header 1 - 6
     */
    test_header: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/header.md', function(payload) {
        var results = [];
        var styles = payload.options.styles;
        for(var i = 0 ; i < 6; i += 1) {
          var headerTextNode = payload.root.children[i].children[0];
          var style = styles['header-' + (i+1)];
          var text = 'Header - 0' + (i+1);
          results.push(assert(headerTextNode.text === text, "header text should be " + text));
          results.push(assert(headerTextNode.font === style.font, "header font should be " + style.font.url));
          results.push(assert(headerTextNode.pixelSize === style.pixelSize, "header pixelSize should be " + style.pixelSize));
        }
        return results;
      }); 
    },

    // test negative header
    test_header_negative: function() {
      return testUtil.loadMarkdown(childScene, basePackageUri + '/test-files/header.md', function(payload) {
        var results = [];
        var styles = payload.options.styles;
        for(var i = 6 ; i < payload.root.numChildren; i += 1) {
          var headerTextNode = payload.root.children[i].children[0];
          var text = '####### Header - 07 ######## Header - 08 ######### Header - 09'; // think is a line, when number of # greater than 6
          results.push(assert(headerTextNode.text === text, "negative header text should be " + text));
        }
        return results;
      }); 
    },
  }
  
  module.exports.tests = tests;
  if(manualTest === true) {
    manual.runTestsManually(tests);
  }

}).catch( function importFailed(err){
  console.error("Import failed for test_md_header.js: " + err)
});
