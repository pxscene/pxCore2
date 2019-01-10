/**
 * unit tests for images
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
     * test network image and local image
     */
    test_image: function() {
      return new Promise(function(resolve, reject) {
        childScene.url = 'mimeScene.js?url=' + basePackageUri + '/test-files/image.md';
        childScene.ready.then(function() {
          childScene.api.mimeRenderer.ready.then(function(md) {
            var root = md.getParentRoot().children[0];
            setTimeout(() => {
              var image1 = root.children[0].children[0].children[0];
              var image2 = root.children[0].children[1].children[0];
              var results = [];
              Promise.all([image1.ready, image2.ready]).then(function(images) {

                // check image size and url
                results.push(assert(images[0].url === 'https://octodex.github.com/images/minion.png', 'image url should be https://octodex.github.com/images/minion.png'));
                results.push(assert(images[0].resource.w === 896, 'image w should be 896'));
                results.push(assert(images[0].resource.h === 896, 'image h should be 896'));

                results.push(assert(images[1].url.split('/').pop() === 'spark.png', 'image url should be spark.png'));
                results.push(assert(images[1].resource.w === 256, 'image w should be 2000'));
                results.push(assert(images[1].resource.h === 256, 'image h should be 2137'));

                resolve(results);
              }).catch(function(err) {
                reject(err);
              });
            }, 3000);
          });
        }).catch(function(err) {
          reject(err);
        });
      });
    },

    /**
     * test not exist image
     */
    test_image_negative: function() {
      return new Promise(function(resolve, reject) {
        childScene.url = 'mimeScene.js?url=' + basePackageUri + '/test-files/image-negative.md';
        childScene.ready.then(function() {
          childScene.api.mimeRenderer.ready.then(function(md) {
            var root = md.getParentRoot().children[0];
            setTimeout(() => {
              var image = root.children[0].children[0].children[0];
              var results = [];
              image.ready.then(function(img) {
                results.push(assert(false, 'image should be not exist'));
                reject(results);
              }).catch(function(err) {
                results.push(assert(true, 'image should be not exist'));
                resolve(results);
              });
            }, 1000);
          });
        }).catch(function(err) {
          reject(err);
        });
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
