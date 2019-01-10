/**
 * Test for pxscene security model for URL
 *
 * Verifies that HTTP requests succeed when not blocked by scene permissions
 */

px.import({
  scene: 'px:scene.1.js'
})
.then(function(imports) {

  var scene = imports.scene;
  var child_url = px.getPackageBaseFilePath() + "/test_permissionsDefault.js";

  module.exports.tests = {
    allowGoogle: function () {
      return new Promise(function (resolve) {
        var new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: child_url,
          permissions: {
            "url" : {
              "allow" : [ "*://google.com" ],
              "block" : [ "*" ]
            }
          }
        });
        new_scene.ready.then(function (s) {
          s.api.tests.test_url_google().then(function (result) {
            resolve(result);
          });
        }).catch(function () {
          resolve("FAILURE: something went wrong");
        });
      });
    },
    blockGoogle: function () {
      return new Promise(function (resolve) {
        var new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: child_url,
          permissions: {
            "url" : {
              "allow" : [ "*://*.com" ],
              "block" : [ "*://google.com" ]
            }
          }
        });
        new_scene.ready.then(function (s) {
          s.api.tests.test_url_google().then(function (result) {
            resolve(result.indexOf("SUCCESS") === 0 ? "FAILURE: request was not blocked" : "SUCCESS");
          });
        }).catch(function () {
          resolve("FAILURE: something went wrong");
        });
      });
    },
    allowGoogleByDefault: function () {
      return new Promise(function (resolve) {
        var new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: child_url
        });
        new_scene.ready.then(function (s) {
          s.api.tests.test_url_google().then(function (result) {
            resolve(result);
          });
        }).catch(function () {
          resolve("FAILURE: something went wrong");
        });
      });
    },
    blockGoogleInRuntime: function () {
      return new Promise(function (resolve) {
        // a scene cannot set its own permissions hence the following assign shouldn't have effect
        scene.permissions = {
          "url" : {
            "block" : [ "*" ]
          }
        };
        // permissions can only be set by a parent scene
        var new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: child_url
        });
        new_scene.ready.then(function (s) {
          s.permissions = {
            "url" : {
              "block" : [ "*://google.com" ]
            }
          };
          s.api.tests.test_url_google().then(function (result) {
            resolve(result.indexOf("SUCCESS") === 0 ? "FAILURE: request was not blocked" : "SUCCESS");
          });
        }).catch(function () {
          resolve("FAILURE: something went wrong");
        });
      });
    }
  };
});
